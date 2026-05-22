#include "hts/scheduling_policy.hpp"

#include "hts/task.hpp"

#include <algorithm>

namespace hts {

namespace {

void prioritize_by_priority(std::vector<std::shared_ptr<Task>> &tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
        return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
}

} // namespace

DeviceType DefaultSchedulingPolicy::select_device(const Task &task, double cpu_load,
                                                  double gpu_load) {
    DeviceType preferred = task.preferred_device();
    if (preferred != DeviceType::Any) {
        return preferred;
    }

    if (task.has_gpu_function() && task.has_cpu_function()) {
        return (gpu_load <= cpu_load) ? DeviceType::GPU : DeviceType::CPU;
    }
    if (task.has_gpu_function()) {
        return DeviceType::GPU;
    }
    if (task.has_cpu_function()) {
        return DeviceType::CPU;
    }

    return DeviceType::CPU;
}

void DefaultSchedulingPolicy::prioritize(std::vector<std::shared_ptr<Task>> &tasks) {
    prioritize_by_priority(tasks);
}

const char *DefaultSchedulingPolicy::name() const {
    return "Default";
}

DeviceType GpuFirstPolicy::select_device(const Task &task, double cpu_load, double gpu_load) {
    (void)cpu_load;
    (void)gpu_load;

    if (task.preferred_device() == DeviceType::CPU) {
        return DeviceType::CPU;
    }
    if (task.has_gpu_function()) {
        return DeviceType::GPU;
    }
    return DeviceType::CPU;
}

void GpuFirstPolicy::prioritize(std::vector<std::shared_ptr<Task>> &tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
        bool a_gpu = a->preferred_device() == DeviceType::GPU ||
                     (a->preferred_device() == DeviceType::Any && a->has_gpu_function());
        bool b_gpu = b->preferred_device() == DeviceType::GPU ||
                     (b->preferred_device() == DeviceType::Any && b->has_gpu_function());

        if (a_gpu != b_gpu) {
            return a_gpu > b_gpu;
        }
        return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
}

const char *GpuFirstPolicy::name() const {
    return "GPU-First";
}

DeviceType CpuFirstPolicy::select_device(const Task &task, double cpu_load, double gpu_load) {
    (void)cpu_load;
    (void)gpu_load;

    if (task.preferred_device() == DeviceType::GPU) {
        return DeviceType::GPU;
    }
    if (task.has_cpu_function()) {
        return DeviceType::CPU;
    }
    return DeviceType::GPU;
}

void CpuFirstPolicy::prioritize(std::vector<std::shared_ptr<Task>> &tasks) {
    std::sort(tasks.begin(), tasks.end(), [](const auto &a, const auto &b) {
        bool a_cpu = a->preferred_device() == DeviceType::CPU ||
                     (a->preferred_device() == DeviceType::Any && a->has_cpu_function());
        bool b_cpu = b->preferred_device() == DeviceType::CPU ||
                     (b->preferred_device() == DeviceType::Any && b->has_cpu_function());

        if (a_cpu != b_cpu) {
            return a_cpu > b_cpu;
        }
        return static_cast<int>(a->priority()) > static_cast<int>(b->priority());
    });
}

const char *CpuFirstPolicy::name() const {
    return "CPU-First";
}

DeviceType RoundRobinPolicy::select_device(const Task &task, double cpu_load, double gpu_load) {
    (void)cpu_load;
    (void)gpu_load;

    if (task.preferred_device() != DeviceType::Any) {
        return task.preferred_device();
    }

    bool next_gpu;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        use_gpu_ = !use_gpu_;
        next_gpu = use_gpu_;
    }

    if (next_gpu && task.has_gpu_function()) {
        return DeviceType::GPU;
    }
    if (!next_gpu && task.has_cpu_function()) {
        return DeviceType::CPU;
    }
    if (task.has_gpu_function()) {
        return DeviceType::GPU;
    }
    return DeviceType::CPU;
}

void RoundRobinPolicy::prioritize(std::vector<std::shared_ptr<Task>> &tasks) {
    prioritize_by_priority(tasks);
}

const char *RoundRobinPolicy::name() const {
    return "Round-Robin";
}

DeviceType ShortestJobFirstPolicy::select_device(const Task &task, double cpu_load,
                                                 double gpu_load) {
    if (task.preferred_device() != DeviceType::Any) {
        return task.preferred_device();
    }
    if (task.has_gpu_function() && task.has_cpu_function()) {
        return (gpu_load <= cpu_load) ? DeviceType::GPU : DeviceType::CPU;
    }
    if (task.has_gpu_function()) {
        return DeviceType::GPU;
    }
    return DeviceType::CPU;
}

void ShortestJobFirstPolicy::prioritize(std::vector<std::shared_ptr<Task>> &tasks) {
    prioritize_by_priority(tasks);
}

const char *ShortestJobFirstPolicy::name() const {
    return "Shortest-Job-First";
}

} // namespace hts
