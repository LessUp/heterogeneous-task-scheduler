# API Reference

Complete API documentation for the Heterogeneous Task Scheduler library.

## Core Components

HTS provides a modular API for DAG-based task scheduling across CPU and GPU devices.

### Main Classes

| Class | Description | Documentation |
|-------|-------------|---------------|
| **Scheduler** | Main entry point for executing task graphs | [Guide](/guide/scheduling) · [API](/api/scheduler) |
| **TaskGraph** | DAG management and dependency tracking | [Guide](/guide/task-graph) · [API](/api/task-graph) |
| **TaskBuilder** | Fluent API for creating and configuring tasks | [API](/api/task-builder) |
| **Task** | Represents a unit of work | [Guide](/guide/task-graph) |
| **TaskContext** | Execution context provided to tasks | [Guide](/guide/task-graph) |

## Supporting Classes

### Scheduling Policies

| Class | Strategy | Use Case |
|-------|----------|----------|
| **GPUPriorityPolicy** | Prefer GPU tasks | GPU-heavy workloads |
| **CPUPriorityPolicy** | Prefer CPU tasks | CPU preprocessing pipelines |
| **RoundRobinPolicy** | Alternate CPU/GPU | Balanced workloads |
| **LoadBasedPolicy** | Select by device load | Dynamic workloads |

[Learn more about scheduling →](/guide/scheduling)

### Memory Management

| Class | Description |
|-------|-------------|
| **MemoryPool** | Buddy system allocator for GPU memory |
| **MemoryPoolConfig** | Pool configuration (size, block sizes, defrag) |

[Learn more about memory pools →](/guide/memory)

### Error Handling

| Class | Description |
|-------|-------------|
| **Error** | Error code and message wrapper |
| **RetryPolicy** | Automatic retry configuration |
| **ErrorCode** | Enumeration of all error codes |

[Learn more about error handling →](/guide/error-handling)

## Quick Start API

### 1. Create Tasks

```cpp
#include <hts/task_builder.hpp>

using namespace hts;

TaskGraph graph;
TaskBuilder builder(graph);

auto cpu_task = builder
    .create_task("CPU_Work")
    .device(DeviceType::CPU)
    .cpu_func([](TaskContext& ctx) {
        // CPU work here
    })
    .priority(10)
    .build();

auto gpu_task = builder
    .create_task("GPU_Work")
    .device(DeviceType::GPU)
    .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
        // GPU work here
    })
    .memory(256 * 1024 * 1024)  // 256 MB
    .build();
```

### 2. Set Dependencies

```cpp
// gpu_task depends on cpu_task
graph.add_dependency(cpu_task->id(), gpu_task->id());
```

### 3. Execute

```cpp
Scheduler scheduler;
scheduler.init(&graph);
scheduler.execute();
scheduler.wait_for_completion();
```

## API Categories

### Task Management

- [TaskGraph API](/api/task-graph) — Complete TaskGraph reference
- [TaskBuilder API](/api/task-builder) — Fluent task creation
- [Task](/guide/task-graph) — Task properties and lifecycle

### Execution

- [Scheduler API](/api/scheduler) — Scheduler configuration and execution
- [Execution Engine](/guide/architecture) — CPU thread pool and GPU streams

### Optimization

- [Scheduling Policies](/guide/scheduling) — Policy selection and customization
- [Memory Pool](/guide/memory) — GPU memory management
- [Profiler](/guide/architecture#profiler) — Performance monitoring

### Reliability

- [Error Handling](/guide/error-handling) — Error codes and recovery
- [Retry Policies](/guide/error-handling#retry-policies) — Automatic retry
- [Task Fallbacks](/guide/error-handling#fallback-tasks) — Graceful degradation

## Namespaces

All HTS classes are in the `hts` namespace:

```cpp
using namespace hts;

// Or prefix explicitly
hts::Scheduler scheduler;
hts::TaskGraph graph;
hts::TaskBuilder builder(graph);
```

## Error Handling

Most HTS functions return an `Error` object:

```cpp
Error err = scheduler.init(&graph);
if (!err.ok()) {
    std::cerr << "Failed: " << err.message() << std::endl;
    return 1;
}
```

Exceptions are thrown for critical errors (e.g., cycle detection):

```cpp
try {
    graph.add_dependency(A->id(), B->id());
    graph.add_dependency(B->id(), A->id());  // Creates cycle
} catch (const CycleDetectedError& e) {
    std::cerr << "Cycle: " << e.what() << std::endl;
}
```

## Threading Model

HTS is thread-safe for concurrent access:

- **TaskGraph**: Thread-safe reads during execution
- **Scheduler**: Thread-safe execution
- **TaskContext**: Thread-safe within task execution
- **MemoryPool**: Thread-safe concurrent allocations

## Performance Considerations

### Minimize Scheduling Overhead

```cpp
// Good: Batch small tasks
for (int i = 0; i < 100; ++i) {
    tasks.push_back(builder.create_task(...).build());
}

// Avoid: Excessive synchronization
// HTS minimizes locks internally
```

### Configure for Your Workload

```cpp
SchedulerConfig config;
config.cpu_thread_count = 8;      // Match your CPU cores
config.gpu_stream_count = 4;      // Adjust for your GPU
config.enable_profiling = true;    // Enable for production monitoring
scheduler.configure(config);
```

### Monitor Performance

```cpp
const auto& stats = scheduler.get_stats();
std::cout << "Parallelism: " << stats.parallelism_factor << "x" << std::endl;
std::cout << "Total time: " << stats.total_time_ms << " ms" << std::endl;
```

## Version Information

Current version: **v1.2.0**

Release history is tracked on [GitHub Releases](https://github.com/LessUp/heterogeneous-task-scheduler/releases).

## Next Steps

- [Quick Start Guide](/guide/quickstart) — Build your first DAG
- [Scheduler API](/api/scheduler) — Detailed scheduler reference
- [TaskGraph API](/api/task-graph) — TaskGraph reference
- [TaskBuilder API](/api/task-builder) — Fluent API reference
- [Examples](/examples/) — Working code examples
