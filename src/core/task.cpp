#include "hts/task.hpp"
#include <stdexcept>

namespace hts {

Task::Task(TaskId id, DeviceType preferred_device)
    : definition_{id, preferred_device, TaskPriority::Normal, ""},
      runtime_state_{TaskState::Pending,
                     preferred_device == DeviceType::Any ? DeviceType::CPU : preferred_device,
                     std::chrono::nanoseconds{0}, false} {}

void Task::set_cpu_function(CpuFunction func) {
    std::lock_guard<std::mutex> lock(func_mutex_);
    cpu_func_ = std::move(func);
}

void Task::set_gpu_function(GpuFunction func) {
    std::lock_guard<std::mutex> lock(func_mutex_);
    gpu_func_ = std::move(func);
}

void Task::execute_cpu(TaskContext &ctx) {
    std::lock_guard<std::mutex> lock(func_mutex_);
    if (!cpu_func_) {
        throw std::runtime_error("Task " + std::to_string(id()) + " has no CPU function");
    }
    cpu_func_(ctx);
}

void Task::execute_gpu(TaskContext &ctx, cudaStream_t stream) {
    std::lock_guard<std::mutex> lock(func_mutex_);
    if (!gpu_func_) {
        throw std::runtime_error("Task " + std::to_string(id()) + " has no GPU function");
    }
    gpu_func_(ctx, stream);
}

void Task::mark_ready() {
    std::lock_guard<std::mutex> lock(runtime_mutex_);
    runtime_state_.state = TaskState::Ready;
}

void Task::mark_running(DeviceType actual_device) {
    std::lock_guard<std::mutex> lock(runtime_mutex_);
    runtime_state_.actual_device = actual_device;
    runtime_state_.state = TaskState::Running;
}

void Task::mark_completed(std::chrono::nanoseconds execution_time) {
    std::lock_guard<std::mutex> lock(runtime_mutex_);
    runtime_state_.execution_time = execution_time;
    runtime_state_.state = TaskState::Completed;
}

void Task::mark_failed() {
    std::lock_guard<std::mutex> lock(runtime_mutex_);
    runtime_state_.state = TaskState::Failed;
}

void Task::mark_blocked() {
    std::lock_guard<std::mutex> lock(runtime_mutex_);
    runtime_state_.state = TaskState::Blocked;
}

void Task::mark_cancelled() {
    std::lock_guard<std::mutex> lock(runtime_mutex_);
    runtime_state_.cancelled = true;
    runtime_state_.state = TaskState::Cancelled;
}

} // namespace hts
