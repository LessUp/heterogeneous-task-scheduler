#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

// Forward declare CUDA types to avoid including cuda_runtime.h in headers
using cudaStream_t = struct CUstream_st *;
using cudaEvent_t = struct CUevent_st *;

namespace hts {

/// Execution device type
enum class DeviceType {
  CPU, ///< Execute on CPU
  GPU, ///< Execute on GPU
  Any  ///< Scheduler decides based on load
};

/// Task execution state
enum class TaskState {
  Pending,   ///< Waiting for dependencies
  Ready,     ///< All dependencies satisfied, can execute
  Running,   ///< Currently executing
  Completed, ///< Successfully completed
  Failed,    ///< Execution failed
  Blocked,   ///< Blocked due to upstream failure
  Cancelled  ///< Cancelled by user
};

/// Task priority (higher value = higher priority)
enum class TaskPriority { Low = 0, Normal = 1, High = 2, Critical = 3 };

/// Unique task identifier
using TaskId = uint64_t;

/// Memory statistics
struct MemoryStats {
  size_t total_bytes = 0;           ///< Total pool size
  size_t used_bytes = 0;            ///< Currently allocated
  size_t peak_bytes = 0;            ///< Peak usage
  size_t allocation_count = 0;      ///< Number of allocations
  size_t free_count = 0;            ///< Number of frees
  double fragmentation_ratio = 0.0; ///< Fragmentation metric [0,1]
};

/// Error information for failed tasks
struct TaskError {
  TaskId task_id = 0;
  DeviceType device = DeviceType::CPU;
  std::string message;
  int cuda_error_code = 0; ///< 0 if not a CUDA error
  std::chrono::system_clock::time_point timestamp;
};

/// Timeline event for visualization
struct TimelineEvent {
  TaskId task_id;
  DeviceType device;
  std::chrono::high_resolution_clock::time_point start_time;
  std::chrono::high_resolution_clock::time_point end_time;
  TaskState final_state;
};

/// Execution timeline
struct ExecutionTimeline {
  std::vector<TimelineEvent> events;
  std::chrono::high_resolution_clock::time_point graph_start;
  std::chrono::high_resolution_clock::time_point graph_end;
};

/// Scheduler configuration
struct SchedulerConfig {
  size_t memory_pool_size = 256 * 1024 * 1024; ///< 256 MB default
  size_t cpu_thread_count = 4;
  size_t gpu_stream_count = 4;
  bool allow_memory_growth = true;
  size_t max_retry_count = 0; ///< Max retries for failed tasks (0 = no retry)
  std::chrono::milliseconds retry_delay{100}; ///< Delay between retries
};

/// Execution statistics
struct ExecutionStats {
  std::chrono::nanoseconds total_time{0};
  std::unordered_map<TaskId, std::chrono::nanoseconds> task_times;
  double cpu_utilization = 0.0;
  double gpu_utilization = 0.0;
  MemoryStats memory_stats;
};

// Forward declarations
class Task;
class TaskContext;
class TaskGraph;
class DependencyManager;
class MemoryPool;
class StreamManager;
class ExecutionEngine;
class Scheduler;

} // namespace hts
