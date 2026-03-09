#include "hts/task_barrier.hpp"

namespace hts {

TaskBarrier::TaskBarrier(const std::string &name, TaskGraph &graph)
    : name_(name), graph_(graph) {
  // Create an internal barrier task that acts as synchronization point
  barrier_task_ = graph_.add_task(DeviceType::CPU);
  barrier_task_->set_name("barrier:" + name);
  barrier_task_->set_cpu_function([](TaskContext &) {
    // Barrier task does nothing - it's just a sync point
  });
}

void TaskBarrier::add_predecessor(TaskId id) {
  predecessors_.push_back(id);
  graph_.add_dependency(id, barrier_task_->id());
}

void TaskBarrier::add_predecessor(TaskPtr task) {
  if (task) {
    add_predecessor(task->id());
  }
}

void TaskBarrier::add_successor(TaskId id) {
  successors_.push_back(id);
  graph_.add_dependency(barrier_task_->id(), id);
}

void TaskBarrier::add_successor(TaskPtr task) {
  if (task) {
    add_successor(task->id());
  }
}

bool TaskBarrier::is_ready() const {
  return completed_count_.load() >= predecessors_.size();
}

void TaskBarrier::notify_completion(TaskId id) {
  for (const auto &pred : predecessors_) {
    if (pred == id) {
      size_t count = ++completed_count_;
      if (count >= predecessors_.size()) {
        std::lock_guard<std::mutex> lock(mutex_);
        cv_.notify_all();
      }
      break;
    }
  }
}

void TaskBarrier::wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  cv_.wait(lock,
           [this] { return completed_count_.load() >= predecessors_.size(); });
}

} // namespace hts
