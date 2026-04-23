#pragma once

#include "hts/types.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <string>

namespace hts {

class TaskContext;

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
    TaskId id() const { return id_; }

    /// Get current state
    TaskState state() const { return state_.load(); }

    /// Set task state
    void set_state(TaskState state) { state_.store(state); }

    /// Get preferred device
    DeviceType preferred_device() const { return preferred_device_; }

    /// Set preferred device
    void set_preferred_device(DeviceType device) { preferred_device_ = device; }

    /// Get actual device (set after scheduling)
    DeviceType actual_device() const { return actual_device_; }

    /// Set actual device
    void set_actual_device(DeviceType device) { actual_device_ = device; }

    /// Get execution time (valid after completion)
    std::chrono::nanoseconds execution_time() const { return exec_time_; }

    /// Set execution time
    void set_execution_time(std::chrono::nanoseconds time) { exec_time_ = time; }

    /// Get task priority
    TaskPriority priority() const { return priority_; }

    /// Set task priority
    void set_priority(TaskPriority priority) { priority_ = priority; }

    /// Get task name (for debugging)
    const std::string &name() const { return name_; }

    /// Set task name
    void set_name(const std::string &name) { name_ = name; }

    /// Check if task is cancelled
    bool is_cancelled() const { return cancelled_.load(); }

    /// Cancel the task
    void cancel() { cancelled_ = true; }

    /// Check if CPU function is set
    bool has_cpu_function() const { return static_cast<bool>(cpu_func_); }

    /// Check if GPU function is set
    bool has_gpu_function() const { return static_cast<bool>(gpu_func_); }

    /// Execute on CPU
    void execute_cpu(TaskContext &ctx);

    /// Execute on GPU
    void execute_gpu(TaskContext &ctx, cudaStream_t stream);

  private:
    TaskId id_;
    std::atomic<TaskState> state_{TaskState::Pending};
    DeviceType preferred_device_;
    DeviceType actual_device_{DeviceType::CPU};
    CpuFunction cpu_func_;
    GpuFunction gpu_func_;
    std::chrono::nanoseconds exec_time_{0};
    TaskPriority priority_{TaskPriority::Normal};
    std::string name_;
    std::atomic<bool> cancelled_{false};
    mutable std::mutex func_mutex_;
};

} // namespace hts
