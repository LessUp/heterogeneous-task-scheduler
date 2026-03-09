#pragma once

#include "hts/task.hpp"
#include "hts/task_graph.hpp"
#include "hts/types.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace hts {

/// TaskGroup manages a collection of related tasks
class TaskGroup {
public:
  using TaskPtr = std::shared_ptr<Task>;

  /// Create a task group with a name
  explicit TaskGroup(const std::string &name, TaskGraph &graph);

  /// Get group name
  const std::string &name() const { return name_; }

  /// Add a new task to the group
  TaskPtr add_task(DeviceType device = DeviceType::Any);

  /// Add an existing task to the group
  void add_task(TaskPtr task);

  /// Get all tasks in the group
  const std::vector<TaskPtr> &tasks() const { return tasks_; }

  /// Get number of tasks
  size_t size() const { return tasks_.size(); }

  /// Check if group is empty
  bool empty() const { return tasks_.empty(); }

  /// Make all tasks in this group depend on another task
  void depends_on(TaskId id);
  void depends_on(TaskPtr task);
  void depends_on(const TaskGroup &other);

  /// Make another task depend on all tasks in this group
  void then(TaskId id);
  void then(TaskPtr task);
  void then(TaskGroup &other);

  /// Apply a function to all tasks
  void for_each(std::function<void(TaskPtr)> func);

  /// Set priority for all tasks
  void set_priority(TaskPriority priority);

  /// Set device for all tasks
  void set_device(DeviceType device);

  /// Cancel all tasks in the group
  void cancel();

  /// Check if all tasks are completed
  bool all_completed() const;

  /// Check if any task failed
  bool any_failed() const;

  /// Get completed task count
  size_t completed_count() const;

private:
  std::string name_;
  TaskGraph &graph_;
  std::vector<TaskPtr> tasks_;
};

} // namespace hts
