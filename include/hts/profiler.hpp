#pragma once

#include "hts/types.hpp"
#include <algorithm>
#include <chrono>
#include <iomanip>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace hts {

/// Performance profiler for task execution analysis
class Profiler {
public:
  /// Task execution record
  struct TaskRecord {
    TaskId task_id;
    std::string task_name;
    DeviceType device;
    TaskState final_state;
    std::chrono::high_resolution_clock::time_point start_time;
    std::chrono::high_resolution_clock::time_point end_time;
    std::chrono::nanoseconds queue_wait_time{0};
    size_t memory_allocated{0};
    size_t memory_freed{0};
  };

  /// Summary statistics
  struct Summary {
    size_t total_tasks{0};
    size_t completed_tasks{0};
    size_t failed_tasks{0};
    size_t blocked_tasks{0};

    std::chrono::nanoseconds total_time{0};
    std::chrono::nanoseconds cpu_time{0};
    std::chrono::nanoseconds gpu_time{0};
    std::chrono::nanoseconds avg_task_time{0};
    std::chrono::nanoseconds max_task_time{0};
    std::chrono::nanoseconds min_task_time{std::chrono::nanoseconds::max()};

    double cpu_utilization{0.0};
    double gpu_utilization{0.0};
    double parallelism{0.0}; // Average concurrent tasks

    size_t peak_memory{0};
    size_t total_allocations{0};
  };

  Profiler() = default;

  /// Start profiling
  void start() {
    std::lock_guard<std::mutex> lock(mutex_);
    records_.clear();
    start_time_ = std::chrono::high_resolution_clock::now();
    active_ = true;
  }

  /// Stop profiling
  void stop() {
    std::lock_guard<std::mutex> lock(mutex_);
    end_time_ = std::chrono::high_resolution_clock::now();
    active_ = false;
  }

  /// Record task start
  void record_task_start(TaskId id, const std::string &name,
                         DeviceType device) {
    if (!active_)
      return;

    std::lock_guard<std::mutex> lock(mutex_);
    TaskRecord record;
    record.task_id = id;
    record.task_name = name;
    record.device = device;
    record.start_time = std::chrono::high_resolution_clock::now();
    pending_records_[id] = record;
  }

  /// Record task end
  void record_task_end(TaskId id, TaskState state) {
    if (!active_)
      return;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pending_records_.find(id);
    if (it != pending_records_.end()) {
      it->second.end_time = std::chrono::high_resolution_clock::now();
      it->second.final_state = state;
      records_.push_back(it->second);
      pending_records_.erase(it);
    }
  }

  /// Record memory allocation
  void record_allocation(TaskId id, size_t bytes) {
    if (!active_)
      return;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pending_records_.find(id);
    if (it != pending_records_.end()) {
      it->second.memory_allocated += bytes;
    }
    total_allocations_++;
  }

  /// Record memory free
  void record_free(TaskId id, size_t bytes) {
    if (!active_)
      return;

    std::lock_guard<std::mutex> lock(mutex_);
    auto it = pending_records_.find(id);
    if (it != pending_records_.end()) {
      it->second.memory_freed += bytes;
    }
  }

  /// Get all records
  const std::vector<TaskRecord> &records() const { return records_; }

  /// Generate summary
  Summary generate_summary() const {
    std::lock_guard<std::mutex> lock(mutex_);

    Summary summary;
    summary.total_tasks = records_.size();

    std::chrono::nanoseconds total_cpu_time{0};
    std::chrono::nanoseconds total_gpu_time{0};

    for (const auto &record : records_) {
      auto duration = record.end_time - record.start_time;
      auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);

      switch (record.final_state) {
      case TaskState::Completed:
        summary.completed_tasks++;
        break;
      case TaskState::Failed:
        summary.failed_tasks++;
        break;
      case TaskState::Blocked:
        summary.blocked_tasks++;
        break;
      default:
        break;
      }

      if (record.device == DeviceType::CPU) {
        total_cpu_time += ns;
      } else {
        total_gpu_time += ns;
      }

      if (ns > summary.max_task_time) {
        summary.max_task_time = ns;
      }
      if (ns < summary.min_task_time) {
        summary.min_task_time = ns;
      }

      cumulative_memory += record.memory_allocated;
      summary.peak_memory = std::max(summary.peak_memory, cumulative_memory);
    }

