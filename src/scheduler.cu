#include "hts/scheduler.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace hts {

Scheduler::Scheduler(const SchedulerConfig &config)
    : config_(config), policy_(std::make_unique<DefaultSchedulingPolicy>()) {
  memory_pool_ = std::make_unique<MemoryPool>(config.memory_pool_size,
                                              config.allow_memory_growth);

  engine_ = std::make_unique<ExecutionEngine>(
      *memory_pool_, config.cpu_thread_count, config.gpu_stream_count);
}

Scheduler::~Scheduler() {
  // Wait for any ongoing execution
  if (executing_) {
    engine_->wait_all();
  }
}

void Scheduler::execute() {
  auto future = execute_async();
  future.get(); // Block until complete
}

std::future<void> Scheduler::execute_async() {
  return std::async(std::launch::async, [this]() { execute_internal(); });
}

void Scheduler::execute_internal() {
  std::lock_guard<std::mutex> lock(execution_mutex_);

  if (!graph_.validate()) {
    throw std::runtime_error("Task graph validation failed - contains cycles");
  }

  executing_ = true;

  // Start profiling if enabled
  if (profiling_enabled_) {
    profiler_.start();
  }

  // Initialize dependency manager
  dep_manager_ = std::make_unique<DependencyManager>(graph_);

  // Reset stats
  stats_ = ExecutionStats{};
  timeline_ = ExecutionTimeline{};
  timeline_.graph_start = std::chrono::high_resolution_clock::now();

  // Track pending futures
  std::unordered_map<TaskId, std::future<void>> task_futures;

  // Initial scheduling of root tasks
  auto ready = dep_manager_->get_ready_tasks();
  for (TaskId id : ready) {
    auto task = graph_.get_task(id);
    if (task) {
      DeviceType device = select_device(*task);
      auto start = std::chrono::high_resolution_clock::now();

      if (profiling_enabled_) {
        profiler_.record_task_start(id, task->name(), device);
      }

      task_futures[id] = engine_->execute_task(task, device);
    }
  }

  // Process until all tasks complete
  while (!task_futures.empty()) {
    // Check for completed tasks
    std::vector<TaskId> completed;

    for (auto &[id, future] : task_futures) {
      if (future.wait_for(std::chrono::milliseconds(1)) ==
          std::future_status::ready) {
        auto task = graph_.get_task(id);

        try {
          future.get(); // May throw if task failed
          on_task_completed(id);

          // Record timeline
          if (task) {
            auto end = std::chrono::high_resolution_clock::now();
            auto start = end - task->execution_time();
            record_timeline_event(id, task->actual_device(), start, end,
                                  TaskState::Completed);
            stats_.task_times[id] = task->execution_time();

            if (profiling_enabled_) {
              profiler_.record_task_end(id, TaskState::Completed);
            }
          }
        } catch (const std::exception &e) {
          on_task_failed(id, e.what());

          if (task) {
            auto end = std::chrono::high_resolution_clock::now();
            record_timeline_event(id, task->actual_device(), end, end,
                                  TaskState::Failed);

            if (profiling_enabled_) {
              profiler_.record_task_end(id, TaskState::Failed);
            }
          }
        }

        completed.push_back(id);
      }
    }

    // Remove completed futures
    for (TaskId id : completed) {
      task_futures.erase(id);
    }

    // Schedule newly ready tasks
    auto newly_ready = dep_manager_->get_ready_tasks();
    for (TaskId id : newly_ready) {
      if (task_futures.find(id) == task_futures.end()) {
        auto task = graph_.get_task(id);
        if (task && task->state() == TaskState::Pending) {
          DeviceType device = select_device(*task);

          if (profiling_enabled_) {
            profiler_.record_task_start(id, task->name(), device);
          }

          task_futures[id] = engine_->execute_task(task, device);
        }
      }
    }

    // Small sleep to avoid busy waiting
    if (task_futures.empty())
      break;
    std::this_thread::sleep_for(std::chrono::microseconds(100));
  }

  timeline_.graph_end = std::chrono::high_resolution_clock::now();
  stats_.total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
      timeline_.graph_end - timeline_.graph_start);

  // Calculate utilization (simplified)
  stats_.cpu_utilization = engine_->get_cpu_load();
  stats_.gpu_utilization = engine_->get_gpu_load();
  stats_.memory_stats = memory_pool_->get_stats();

  // Stop profiling
  if (profiling_enabled_) {
    profiler_.stop();
  }

  executing_ = false;
}

DeviceType Scheduler::select_device(const Task &task) {
  return policy_->select_device(task, engine_->get_cpu_load(),
                                engine_->get_gpu_load());
}

void Scheduler::on_task_completed(TaskId id) {
  dep_manager_->mark_completed(id);
}

void Scheduler::on_task_failed(TaskId id, const std::string &error) {
  dep_manager_->mark_failed(id);

  if (error_callback_) {
    error_callback_(id, error);
  }
}

void Scheduler::set_error_callback(ErrorCallback callback) {
  error_callback_ = std::move(callback);
}

ExecutionStats Scheduler::get_stats() const { return stats_; }

void Scheduler::record_timeline_event(
    TaskId id, DeviceType device,
    std::chrono::high_resolution_clock::time_point start,
    std::chrono::high_resolution_clock::time_point end, TaskState state) {
  TimelineEvent event;
  event.task_id = id;
  event.device = device;
  event.start_time = start;
  event.end_time = end;
  event.final_state = state;
  timeline_.events.push_back(event);
}

std::string Scheduler::generate_timeline_json() const {
  std::ostringstream oss;
  oss << std::fixed << std::setprecision(6);

  auto to_ms = [&](auto tp) {
    auto duration = tp - timeline_.graph_start;
    return std::chrono::duration<double, std::milli>(duration).count();
  };

  oss << "{\n";
  oss << "  \"graph_start_ms\": 0,\n";
  oss << "  \"graph_end_ms\": " << to_ms(timeline_.graph_end) << ",\n";
  oss << "  \"events\": [\n";

  for (size_t i = 0; i < timeline_.events.size(); ++i) {
    const auto &event = timeline_.events[i];
    oss << "    {\n";
    oss << "      \"task_id\": " << event.task_id << ",\n";
    oss << "      \"device\": \""
        << (event.device == DeviceType::GPU ? "GPU" : "CPU") << "\",\n";
    oss << "      \"start_ms\": " << to_ms(event.start_time) << ",\n";
    oss << "      \"end_ms\": " << to_ms(event.end_time) << ",\n";
    oss << "      \"state\": \"" << static_cast<int>(event.final_state)
        << "\"\n";
    oss << "    }";
    if (i < timeline_.events.size() - 1)
      oss << ",";
    oss << "\n";
  }

  oss << "  ]\n";
  oss << "}\n";

  return oss.str();
}

void Scheduler::set_policy(std::unique_ptr<SchedulingPolicy> policy) {
  policy_ = std::move(policy);
}

const char *Scheduler::policy_name() const {
  return policy_ ? policy_->name() : "None";
}

} // namespace hts
