#pragma once

#include "hts/task.hpp"
#include "hts/types.hpp"
#include <algorithm>
#include <memory>
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
  virtual DeviceType select_device(const Task &task, double cpu_load,
                                   double gpu_load) = 0;

  /// Sort ready tasks by execution priority
  /// @param tasks Vector of ready tasks (modified in place)
  virtual void prioritize(std::vector<std::shared_ptr<Task>> &tasks) = 0;

  /// Get policy name
  virtual const char *name() const = 0;
};

/// Default policy: load-based device selection, priority-based ordering
class DefaultSchedulingPolicy : public SchedulingPolicy {
public:
  DeviceType select_device(const Task &task, double cpu_load,
                           double gpu_load) override {
    DeviceType preferred = task.preferred_device();

    if (preferred != DeviceType::Any) {
      return preferred;
    }

    // Prefer GPU if both functions available and GPU is less loaded
    if (task.has_gpu_function() && task.has_cpu_function()) {
      return (gpu_load <= cpu_load) ? DeviceType::GPU : DeviceType::CPU;
    }

    if (task.has_gpu_function())
      return DeviceType::GPU;
    if (task.has_cpu_function())
      return DeviceType::CPU;

    return DeviceType::CPU; // Fallback
  }

  void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override {
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
      return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
  }

  const char *name() const override { return "Default"; }
};

/// GPU-first policy: prefer GPU execution when possible
class GpuFirstPolicy : public SchedulingPolicy {
public:
  DeviceType select_device(const Task &task, double cpu_load,
                           double gpu_load) override {
    if (task.preferred_device() == DeviceType::CPU) {
      return DeviceType::CPU;
    }

    if (task.has_gpu_function()) {
      return DeviceType::GPU;
    }

    return DeviceType::CPU;
  }

  void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override {
    // GPU tasks first, then by priority
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
      bool a_gpu =
          a->preferred_device() == DeviceType::GPU ||
          (a->preferred_device() == DeviceType::Any && a->has_gpu_function());
      bool b_gpu =
          b->preferred_device() == DeviceType::GPU ||
          (b->preferred_device() == DeviceType::Any && b->has_gpu_function());

      if (a_gpu != b_gpu)
        return a_gpu > b_gpu;
      return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
  }

  const char *name() const override { return "GPU-First"; }
};

/// CPU-first policy: prefer CPU execution when possible
class CpuFirstPolicy : public SchedulingPolicy {
public:
  DeviceType select_device(const Task &task, double cpu_load,
                           double gpu_load) override {
    if (task.preferred_device() == DeviceType::GPU) {
      return DeviceType::GPU;
    }

    if (task.has_cpu_function()) {
      return DeviceType::CPU;
    }

    return DeviceType::GPU;
  }

  void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override {
    // CPU tasks first, then by priority
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
      bool a_cpu =
          a->preferred_device() == DeviceType::CPU ||
          (a->preferred_device() == DeviceType::Any && a->has_cpu_function());
      bool b_cpu =
          b->preferred_device() == DeviceType::CPU ||
          (b->preferred_device() == DeviceType::Any && b->has_cpu_function());

      if (a_cpu != b_cpu)
        return a_cpu > b_cpu;
      return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
  }

  const char *name() const override { return "CPU-First"; }
};

/// Round-robin policy: alternate between CPU and GPU
class RoundRobinPolicy : public SchedulingPolicy {
public:
  DeviceType select_device(const Task &task, double cpu_load,
                           double gpu_load) override {
    if (task.preferred_device() != DeviceType::Any) {
      return task.preferred_device();
    }

    use_gpu_ = !use_gpu_;

    if (use_gpu_ && task.has_gpu_function()) {
      return DeviceType::GPU;
    }
    if (!use_gpu_ && task.has_cpu_function()) {
      return DeviceType::CPU;
    }

    // Fallback to available
    if (task.has_gpu_function())
      return DeviceType::GPU;
    return DeviceType::CPU;
  }

  void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override {
    // Just sort by priority
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
      return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
  }

  const char *name() const override { return "Round-Robin"; }

private:
  bool use_gpu_ = false;
};

/// Shortest-job-first policy (requires estimated execution time)
class ShortestJobFirstPolicy : public SchedulingPolicy {
public:
  DeviceType select_device(const Task &task, double cpu_load,
                           double gpu_load) override {
    // Same as default
    if (task.preferred_device() != DeviceType::Any) {
      return task.preferred_device();
    }

    if (task.has_gpu_function() && task.has_cpu_function()) {
      return (gpu_load <= cpu_load) ? DeviceType::GPU : DeviceType::CPU;
    }

    if (task.has_gpu_function())
      return DeviceType::GPU;
    return DeviceType::CPU;
  }

  void prioritize(std::vector<std::shared_ptr<Task>> &tasks) override {
    // Sort by priority first (as proxy for job size)
    // Higher priority = shorter/more important job
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
      return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
  }

  const char *name() const override { return "Shortest-Job-First"; }
};

} // namespace hts
