#include "hts/task.hpp"
#include <stdexcept>

namespace hts {

Task::Task(TaskId id, DeviceType preferred_device)
    : id_(id), preferred_device_(preferred_device),
      actual_device_(preferred_device == DeviceType::Any ? DeviceType::CPU
                                                         : preferred_device) {}

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
    throw std::runtime_error("Task " + std::to_string(id_) +
                             " has no CPU function");
  }
  cpu_func_(ctx);
}

void Task::execute_gpu(TaskContext &ctx, cudaStream_t stream) {
  std::lock_guard<std::mutex> lock(func_mutex_);
  if (!gpu_func_) {
    throw std::runtime_error("Task " + std::to_string(id_) +
                             " has no GPU function");
  }
  gpu_func_(ctx, stream);
}

} // namespace hts
