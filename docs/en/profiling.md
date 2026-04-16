# Performance Profiling

> Monitoring and optimizing HTS application performance

---

## Table of Contents

- [Overview](#overview)
- [Enabling Profiling](#enabling-profiling)
- [Profile Summary](#profile-summary)
- [Timeline Visualization](#timeline-visualization)
- [Performance Metrics](#performance-metrics)
- [Optimization Guide](#optimization-guide)
- [Advanced Profiling](#advanced-profiling)

---

## Overview

HTS provides comprehensive profiling capabilities to help you understand and optimize your application performance:

- **Execution timing** for all tasks
- **Parallelism metrics** to measure CPU/GPU utilization
- **Memory usage** tracking
- **Chrome trace** export for visualization
- **Custom events** for application-specific metrics

### Profiler Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     Profiling System                         │
├─────────────────────────────────────────────────────────────┤
│                                                              │
│   Task Execution          Event Recording                    │
│        │                        │                           │
│        ▼                        ▼                           │
│   ┌─────────┐             ┌───────────┐                     │
│   │  Timer  │────────────→│  Events   │                     │
│   └─────────┘             └─────┬─────┘                     │
│                                 │                           │
│                                 ▼                           │
│   ┌─────────────────────────────────────────────────────┐   │
│   │                 Event Buffer                         │   │
│   │  [TaskStart][TaskEnd][MemoryAlloc][StreamSync]...   │   │
│   └─────────────────────────────────────────────────────┘   │
│                                 │                           │
│                    ┌────────────┼────────────┐              │
│                    ▼            ▼            ▼              │
│              ┌─────────┐  ┌─────────┐  ┌─────────┐         │
│              │Summary  │  │Timeline │  │ Custom  │         │
│              │ Report  │  │  JSON   │  │ Metrics │         │
│              └─────────┘  └─────────┘  └─────────┘         │
│                                                              │
└─────────────────────────────────────────────────────────────┘
```

---

## Enabling Profiling

### Basic Usage

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    Scheduler scheduler;
    
    // Enable profiling
    scheduler.set_profiling(true);
    
    // Build and execute your task graph
    // ...
    
    scheduler.execute();
    
    // Access profiler
    Profiler& profiler = scheduler.profiler();
    
    return 0;
}
```

### Configuration

```cpp
SchedulerConfig config;
config.enable_profiling = true;

// Optional: detailed memory tracking (small overhead)
config.detailed_memory_profiling = true;

Scheduler scheduler(config);
```

---

## Profile Summary

### Generate Summary

```cpp
scheduler.execute();

auto summary = scheduler.profiler().generate_summary();

std::cout << "=== Execution Summary ===\n"
          << "Total time: " << summary.total_time.count() / 1e6 << " ms\n"
          << "CPU time: " << summary.cpu_time.count() / 1e6 << " ms\n"
          << "GPU time: " << summary.gpu_time.count() / 1e6 << " ms\n"
          << "Parallelism: " << summary.parallelism << "x\n"
          << "Tasks executed: " << summary.task_count << "\n"
          << "CPU tasks: " << summary.cpu_task_count << "\n"
          << "GPU tasks: " << summary.gpu_task_count << "\n";
```

### Summary Structure

```cpp
struct ProfileSummary {
    // Timing
    std::chrono::nanoseconds total_time;      // Wall clock time
    std::chrono::nanoseconds cpu_time;        // Sum of all CPU task times
    std::chrono::nanoseconds gpu_time;        // Sum of all GPU task times
    
    // Efficiency
    double parallelism;                       // (cpu_time + gpu_time) / total_time
    
    // Counts
    size_t task_count;
    size_t cpu_task_count;
    size_t gpu_task_count;
    size_t completed_count;
    size_t failed_count;
    size_t retried_count;
    
    // Memory
    size_t peak_memory_bytes;
    size_t total_allocations;
    
    // Per-task statistics
    std::vector<TaskProfile> task_profiles;
};
```

### Interpreting Results

| Metric | Good | Poor | Action |
|--------|------|------|--------|
| Parallelism | >2.0 | <1.5 | Add more parallelism |
| CPU/GPU ratio | Balanced | One idle | Adjust scheduling policy |
| Failed tasks | 0% | >5% | Improve error handling |
| GPU utilization | >80% | <50% | Move more work to GPU |

### Detailed Report

```cpp
std::cout << scheduler.profiler().generate_report();
```

Example output:
```
=== HTS Profile Report ===
Generated: 2026-04-16 10:30:15

OVERVIEW
--------
Total time: 125.4 ms
Parallelism: 3.2x
Tasks: 50 (CPU: 30, GPU: 20)

TOP 5 SLOWEST TASKS
-------------------
#1 "HeavyCompute" (GPU) : 45.2 ms
#2 "DataLoad" (CPU)     : 23.1 ms
#3 "Preprocess" (CPU)   : 12.5 ms
#4 "Transform" (GPU)    : 8.7 ms
#5 "Finalize" (CPU)     : 5.3 ms

DEVICE UTILIZATION
------------------
CPU: ████████████████████░░░░░ 78%
GPU: █████████████████░░░░░░░░ 65%

MEMORY
------
Peak usage: 245.8 MB
Allocations: 120
Fragmentation: 2.3%
```

---

## Timeline Visualization

### Chrome Tracing Format

Export profiling data for Chrome's `chrome://tracing`:

```cpp
scheduler.execute();

// Export timeline
scheduler.profiler().export_timeline("profile.json");
```

### Viewing

1. Open Chrome browser
2. Navigate to `chrome://tracing`
3. Click "Load" and select `profile.json`

### Timeline Structure

```json
{
  "traceEvents": [
    {
      "name": "Task1",
      "ph": "B",           // Begin
      "ts": 1000000,       // Timestamp (microseconds)
      "pid": 1,            // Process ID
      "tid": 1,            // Thread/Stream ID
      "cat": "cpu_task"    // Category
    },
    {
      "name": "Task1",
      "ph": "E",           // End
      "ts": 1050000,
      "pid": 1,
      "tid": 1
    },
    {
      "name": "Task2",
      "ph": "B",
      "ts": 1020000,
      "pid": 1,
      "tid": 2,
      "cat": "gpu_task"
    }
  ],
  "displayTimeUnit": "ms"
}
```

### Visual Analysis

```
CPU Thread 1:  [Task1=======]        [Task3===]
CPU Thread 2:           [Task2=======]
GPU Stream 0:  [GPU1==============][GPU2====]

Time →
```

**What to look for:**
- **Gaps**: Indicate synchronization or idle time
- **Overlap**: Shows parallel execution
- **Long tasks**: Optimization candidates

---

## Performance Metrics

### Task-Level Metrics

```cpp
struct TaskProfile {
    TaskId id;
    std::string name;
    DeviceType device;
    
    // Timing
    std::chrono::nanoseconds queued_time;
    std::chrono::nanoseconds start_time;
    std::chrono::nanoseconds end_time;
    std::chrono::nanoseconds duration;
    
    // Resource usage
    size_t memory_allocated;
    size_t memory_freed;
    
    // Status
    TaskState final_state;
    size_t retry_count;
};
```

### Accessing Task Profiles

```cpp
auto summary = profiler.generate_summary();

// Find slowest tasks
std::vector<TaskProfile> sorted = summary.task_profiles;
std::sort(sorted.begin(), sorted.end(),
    [](const auto& a, const auto& b) {
        return a.duration > b.duration;
    });

for (const auto& tp : sorted | std::views::take(5)) {
    std::cout << tp.name << ": " 
              << tp.duration.count() / 1e6 << " ms\n";
}
```

### Custom Events

```cpp
// Record custom events in task code
task->set_cpu_function([](TaskContext& ctx) {
    ctx.profiler().begin_event("DataLoading");
    load_data();
    ctx.profiler().end_event("DataLoading");
    
    ctx.profiler().begin_event("Processing");
    process();
    ctx.profiler().end_event("Processing");
});
```

---

## Optimization Guide

### Step 1: Identify Bottlenecks

```cpp
void analyze_bottlenecks(const ProfileSummary& summary) {
    // Find critical path
    auto critical_time = std::chrono::nanoseconds::zero();
    for (const auto& tp : summary.task_profiles) {
        if (tp.duration > critical_time) {
            critical_time = tp.duration;
            std::cout << "Critical path task: " << tp.name << "\n";
        }
    }
    
    // Check device balance
    double cpu_ratio = static_cast<double>(summary.cpu_time.count()) / 
                       (summary.cpu_time.count() + summary.gpu_time.count());
    std::cout << "CPU/GPU ratio: " << cpu_ratio * 100 << "% / "
              << (1 - cpu_ratio) * 100 << "%\n";
    
    // Check for idle time
    double ideal_time = summary.cpu_time.count() + summary.gpu_time.count();
    double idle_ratio = 1.0 - (ideal_time / summary.total_time.count());
    std::cout << "Idle time: " << idle_ratio * 100 << "%\n";
}
```

### Step 2: Common Optimizations

#### A. Increase Parallelism

```cpp
// Before: Sequential
for (int i = 0; i < 100; ++i) {
    auto t = graph.add_task();
    // ... (no parallelism)
}

// After: Parallel batches
int batch_size = 10;
for (int i = 0; i < 100; i += batch_size) {
    TaskGroup batch("batch_" + std::to_string(i), graph);
    for (int j = i; j < std::min(i + batch_size, 100); ++j) {
        auto t = batch.add_task(DeviceType::Any);
        // ...
    }
}
```

#### B. Optimize Data Transfer

```cpp
// Before: Transfer each batch
for (auto& batch : batches) {
    auto t = graph.add_task(DeviceType::GPU);
    t->set_gpu_function([&batch](auto& ctx, auto stream) {
        // H2D copy every iteration
        cudaMemcpyAsync(d_data, batch.data(), ...);
        kernel<<<...>>>(d_data);
    });
}

// After: Persistent GPU memory
auto t = graph.add_task(DeviceType::CPU);
t->set_cpu_function([](auto& ctx) {
    void* d_data = ctx.allocate_gpu_memory(total_size);
    cudaMemcpyAsync(d_data, all_data, ...);
    ctx.set_output("gpu_data", d_data, total_size);
});
```

#### C. Reduce Synchronization

```cpp
// Before: Synchronize after each kernel
for (auto& task : tasks) {
    task->set_gpu_function([](auto& ctx, auto stream) {
        kernel1<<<...>>>(...);
        cudaStreamSynchronize(stream);  // Bad!
        kernel2<<<...>>>(...);
    });
}

// After: Let scheduler handle dependencies
task1->set_gpu_function([](auto& ctx, auto stream) {
    kernel1<<<...>>>(...);  // No sync needed
});

task2->set_gpu_function([](auto& ctx, auto stream) {
    kernel2<<<...>>>(...);
});
graph.add_dependency(task1->id(), task2->id());
```

### Step 3: Validate Improvements

```cpp
void benchmark(const std::string& name, auto&& setup_fn) {
    Scheduler scheduler;
    setup_fn(scheduler);
    
    scheduler.set_profiling(true);
    
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.execute();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto summary = scheduler.profiler().generate_summary();
    
    std::cout << name << ": "
              << std::chrono::duration_cast<std::chrono::milliseconds>(
                     end - start).count()
              << " ms (parallelism: " << summary.parallelism << "x)\n";
}

// Compare implementations
benchmark("Baseline", baseline_setup);
benchmark("Optimized", optimized_setup);
```

---

## Advanced Profiling

### Continuous Profiling

```cpp
class ContinuousProfiler {
    std::vector<ProfileSummary> history_;
    
public:
    void record(const ProfileSummary& summary) {
        history_.push_back(summary);
        
        // Detect performance regression
        if (history_.size() >= 10) {
            auto recent = average_last_n(5);
            auto older = average_last_n(5, 5);
            
            if (recent.total_time > older.total_time * 1.2) {
                alert("Performance regression detected!");
            }
        }
    }
};
```

### Memory Profiling

```cpp
// Track memory patterns
struct MemoryProfile {
    void track_allocation(size_t size) {
        allocations.push_back({now(), size});
    }
    
    void generate_report() {
        size_t peak = 0;
        size_t current = 0;
        
        for (const auto& [time, size] : allocations) {
            current += size;
            peak = std::max(peak, current);
        }
        
        std::cout << "Peak memory: " << peak / (1024*1024) << " MB\n";
    }
};
```

### Distributed Profiling

```cpp
// Aggregate profiles from multiple nodes
class DistributedProfiler {
    std::vector<ProfileSummary> node_profiles_;
    
public:
    void add_node_profile(const ProfileSummary& profile) {
        node_profiles_.push_back(profile);
    }
    
    ProfileSummary aggregate() {
        ProfileSummary total;
        
        for (const auto& p : node_profiles_) {
            total.total_time = std::max(total.total_time, p.total_time);
            total.task_count += p.task_count;
            // ... aggregate other metrics
        }
        
        return total;
    }
};
```

---

## Further Reading

- [API Reference](api-reference.md) - Profiler API details
- [Architecture Overview](architecture.md) - System design
- [Examples](examples.md) - Profiling examples
