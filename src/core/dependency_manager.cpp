#include "hts/dependency_manager.hpp"
#include <queue>

namespace hts {

DependencyManager::DependencyManager(const TaskGraph &graph) : graph_(graph) {
    init_pending_counts();
}

void DependencyManager::init_pending_counts() {
    std::lock_guard<std::mutex> lock(mutex_);

    pending_deps_.clear();
    completed_.clear();
    failed_.clear();
    blocked_.clear();

    for (const auto &[id, task] : graph_.tasks()) {
        pending_deps_[id] = graph_.in_degree(id);
    }
}

void DependencyManager::mark_completed(TaskId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (completed_.count(id) || failed_.count(id) || blocked_.count(id)) {
        return; // Already in terminal state
    }

    completed_.insert(id);

    // Notify all dependents
    auto successors = graph_.get_successors(id);
    for (const auto &succ : successors) {
        TaskId succ_id = succ->id();
        if (pending_deps_.count(succ_id) && pending_deps_[succ_id] > 0) {
            pending_deps_[succ_id]--;
        }
    }
}

void DependencyManager::mark_failed(TaskId id) {
    std::lock_guard<std::mutex> lock(mutex_);

    if (completed_.count(id) || failed_.count(id) || blocked_.count(id)) {
        return; // Already in terminal state
    }

    failed_.insert(id);

    // Block all dependents recursively
    block_dependents(id);
}

void DependencyManager::block_dependents(TaskId id) {
    // BFS to block all transitive dependents
    std::queue<TaskId> queue;

    auto successors = graph_.get_successors(id);
    for (const auto &succ : successors) {
        queue.push(succ->id());
    }

    while (!queue.empty()) {
        TaskId current = queue.front();
        queue.pop();

        if (blocked_.count(current) || completed_.count(current) || failed_.count(current)) {
            continue;
        }

        blocked_.insert(current);

        // Update task state
        auto task = graph_.get_task(current);
        if (task) {
            task->set_state(TaskState::Blocked);
        }

        // Add successors to queue
        auto current_successors = graph_.get_successors(current);
        for (const auto &succ : current_successors) {
            queue.push(succ->id());
        }
    }
}

bool DependencyManager::is_ready(TaskId id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    // Not ready if already processed or blocked
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
    init_pending_counts();
}

} // namespace hts
