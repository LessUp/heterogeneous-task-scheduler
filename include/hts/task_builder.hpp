#pragma once

#include "hts/task.hpp"
#include "hts/task_graph.hpp"
#include "hts/types.hpp"
#include <memory>
#include <string>

namespace hts {

/// Fluent builder for creating tasks
class TaskBuilder {
public:
  explicit TaskBuilder(TaskGraph &graph) : graph_(graph) {}

  /// Set task name
  TaskBuilder &name(const std::string &name) {
    name_ = name;
    return *this;
  }

  /// Set preferred device
  TaskBuilder &device(DeviceType device) {
    device_ = device;
    return *this;
  }

  /// Set task priority
  TaskBuilder &priority(TaskPriority priority) {
    priority_ = priority;
    return *this;
  }

  /// Set CPU function
  TaskBuilder &cpu(Task::CpuFunction func) {
    cpu_func_ = std::move(func);
    return *this;
  }

  /// Set GPU function
  TaskBuilder &gpu(Task::GpuFunction func) {
    gpu_func_ = std::move(func);
    return *this;
  }

  /// Add dependency on another task
  TaskBuilder &after(TaskId id) {
    dependencies_.push_back(id);
    return *this;
  }

  /// Add dependency on another task
  TaskBuilder &after(const std::shared_ptr<Task> &task) {
    dependencies_.push_back(task->id());
    return *this;
  }

  /// Build and add task to graph
  std::shared_ptr<Task> build() {
    auto task = graph_.add_task(device_);

    if (!name_.empty()) {
      task->set_name(name_);
    }

    task->set_priority(priority_);

    if (cpu_func_) {
      task->set_cpu_function(std::move(cpu_func_));
    }

    if (gpu_func_) {
      task->set_gpu_function(std::move(gpu_func_));
    }

    for (TaskId dep : dependencies_) {
      graph_.add_dependency(dep, task->id());
    }

    // Reset for next build
    reset();

    return task;
  }

private:
  void reset() {
    name_.clear();
    device_ = DeviceType::Any;
    priority_ = TaskPriority::Normal;
    cpu_func_ = nullptr;
    gpu_func_ = nullptr;
    dependencies_.clear();
  }

  TaskGraph &graph_;
  std::string name_;
  DeviceType device_ = DeviceType::Any;
  TaskPriority priority_ = TaskPriority::Normal;
  Task::CpuFunction cpu_func_;
  Task::GpuFunction gpu_func_;
  std::vector<TaskId> dependencies_;
};

} // namespace hts