    summary.total_time = std::chrono::duration_cast<std::chrono::nanoseconds>(
        end_time_ - start_time_);
    summary.cpu_time = total_cpu_time;
    summary.gpu_time = total_gpu_time;

    if (!records_.empty()) {
      summary.avg_task_time =
          (total_cpu_time + total_gpu_time) / records_.size();
    }

    if (summary.total_time.count() > 0) {
      summary.cpu_utilization = static_cast<double>(total_cpu_time.count()) /
                                summary.total_time.count();
      summary.gpu_utilization = static_cast<double>(total_gpu_time.count()) /
                                summary.total_time.count();
      summary.parallelism =
          static_cast<double>(total_cpu_time.count() + total_gpu_time.count()) /
          summary.total_time.count();
    }

    summary.total_allocations = total_allocations_;

    if (summary.min_task_time == std::chrono::nanoseconds::max()) {
      summary.min_task_time = std::chrono::nanoseconds{0};
    }

    return summary;
  }

  /// Generate text report
  std::string generate_report() const {
    auto summary = generate_summary();

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2);

    oss << "=== HTS Performance Report ===\n\n";

    oss << "Task Statistics:\n";
    oss << "  Total tasks:     " << summary.total_tasks << "\n";
    oss << "  Completed:       " << summary.completed_tasks << "\n";
    oss << "  Failed:          " << summary.failed_tasks << "\n";
    oss << "  Blocked:         " << summary.blocked_tasks << "\n\n";

    oss << "Timing:\n";
    oss << "  Total time:      " << summary.total_time.count() / 1e6 << " ms\n";
    oss << "  CPU time:        " << summary.cpu_time.count() / 1e6 << " ms\n";
    oss << "  GPU time:        " << summary.gpu_time.count() / 1e6 << " ms\n";
    oss << "  Avg task time:   " << summary.avg_task_time.count() / 1e3
        << " us\n";
    oss << "  Max task time:   " << summary.max_task_time.count() / 1e3
        << " us\n";
    oss << "  Min task time:   " << summary.min_task_time.count() / 1e3
        << " us\n\n";

    oss << "Utilization:\n";
    oss << "  CPU utilization: " << summary.cpu_utilization * 100 << "%\n";
    oss << "  GPU utilization: " << summary.gpu_utilization * 100 << "%\n";
    oss << "  Parallelism:     " << summary.parallelism << "x\n\n";

    oss << "Memory:\n";
    oss << "  Peak memory:     " << summary.peak_memory / 1024 << " KB\n";
    oss << "  Allocations:     " << summary.total_allocations << "\n\n";

    oss << "Task Details:\n";
    for (const auto &record : records_) {
      auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
          record.end_time - record.start_time);

      oss << "  [" << record.task_id << "] ";
      if (!record.task_name.empty()) {
        oss << record.task_name << " ";
      }
      oss << "(" << (record.device == DeviceType::CPU ? "CPU" : "GPU") << ") ";
      oss << duration.count() << " us ";
      oss << "[" << state_string(record.final_state) << "]\n";
    }

    return oss.str();
  }

private:
  static const char *state_string(TaskState state) {
    switch (state) {
    case TaskState::Completed:
      return "OK";
    case TaskState::Failed:
      return "FAIL";
    case TaskState::Blocked:
      return "BLOCKED";
    case TaskState::Cancelled:
      return "CANCELLED";
    default:
      return "UNKNOWN";
    }
  }

  mutable std::mutex mutex_;
  bool active_ = false;
  std::chrono::high_resolution_clock::time_point start_time_;
  std::chrono::high_resolution_clock::time_point end_time_;
  std::vector<TaskRecord> records_;
  std::unordered_map<TaskId, TaskRecord> pending_records_;
  size_t total_allocations_ = 0;
};

} // namespace hts
