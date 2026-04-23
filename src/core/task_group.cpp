#include "hts/task_group.hpp"

namespace hts {

TaskGroup::TaskGroup(const std::string &name, TaskGraph &graph) : name_(name), graph_(graph) {}

TaskGroup::TaskPtr TaskGroup::add_task(DeviceType device) {
    auto task = graph_.add_task(device);
    tasks_.push_back(task);
    return task;
}

void TaskGroup::add_task(TaskPtr task) {
    tasks_.push_back(task);
}

void TaskGroup::depends_on(TaskId id) {
    for (auto &task : tasks_) {
        graph_.add_dependency(id, task->id());
    }
}

void TaskGroup::depends_on(TaskPtr task) {
    depends_on(task->id());
}

void TaskGroup::depends_on(const TaskGroup &other) {
    for (const auto &dep : other.tasks_) {
        for (auto &task : tasks_) {
            graph_.add_dependency(dep->id(), task->id());
        }
    }
}

void TaskGroup::then(TaskId id) {
    for (auto &task : tasks_) {
        graph_.add_dependency(task->id(), id);
    }
}

void TaskGroup::then(TaskPtr task) {
    then(task->id());
}

void TaskGroup::then(TaskGroup &other) {
    for (auto &task : tasks_) {
        for (auto &dep : other.tasks_) {
            graph_.add_dependency(task->id(), dep->id());
        }
    }
}

void TaskGroup::for_each(std::function<void(TaskPtr)> func) {
    for (auto &task : tasks_) {
        func(task);
    }
}

void TaskGroup::set_priority(TaskPriority priority) {
    for (auto &task : tasks_) {
        task->set_priority(priority);
    }
}

void TaskGroup::set_device(DeviceType device) {
    for (auto &task : tasks_) {
        // Only override tasks that don't have a specific device preference
        if (task->preferred_device() == DeviceType::Any) {
            task->set_preferred_device(device);
        }
    }
}

void TaskGroup::cancel() {
    for (auto &task : tasks_) {
        task->cancel();
    }
}

bool TaskGroup::all_completed() const {
    for (const auto &task : tasks_) {
        if (task->state() != TaskState::Completed) {
            return false;
        }
    }
    return !tasks_.empty();
}

bool TaskGroup::any_failed() const {
    for (const auto &task : tasks_) {
        TaskState state = task->state();
        if (state == TaskState::Failed || state == TaskState::Blocked) {
            return true;
        }
    }
    return false;
}

size_t TaskGroup::completed_count() const {
    size_t count = 0;
    for (const auto &task : tasks_) {
        if (task->state() == TaskState::Completed) {
            ++count;
        }
    }
    return count;
}

} // namespace hts
