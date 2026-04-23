#pragma once

#include "hts/task_graph.hpp"
#include "hts/types.hpp"
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace hts {

/// DependencyManager tracks task completion and dependency satisfaction
class DependencyManager {
  public:
    /// Construct from a task graph
    explicit DependencyManager(const TaskGraph &graph);

    /// Mark a task as completed
    /// @param id Task ID
    void mark_completed(TaskId id);

    /// Mark a task as failed (blocks all dependents)
    /// @param id Task ID
    void mark_failed(TaskId id);

    /// Check if a task is ready to execute
    /// @param id Task ID
    /// @return true if all dependencies are satisfied
    bool is_ready(TaskId id) const;

    /// Get all tasks that are ready to execute
    /// @return Vector of ready task IDs
    std::vector<TaskId> get_ready_tasks() const;

    /// Get number of pending dependencies for a task
    /// @param id Task ID
    /// @return Number of unsatisfied dependencies
    size_t pending_dependency_count(TaskId id) const;

    /// Check if a task is completed
    /// @param id Task ID
    bool is_completed(TaskId id) const;

    /// Check if a task is failed
    /// @param id Task ID
    bool is_failed(TaskId id) const;

    /// Check if a task is blocked (due to upstream failure)
    /// @param id Task ID
    bool is_blocked(TaskId id) const;

    /// Reset all tracking state
    void reset();

    /// Get all completed tasks
    const std::unordered_set<TaskId> &completed_tasks() const { return completed_; }

    /// Get all failed tasks
    const std::unordered_set<TaskId> &failed_tasks() const { return failed_; }

    /// Get all blocked tasks
    const std::unordered_set<TaskId> &blocked_tasks() const { return blocked_; }

  private:
    const TaskGraph &graph_;

    // Pending dependency count for each task
    std::unordered_map<TaskId, size_t> pending_deps_;

    // Task states
    std::unordered_set<TaskId> completed_;
    std::unordered_set<TaskId> failed_;
    std::unordered_set<TaskId> blocked_;

    mutable std::mutex mutex_;

    /// Block all tasks that depend on the given task (recursively)
    void block_dependents(TaskId id);

    /// Initialize pending dependency counts
    void init_pending_counts();
};

} // namespace hts
