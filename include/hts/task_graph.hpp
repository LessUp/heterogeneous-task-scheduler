#pragma once

#include "hts/task.hpp"
#include "hts/types.hpp"
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hts {

/// TaskGraph manages tasks and their dependencies as a DAG
class TaskGraph {
public:
  using TaskPtr = std::shared_ptr<Task>;

  TaskGraph() = default;
  ~TaskGraph() = default;

  // Non-copyable, movable
  TaskGraph(const TaskGraph &) = delete;
  TaskGraph &operator=(const TaskGraph &) = delete;
  TaskGraph(TaskGraph &&) = default;
  TaskGraph &operator=(TaskGraph &&) = default;

  /// Add a new task to the graph
  /// @param device Preferred execution device
  /// @return Shared pointer to the created task
  TaskPtr add_task(DeviceType device = DeviceType::Any);

  /// Add a dependency: 'to' depends on 'from'
  /// @param from Task that must complete first
  /// @param to Task that depends on 'from'
  /// @return true if dependency added, false if it would create a cycle
  bool add_dependency(TaskId from, TaskId to);

  /// Validate the graph is a valid DAG
  /// @return true if graph is valid
  bool validate() const;

  /// Get topological sort of tasks
  /// @return Tasks in topological order
  std::vector<TaskPtr> topological_sort() const;

  /// Get root tasks (no dependencies)
  /// @return Tasks with no predecessors
  std::vector<TaskPtr> get_root_tasks() const;

  /// Get successors of a task
  /// @param id Task ID
  /// @return Tasks that depend on the given task
  std::vector<TaskPtr> get_successors(TaskId id) const;

  /// Get predecessors of a task
  /// @param id Task ID
  /// @return Tasks that the given task depends on
  std::vector<TaskPtr> get_predecessors(TaskId id) const;

  /// Get task by ID
  /// @param id Task ID
  /// @return Task pointer or nullptr if not found
  TaskPtr get_task(TaskId id) const;

  /// Get all tasks
  /// @return Map of all tasks
  const std::unordered_map<TaskId, TaskPtr> &tasks() const { return tasks_; }

  /// Get number of tasks
  size_t size() const { return tasks_.size(); }

  /// Check if graph is empty
  bool empty() const { return tasks_.empty(); }

  /// Clear all tasks and dependencies
  void clear();

  /// Get in-degree of a task (number of dependencies)
  size_t in_degree(TaskId id) const;

  /// Get out-degree of a task (number of dependents)
  size_t out_degree(TaskId id) const;

private:
  std::unordered_map<TaskId, TaskPtr> tasks_;
  std::unordered_map<TaskId, std::vector<TaskId>>
      adjacency_list_; // from -> [to]
  std::unordered_map<TaskId, std::vector<TaskId>>
      reverse_adjacency_; // to -> [from]

  std::atomic<TaskId> next_id_{1};
  mutable std::mutex mutex_;

  /// Check if adding edge would create a cycle using DFS
  bool would_create_cycle(TaskId from, TaskId to) const;

  /// DFS helper for cycle detection
  bool dfs_has_path(TaskId start, TaskId target,
                    std::unordered_set<TaskId> &visited) const;
};

} // namespace hts
