# Scheduling Policies

> Understanding and configuring task scheduling strategies

---

## Table of Contents

- [Overview](#overview)
- [Built-in Policies](#built-in-policies)
- [Policy Selection Guide](#policy-selection-guide)
- [Custom Policies](#custom-policies)
- [Performance Tuning](#performance-tuning)
- [Examples](#examples)

---

## Overview

HTS uses a pluggable scheduling policy system to determine which device (CPU or GPU) executes each task. The policy receives task characteristics and system status, then makes an execution decision.

### Policy Interface

```cpp
class SchedulingPolicy {
public:
    virtual ~SchedulingPolicy() = default;
    
    virtual DeviceType select_device(
        const Task& task,
        const SystemStatus& status
    ) = 0;
    
    virtual std::string name() const = 0;
};
```

### System Status

Policies receive current system state:

```cpp
struct SystemStatus {
    // CPU status
    size_t cpu_queue_depth;           // Pending CPU tasks
    double cpu_utilization;           // CPU utilization (0-1)
    
    // GPU status
    size_t gpu_queue_depth;           // Pending GPU tasks
    double gpu_utilization;           // GPU utilization (0-1)
    size_t free_gpu_memory;           // Available GPU memory
    
    // Task history
    double avg_cpu_task_time;         // Average CPU task duration
    double avg_gpu_task_time;         // Average GPU task duration
};
```

---

## Built-in Policies

### 1. DefaultSchedulingPolicy

**Description**: Makes decisions based on current system load and queue depth.

**Algorithm**:
1. If task specifies CPU or GPU, respect that preference
2. Compare queue depths (shorter queue wins)
3. Consider historical task performance

**Best for**: General-purpose workloads with mixed CPU/GPU tasks

```cpp
scheduler.set_policy(std::make_unique<DefaultSchedulingPolicy>());
```

### 2. GpuFirstPolicy

**Description**: Prefers GPU execution when available.

**Algorithm**:
1. Use GPU if task has GPU function
2. Fall back to CPU only if GPU unavailable

**Best for**: GPU-bound workloads, compute-intensive tasks

```cpp
scheduler.set_policy(std::make_unique<GpuFirstPolicy>());
```

**Characteristics**:
- Maximizes GPU utilization
- May leave CPU underutilized
- Good for deep learning inference, image processing

### 3. CpuFirstPolicy

**Description**: Prefers CPU execution when possible.

**Algorithm**:
1. Use CPU if task has CPU function
2. Use GPU only for tasks without CPU implementation

**Best for**: CPU-bound workloads, latency-sensitive tasks

```cpp
scheduler.set_policy(std::make_unique<CpuFirstPolicy>());
```

**Characteristics**:
- Preserves GPU memory
- Avoids CPU→GPU data transfer overhead
- Good for control logic, preprocessing

### 4. RoundRobinPolicy

**Description**: Alternates between CPU and GPU for load balancing.

**Algorithm**:
1. Track last device used
2. Pick opposite device for next task
3. Respect task device constraints

**Best for**: Balanced utilization requirements

```cpp
scheduler.set_policy(std::make_unique<RoundRobinPolicy>());
```

**Characteristics**:
- Even distribution of work
- Prevents either device from being idle
- May cause unnecessary data transfers

### 5. ShortestJobFirstPolicy

**Description**: Prioritizes tasks based on estimated completion time.

**Algorithm**:
1. Use historical data to estimate execution time
2. Pick device with shorter expected completion

**Best for**: Latency-sensitive, heterogeneous task durations

```cpp
scheduler.set_policy(std::make_unique<ShortestJobFirstPolicy>());
```

**Characteristics**:
- Minimizes makespan
- Requires historical data (warmup period)
- Good for real-time processing

---

## Policy Selection Guide

### Decision Tree

```
Workload Type?
│
├─> Primarily GPU computation (ML inference, image processing)
│   └─> GpuFirstPolicy
│
├─> Primarily CPU computation (parsing, control flow)
│   └─> CpuFirstPolicy
│
├─> Mixed CPU/GPU pipeline
│   └─> Need balanced utilization?
│       ├─> Yes → RoundRobinPolicy
│       └─> No → DefaultSchedulingPolicy
│
└─> Latency-sensitive, varying task sizes
    └─> ShortestJobFirstPolicy
```

### Workload-Specific Recommendations

| Workload Type | Recommended Policy | Rationale |
|---------------|-------------------|-----------|
| Deep Learning Inference | GpuFirstPolicy | Maximize GPU throughput |
| Video Processing | DefaultSchedulingPolicy | Balance decode (CPU) and filter (GPU) |
| Data Pipeline | CpuFirstPolicy | Minimize transfers, CPU preprocessing |
| Real-time System | ShortestJobFirstPolicy | Meet latency requirements |
| Hybrid Compute | RoundRobinPolicy | Even resource utilization |

---

## Custom Policies

### Basic Custom Policy

```cpp
#include <hts/scheduling_policy.hpp>

class MyCustomPolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task, 
                             const SystemStatus& status) override {
        // Always use GPU for high-priority tasks
        if (task.priority() == TaskPriority::Critical) {
            return DeviceType::GPU;
        }
        
        // Use CPU if GPU is overloaded
        if (status.gpu_queue_depth > 10) {
            return DeviceType::CPU;
        }
        
        // Default to GPU
        return DeviceType::GPU;
    }
    
    std::string name() const override {
        return "MyCustomPolicy";
    }
};

// Usage
scheduler.set_policy(std::make_unique<MyCustomPolicy>());
```

### Data-Aware Policy

```cpp
class DataAwarePolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        // Check if data is already on GPU
        if (task.has_gpu_input()) {
            return DeviceType::GPU;  // Stay on GPU
        }
        
        // For small data, CPU might be faster
        if (task.estimated_data_size() < 1024) {
            return DeviceType::CPU;
        }
        
        // Large data: GPU is usually better
        return DeviceType::GPU;
    }
    
    std::string name() const override {
        return "DataAwarePolicy";
    }
};
```

### Historical Performance Policy

```cpp
class HistoricalPerformancePolicy : public SchedulingPolicy {
    std::unordered_map<TaskId, std::pair<double, double>> history_;
    
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        auto it = history_.find(task.id());
        if (it != history_.end()) {
            double cpu_time = it->second.first;
            double gpu_time = it->second.second;
            
            // Pick faster device based on history
            return (cpu_time < gpu_time) ? DeviceType::CPU : DeviceType::GPU;
        }
        
        // No history: try both and record
        return DeviceType::Any;
    }
    
    void record_performance(TaskId id, DeviceType device, double time) {
        auto& [cpu_time, gpu_time] = history_[id];
        if (device == DeviceType::CPU) {
            cpu_time = time;
        } else {
            gpu_time = time;
        }
    }
    
    std::string name() const override {
        return "HistoricalPerformancePolicy";
    }
};
```

---

## Performance Tuning

### Measuring Policy Effectiveness

```cpp
scheduler.set_profiling(true);
scheduler.execute();

auto summary = scheduler.profiler().generate_summary();

std::cout << "Total time: " << summary.total_time.count() / 1e6 << " ms\n";
std::cout << "Parallelism: " << summary.parallelism << "x\n";
std::cout << "CPU tasks: " << summary.cpu_task_count << "\n";
std::cout << "GPU tasks: " << summary.gpu_task_count << "\n";
```

### A/B Testing Policies

```cpp
void benchmark_policy(const std::string& name, 
                      std::unique_ptr<SchedulingPolicy> policy) {
    static TaskGraph template_graph = create_benchmark_graph();
    
    Scheduler scheduler;
    scheduler.graph() = template_graph;  // Copy graph
    scheduler.set_policy(std::move(policy));
    scheduler.set_profiling(true);
    
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.execute();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << name << ": " << duration.count() << " ms\n";
}

// Compare policies
benchmark_policy("Default", std::make_unique<DefaultSchedulingPolicy>());
benchmark_policy("GPU First", std::make_unique<GpuFirstPolicy>());
benchmark_policy("Round Robin", std::make_unique<RoundRobinPolicy>());
```

---

## Examples

### Example 1: Dynamic Policy Switching

```cpp
Scheduler scheduler;

// Phase 1: GPU-intensive processing
scheduler.set_policy(std::make_unique<GpuFirstPolicy>());
// ... add GPU tasks ...
scheduler.execute();
scheduler.reset();

// Phase 2: CPU-intensive post-processing
scheduler.set_policy(std::make_unique<CpuFirstPolicy>());
// ... add CPU tasks ...
scheduler.execute();
```

### Example 2: Priority-Based Policy

```cpp
class PriorityAwarePolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        switch (task.priority()) {
            case TaskPriority::Critical:
                // Critical tasks get the fastest available device
                return (status.gpu_queue_depth < status.cpu_queue_depth) 
                       ? DeviceType::GPU : DeviceType::CPU;
                       
            case TaskPriority::High:
                // High priority: prefer GPU unless overloaded
                return (status.gpu_queue_depth < 5) 
                       ? DeviceType::GPU : DeviceType::CPU;
                       
            case TaskPriority::Normal:
                // Normal: load balance
                return (status.cpu_queue_depth < status.gpu_queue_depth)
                       ? DeviceType::CPU : DeviceType::GPU;
                       
            case TaskPriority::Low:
                // Low priority: use whatever is least loaded
                return (status.cpu_utilization < status.gpu_utilization)
                       ? DeviceType::CPU : DeviceType::GPU;
        }
        return DeviceType::Any;
    }
    
    std::string name() const override {
        return "PriorityAwarePolicy";
    }
};

// Usage with priority-based scheduling
scheduler.set_policy(std::make_unique<PriorityAwarePolicy>());

critical_task->set_priority(TaskPriority::Critical);
background_task->set_priority(TaskPriority::Low);
```

### Example 3: Memory-Aware Policy

```cpp
class MemoryAwarePolicy : public SchedulingPolicy {
    static constexpr size_t GPU_MEMORY_THRESHOLD = 1024 * 1024 * 1024;  // 1GB
    
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        // Large memory requirements: check availability
        if (task.estimated_memory() > GPU_MEMORY_THRESHOLD / 2) {
            if (status.free_gpu_memory < task.estimated_memory()) {
                // Not enough GPU memory, use CPU
                return DeviceType::CPU;
            }
        }
        
        // Normal decision based on load
        return (status.gpu_queue_depth <= status.cpu_queue_depth)
               ? DeviceType::GPU : DeviceType::CPU;
    }
    
    std::string name() const override {
        return "MemoryAwarePolicy";
    }
};
```

---

## Best Practices

1. **Start with Default**: Begin with `DefaultSchedulingPolicy` and measure
2. **Profile First**: Don't guess; use profiler data to guide decisions
3. **Consider Data Locality**: Minimize CPU↔GPU data transfers
4. **Respect Task Hints**: If task specifies device, override only when necessary
5. **Monitor Queue Depths**: Prevent either device from being starved
6. **Account for Warmup**: GPU tasks may have initial overhead

---

## Further Reading

- [Architecture Overview](architecture.md) - System design
- [Performance Profiling](profiling.md) - Measuring performance
- [API Reference](api-reference.md) - Policy API details
