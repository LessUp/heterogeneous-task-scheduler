#include "hts/dependency_manager.hpp"
#include <queue>

namespace hts {

DependencyManager::DependencyManager(const TaskGraph &graph) {
    init_from_graph(graph);
}

void DependencyManager::init_from_graph(const TaskGraph &graph) {
    std::lock_guard<std::mutex> lock(mutex_);

    successors_.clear();
    pending_deps_.clear();
    completed_.clear();
    failed_.clear();
    blocked_.clear();

    // Snapshot successors and compute in-degrees (pending deps).
    for (const auto &[id, task] : graph.tasks()) {
        successors_[id] = {};
        pending_deps_[id] = 0;
    }

    for (const auto &[from, task_ptr] : graph.tasks()) {
        (void)task_ptr;
        for (const auto &succ_task : graph.get_successors(from)) {
            TaskId succ = succ_task->id();
            successors_[from].push_back(succ);
            pending_deps_[succ]++;
        }
    }
}

std::vector<TaskId> DependencyManager::mark_completed(TaskId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (completed_.count(id) || failed_.count(id) || blocked_.count(id)) {
        return {}; // Already in terminal state
    }

    completed_.insert(id);

    // Decrement pending dependency count for all successors.
    std::vector<TaskId> newly_ready;
    auto it = successors_.find(id);
    if (it != successors_.end()) {
        for (TaskId succ_id : it->second) {
            auto pit = pending_deps_.find(succ_id);
            if (pit != pending_deps_.end() && pit->second > 0) {
                if (--pit->second == 0) {
                    newly_ready.push_back(succ_id);
                }
            }
        }
    }
    return newly_ready;
}

std::vector<TaskId> DependencyManager::mark_failed(TaskId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (completed_.count(id) || failed_.count(id) || blocked_.count(id)) {
        return {}; // Already in terminal state
    }

    failed_.insert(id);

    // BFS to block all transitive dependents.
    std::vector<TaskId> blocked;
    std::queue<TaskId> queue;

    auto it = successors_.find(id);
    if (it != successors_.end()) {
        for (TaskId succ_id : it->second) {
            queue.push(succ_id);
        }
    }

    while (!queue.empty()) {
        TaskId current = queue.front();
        queue.pop();

        if (blocked_.count(current) || completed_.count(current) || failed_.count(current)) {
            continue;
        }

        blocked_.insert(current);
        blocked.push_back(current);

        auto sit = successors_.find(current);
        if (sit != successors_.end()) {
            for (TaskId succ_id : sit->second) {
                queue.push(succ_id);
            }
        }
    }

    return blocked;
}

bool DependencyManager::is_ready(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    if (completed_.count(id) || failed_.count(id) || blocked_.count(id)) {
        return false;
    }

    auto it = pending_deps_.find(id);
    return it != pending_deps_.end() && it->second == 0;
}

std::vector<TaskId> DependencyManager::get_ready_tasks() const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<TaskId> ready;
    for (const auto &[id, count] : pending_deps_) {
        if (count == 0 && !completed_.count(id) && !failed_.count(id) && !blocked_.count(id)) {
            ready.push_back(id);
        }
    }
    return ready;
}

size_t DependencyManager::pending_dependency_count(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    auto it = pending_deps_.find(id);
    return (it != pending_deps_.end()) ? it->second : 0;
}

bool DependencyManager::is_completed(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return completed_.count(id) > 0;
}

bool DependencyManager::is_failed(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return failed_.count(id) > 0;
}

bool DependencyManager::is_blocked(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return blocked_.count(id) > 0;
}

void DependencyManager::reset() {
    std::lock_guard<std::mutex> lock(mutex_);

    completed_.clear();
    failed_.clear();
    blocked_.clear();

    for (auto &[id, count] : pending_deps_) {
        count = 0;
        (void)id;
    }

    // Recompute pending_deps from the snapshot.
    for (const auto &[from, tos] : successors_) {
        (void)from;
        for (TaskId to : tos) {
            pending_deps_[to]++;
        }
    }
}

} // namespace hts
