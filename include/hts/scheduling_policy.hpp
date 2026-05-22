#pragma once

#include "hts/types.hpp"
#include <memory>
#include <mutex>
#include <vector>

namespace hts {

/// Abstract scheduling policy interface
class SchedulingPolicy {
  public:
    virtual ~SchedulingPolicy() = default;

    /// Select device for a task
    /// @param task The task to schedule
    /// @param cpu_load Current CPU load (0.0 to 1.0)
    /// @param gpu_load Current GPU load (0.0 to 1.0)
    /// @return Selected device type
    virtual DeviceType select_device(const Task &task, double cpu_load, double gpu_load) = 0;

    /// Sort ready tasks by execution priority
    /// @param tasks Vector of ready tasks (modified in place)
    virtual void prioritize(std::vector<std::shared_ptr<Task>> &tasks) = 0;

    /// Get policy name
    virtual const char *name() const = 0;
};

/// Default policy: load-based device selection, priority-based ordering
class DefaultSchedulingPolicy : public SchedulingPolicy {
  public:
    DeviceType select_device(const Task &task, double cpu_load, double gpu_load) override;
    void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override;
    const char *name() const override;
};

/// GPU-first policy: prefer GPU execution when possible
class GpuFirstPolicy : public SchedulingPolicy {
  public:
    DeviceType select_device(const Task &task, double cpu_load, double gpu_load) override;
    void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override;
    const char *name() const override;
};

/// CPU-first policy: prefer CPU execution when possible
class CpuFirstPolicy : public SchedulingPolicy {
  public:
    DeviceType select_device(const Task &task, double cpu_load, double gpu_load) override;
    void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override;
    const char *name() const override;
};

/// Round-robin policy: alternate between CPU and GPU
class RoundRobinPolicy : public SchedulingPolicy {
  public:
    DeviceType select_device(const Task &task, double cpu_load, double gpu_load) override;
    void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override;
    const char *name() const override;

  private:
    mutable std::mutex mutex_;
    bool use_gpu_ = false;
};

/// Shortest-job-first policy (requires estimated execution time)
class ShortestJobFirstPolicy : public SchedulingPolicy {
  public:
    DeviceType select_device(const Task &task, double cpu_load, double gpu_load) override;
    void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override;
    const char *name() const override;
};

} // namespace hts
