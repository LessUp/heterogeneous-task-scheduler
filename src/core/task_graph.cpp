#include "hts/task_graph.hpp"
#include <algorithm>
#include <queue>
#include <stdexcept>

namespace hts {

TaskGraph::TaskPtr TaskGraph::add_task(DeviceType device) {
    std::lock_guard<std::mutex> lock(mutex_);

    TaskId id = next_id_++;
    auto task = std::make_shared<Task>(id, device);

    tasks_[id] = task;
    adjacency_list_[id] = {};
    reverse_adjacency_[id] = {};

    return task;
}

bool TaskGraph::add_dependency(TaskId from, TaskId to) {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check tasks exist
    if (tasks_.find(from) == tasks_.end() || tasks_.find(to) == tasks_.end()) {
        throw std::invalid_argument("Task ID not found");
    }

    // Check for self-loop
    if (from == to) {
        return false;
    }

    // Check if dependency already exists
    auto &successors = adjacency_list_[from];
    if (std::find(successors.begin(), successors.end(), to) != successors.end()) {
        return true; // Already exists, not an error
    }

    // Check if adding this edge would create a cycle
    // A cycle would be created if there's already a path from 'to' to 'from'
    if (would_create_cycle(from, to)) {
        return false;
    }

    // Add the dependency
    adjacency_list_[from].push_back(to);
    reverse_adjacency_[to].push_back(from);

    return true;
}

bool TaskGraph::would_create_cycle(TaskId from, TaskId to) const {
    // If there's a path from 'to' to 'from', adding from->to creates a cycle
    std::unordered_set<TaskId> visited;
    return dfs_has_path(to, from, visited);
}

bool TaskGraph::dfs_has_path(TaskId start, TaskId target,
                             std::unordered_set<TaskId> &visited) const {
    if (start == target) {
        return true;
    }

    if (visited.count(start)) {
        return false;
    }
    visited.insert(start);

    auto it = adjacency_list_.find(start);
    if (it != adjacency_list_.end()) {
        for (TaskId next : it->second) {
            if (dfs_has_path(next, target, visited)) {
                return true;
            }
        }
    }

    return false;
}

bool TaskGraph::validate() const {
    std::lock_guard<std::mutex> lock(mutex_);

    // Check for cycles using topological sort (Kahn's algorithm)
    std::unordered_map<TaskId, size_t> in_degrees;
    for (const auto &[id, _] : tasks_) {
        in_degrees[id] = 0;
    }

    for (const auto &[from, tos] : adjacency_list_) {
        for (TaskId to : tos) {
            in_degrees[to]++;
        }
    }

    std::queue<TaskId> queue;
    for (const auto &[id, degree] : in_degrees) {
        if (degree == 0) {
            queue.push(id);
        }
    }

    size_t processed = 0;
    while (!queue.empty()) {
        TaskId current = queue.front();
        queue.pop();
        processed++;

        auto it = adjacency_list_.find(current);
        if (it != adjacency_list_.end()) {
            for (TaskId next : it->second) {
                if (--in_degrees[next] == 0) {
                    queue.push(next);
                }
            }
        }
    }

    return processed == tasks_.size();
}

std::vector<TaskGraph::TaskPtr> TaskGraph::topological_sort() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<TaskPtr> result;
    result.reserve(tasks_.size());

    // Kahn's algorithm
    std::unordered_map<TaskId, size_t> in_degrees;
    for (const auto &[id, _] : tasks_) {
        in_degrees[id] = 0;
    }

    for (const auto &[from, tos] : adjacency_list_) {
        for (TaskId to : tos) {
            in_degrees[to]++;
        }
    }

    std::queue<TaskId> queue;
    for (const auto &[id, degree] : in_degrees) {
        if (degree == 0) {
            queue.push(id);
        }
    }

    while (!queue.empty()) {
        TaskId current = queue.front();
        queue.pop();

        result.push_back(tasks_.at(current));

        auto it = adjacency_list_.find(current);
        if (it != adjacency_list_.end()) {
            for (TaskId next : it->second) {
                if (--in_degrees[next] == 0) {
                    queue.push(next);
                }
            }
        }
    }

    return result;
}

std::vector<TaskGraph::TaskPtr> TaskGraph::get_root_tasks() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<TaskPtr> roots;
    for (const auto &[id, task] : tasks_) {
        auto it = reverse_adjacency_.find(id);
        if (it == reverse_adjacency_.end() || it->second.empty()) {
            roots.push_back(task);
        }
    }
    return roots;
}

std::vector<TaskGraph::TaskPtr> TaskGraph::get_successors(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<TaskPtr> successors;
    auto it = adjacency_list_.find(id);
    if (it != adjacency_list_.end()) {
        for (TaskId succ_id : it->second) {
            auto task_it = tasks_.find(succ_id);
            if (task_it != tasks_.end()) {
                successors.push_back(task_it->second);
            }
        }
    }
    return successors;
}

std::vector<TaskGraph::TaskPtr> TaskGraph::get_predecessors(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<TaskPtr> predecessors;
    auto it = reverse_adjacency_.find(id);
    if (it != reverse_adjacency_.end()) {
        for (TaskId pred_id : it->second) {
            auto task_it = tasks_.find(pred_id);
            if (task_it != tasks_.end()) {
                predecessors.push_back(task_it->second);
            }
        }
    }
    return predecessors;
}

TaskGraph::TaskPtr TaskGraph::get_task(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = tasks_.find(id);
    return (it != tasks_.end()) ? it->second : nullptr;
}

void TaskGraph::clear() {
    std::lock_guard<std::mutex> lock(mutex_);

    tasks_.clear();
    adjacency_list_.clear();
    reverse_adjacency_.clear();
    next_id_ = 1;
}

size_t TaskGraph::in_degree(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = reverse_adjacency_.find(id);
    return (it != reverse_adjacency_.end()) ? it->second.size() : 0;
}

size_t TaskGraph::out_degree(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = adjacency_list_.find(id);
    return (it != adjacency_list_.end()) ? it->second.size() : 0;
}

} // namespace hts
