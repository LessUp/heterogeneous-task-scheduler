#pragma once

#include "hts/task.hpp"
#include "hts/task_graph.hpp"
#include "hts/types.hpp"
#include <atomic>
#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace hts {

/// TaskBarrier provides synchronization points in task execution
class TaskBarrier {
public:
  using TaskPtr = std::shared_ptr<Task>;

  /// Create a barrier with a name
  explicit TaskBarrier(const std::string &name, TaskGraph &graph);

  /// Get barrier name
  const std::string &name() const { return name_; }

  /// Add a task that must complete before the barrier
  void add_predecessor(TaskId id);
  void add_predecessor(TaskPtr task);

  /// Add a task that waits on the barrier
  void add_successor(TaskId id);
  void add_successor(TaskPtr task);

  /// Get the barrier task (internal synchronization point)
  TaskPtr barrier_task() const { return barrier_task_; }

  /// Get all predecessor tasks
  const std::vector<TaskId> &predecessors() const { return predecessors_; }

  /// Get all successor tasks
  const std::vector<TaskId> &successors() const { return successors_; }

  /// Check if all predecessors are complete
  bool is_ready() const;

  /// Get completion count
  size_t completed_count() const { return completed_count_.load(); }

  /// Get total predecessor count
  size_t total_count() const { return predecessors_.size(); }

  /// Notify that a predecessor completed
  void notify_completion(TaskId id);

  /// Wait for all predecessors to complete
  void wait();

  /// Wait with timeout
  template <typename Rep, typename Period>
  bool wait_for(const std::chrono::duration<Rep, Period> &timeout) {
    std::unique_lock<std::mutex> lock(mutex_);
    return cv_.wait_for(lock, timeout, [this] {
      return completed_count_.load() >= predecessors_.size();
    });
  }

private:
  std::string name_;
  TaskGraph &graph_;
  TaskPtr barrier_task_;
  std::vector<TaskId> predecessors_;
  std::vector<TaskId> successors_;
  std::atomic<size_t> completed_count_{0};
  std::mutex mutex_;
  std::condition_variable cv_;
};

/// BarrierGroup manages multiple barriers for complex synchronization
class BarrierGroup {
public:
  explicit BarrierGroup(TaskGraph &graph) : graph_(graph) {}

  /// Create a new barrier
  TaskBarrier &create_barrier(const std::string &name) {
    barriers_.emplace_back(std::make_unique<TaskBarrier>(name, graph_));
    return *barriers_.back();
  }

  /// Get barrier by name
  TaskBarrier *get_barrier(const std::string &name) {
    for (auto &barrier : barriers_) {
      if (barrier->name() == name) {
        return barrier.get();
      }
    }
    return nullptr;
  }

  /// Get all barriers
  const std::vector<std::unique_ptr<TaskBarrier>> &barriers() const {
    return barriers_;
  }

  /// Wait for all barriers
  void wait_all() {
    for (auto &barrier : barriers_) {
      barrier->wait();
    }
  }

private:
  TaskGraph &graph_;
  std::vector<std::unique_ptr<TaskBarrier>> barriers_;
};

} // namespace hts
