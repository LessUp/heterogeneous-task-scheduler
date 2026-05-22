#pragma once

#include "hts/types.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace hts {

class TaskContext;

struct TaskDefinition {
    TaskId id = 0;
    DeviceType preferred_device = DeviceType::Any;
    TaskPriority priority = TaskPriority::Normal;
    std::string name;
};

struct TaskRuntimeState {
    TaskState state = TaskState::Pending;
    DeviceType actual_device = DeviceType::CPU;
    std::chrono::nanoseconds execution_time{0};
    bool cancelled = false;
};

/// Task represents a unit of work that can execute on CPU or GPU
class Task {
  public:
    using CpuFunction = std::function<void(TaskContext &)>;
    using GpuFunction = std::function<void(TaskContext &, cudaStream_t)>;

    /// Construct a task with given ID and preferred device
    explicit Task(TaskId id, DeviceType preferred_device = DeviceType::Any);

    // Non-copyable, movable
    Task(const Task &) = delete;
    Task &operator=(const Task &) = delete;
    Task(Task &&) noexcept = default;
    Task &operator=(Task &&) noexcept = default;

    /// Set CPU execution function
    void set_cpu_function(CpuFunction func);

    /// Set GPU execution function
    void set_gpu_function(GpuFunction func);

    /// Get task ID
    TaskId id() const { return definition_.id; }

    /// Get current state
    TaskState state() const {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        return runtime_state_.state;
    }

    /// Set task state
    void set_state(TaskState state) {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        runtime_state_.state = state;
    }

    /// Get preferred device
    DeviceType preferred_device() const { return definition_.preferred_device; }

    /// Set preferred device
    void set_preferred_device(DeviceType device) { definition_.preferred_device = device; }

    /// Get actual device (set after scheduling)
    DeviceType actual_device() const {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        return runtime_state_.actual_device;
    }

    /// Set actual device
    void set_actual_device(DeviceType device) {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        runtime_state_.actual_device = device;
    }

    /// Get execution time (valid after completion)
    std::chrono::nanoseconds execution_time() const {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        return runtime_state_.execution_time;
    }

    /// Set execution time
    void set_execution_time(std::chrono::nanoseconds time) {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        runtime_state_.execution_time = time;
    }

    /// Get task priority
    TaskPriority priority() const { return definition_.priority; }

    /// Set task priority
    void set_priority(TaskPriority priority) { definition_.priority = priority; }

    /// Get task name (for debugging)
    const std::string &name() const { return definition_.name; }

    /// Set task name
    void set_name(const std::string &name) { definition_.name = name; }

    /// Check if task is cancelled
    bool is_cancelled() const {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        return runtime_state_.cancelled;
    }

    /// Cancel the task
    void cancel() { mark_cancelled(); }

    /// Get immutable task definition
    const TaskDefinition &definition() const { return definition_; }

    /// Get a snapshot of mutable runtime state
    TaskRuntimeState runtime_state() const {
        std::lock_guard<std::mutex> lock(runtime_mutex_);
        return runtime_state_;
    }

    /// Runtime transitions (separate from declarative definition)
    void mark_ready();
    void mark_running(DeviceType actual_device);
    void mark_completed(std::chrono::nanoseconds execution_time);
    void mark_failed();
    void mark_blocked();
    void mark_cancelled();

    /// Check if CPU function is set
    bool has_cpu_function() const { return static_cast<bool>(cpu_func_); }

    /// Check if GPU function is set
    bool has_gpu_function() const { return static_cast<bool>(gpu_func_); }

    /// Execute on CPU
    void execute_cpu(TaskContext &ctx);

    /// Execute on GPU
    void execute_gpu(TaskContext &ctx, cudaStream_t stream);

  private:
    TaskDefinition definition_;
    mutable std::mutex runtime_mutex_;
    TaskRuntimeState runtime_state_;
    CpuFunction cpu_func_;
    GpuFunction gpu_func_;
    mutable std::mutex func_mutex_;
};

} // namespace hts
