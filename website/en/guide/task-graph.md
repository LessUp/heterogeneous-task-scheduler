# Task Graph

The TaskGraph is the foundation of HTS, providing DAG (Directed Acyclic Graph) based task management.

## Overview

A `TaskGraph` represents a collection of tasks and their dependencies. HTS ensures tasks are executed in an order that respects all dependencies while maximizing parallelism.

## Creating Tasks

### Basic Task Creation

```cpp
#include <hts/task_graph.hpp>

using namespace hts;

TaskGraph graph;

// Create a CPU task
auto cpu_task = graph.add_task(DeviceType::CPU, "CPU_Work");

// Create a GPU task  
auto gpu_task = graph.add_task(DeviceType::GPU, "GPU_Work");
```

### Configuring Tasks

Tasks can be configured with various properties:

```cpp
// Set the function to execute
cpu_task->set_cpu_function([](TaskContext& ctx) {
    // CPU work here
    std::cout << "Executing on CPU" << std::endl;
});

gpu_task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // GPU work here
    std::cout << "Executing on GPU" << std::endl;
});

// Set priority (higher = more important)
cpu_task->set_priority(10);

// Set memory requirements
cpu_task->set_memory_requirement(1024 * 1024); // 1MB
```

### Using TaskBuilder (Recommended)

The `TaskBuilder` provides a fluent API for cleaner code:

```cpp
#include <hts/task_builder.hpp>

TaskBuilder builder(graph);

auto task = builder
    .create_task("ComplexTask")
    .device(DeviceType::GPU)
    .cpu_func([](TaskContext& ctx) {
        // Fallback if GPU unavailable
    })
    .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
        // Primary GPU execution
    })
    .priority(5)
    .retry_policy(RetryPolicy{
        .max_retries = 3,
        .backoff_ms = 100,
        .backoff_multiplier = 2.0f
    })
    .build();
```

## Managing Dependencies

### Adding Dependencies

```cpp
// taskB depends on taskA (taskA must complete before taskB starts)
graph.add_dependency(taskA->id(), taskB->id());

// Complex dependency chain
graph.add_dependency(A->id(), B->id());
graph.add_dependency(B->id(), C->id());
graph.add_dependency(C->id(), D->id());
// Execution order: A → B → C → D
```

### Removing Dependencies

```cpp
// Remove a specific dependency
graph.remove_dependency(taskA->id(), taskB->id());
```

### Checking Dependencies

```cpp
// Check if a dependency exists
bool has_dep = graph.has_dependency(taskB->id(), taskA->id());

// Get all predecessors of a task
auto predecessors = graph.get_predecessors(taskB->id());

// Get all successors of a task
auto successors = graph.get_successors(taskA->id());
```

## Task States

Tasks progress through several states during execution:

```
Created → Ready → Running → Completed
              ↓
          Failed → Retrying → Running
              ↓
          Failed (max retries exceeded)
```

```cpp
enum class TaskStatus {
    Created,     // Task created but not ready
    Ready,       // Dependencies satisfied, ready to run
    Running,     // Currently executing
    Completed,   // Successfully finished
    Failed       // Execution failed
};
```

## Graph Validation

HTS automatically validates the graph:

### Cycle Detection

```cpp
try {
    graph.add_dependency(A->id(), B->id());
    graph.add_dependency(B->id(), C->id());
    graph.add_dependency(C->id(), A->id()); // Creates cycle!
} catch (const std::runtime_error& e) {
    std::cerr << "Cycle detected: " << e.what() << std::endl;
}
```

### Orphan Detection

```cpp
// Check for unreachable tasks
auto orphans = graph.find_orphan_tasks();
if (!orphans.empty()) {
    std::cout << "Found " << orphans.size() << " orphan tasks" << std::endl;
}
```

## Graph Statistics

```cpp
std::cout << "Total tasks: " << graph.num_tasks() << std::endl;
std::cout << "Total dependencies: " << graph.num_dependencies() << std::endl;

// Critical path (longest dependency chain)
auto critical_path = graph.get_critical_path();
std::cout << "Critical path length: " << critical_path.size() << std::endl;

// Topological sort
auto order = graph.topological_sort();
std::cout << "Execution order:" << std::endl;
for (const auto& task_id : order) {
    std::cout << "  Task " << task_id << std::endl;
}
```

## Advanced Features

### Task Groups

Group related tasks for easier management:

```cpp
#include <hts/task_group.hpp>

TaskGroup preprocessing(graph);

auto task1 = preprocessing.add_task(DeviceType::CPU, "LoadData");
auto task2 = preprocessing.add_task(DeviceType::CPU, "ParseData");
auto task3 = preprocessing.add_task(DeviceType::CPU, "ValidateData");

graph.add_dependency(task1->id(), task2->id());
graph.add_dependency(task2->id(), task3->id());

// Execute all tasks in group
preprocessing.execute_all();
```

### Conditional Execution

Tasks can be conditionally enabled:

```cpp
task->set_condition([](const TaskContext& ctx) {
    // Only execute if certain conditions are met
    return ctx.get_retry_count() == 0;
});
```

### Task Tags

Add metadata to tasks for easier querying:

```cpp
task->add_tag("preprocessing");
task->add_tag("io-bound");

// Query by tag
auto io_tasks = graph.find_tasks_by_tag("io-bound");
```

## Best Practices

1. **Minimize Dependencies**: Only add necessary dependencies to maximize parallelism
2. **Use TaskBuilder**: Provides cleaner API and automatic validation
3. **Set Priorities**: Help the scheduler make better decisions
4. **Add Retry Policies**: Handle transient failures gracefully
5. **Validate Early**: Call `graph.validate()` before execution to catch issues
6. **Use Meaningful Names**: Helps with debugging and profiling

## Example: Complex DAG

```cpp
TaskGraph graph;
TaskBuilder builder(graph);

// Data loading (CPU)
auto load_data = builder.create_task("LoadData")
    .device(DeviceType::CPU)
    .cpu_func(load_data_func)
    .build();

// Preprocessing (CPU, parallel)
auto preprocess_1 = builder.create_task("Preprocess_1")
    .device(DeviceType::CPU)
    .cpu_func(preprocess_func)
    .build();

auto preprocess_2 = builder.create_task("Preprocess_2")
    .device(DeviceType::CPU)
    .cpu_func(preprocess_func)
    .build();

// GPU processing
auto gpu_process = builder.create_task("GPU_Process")
    .device(DeviceType::GPU)
    .gpu_func(gpu_kernel)
    .build();

// Postprocessing (CPU)
auto postprocess = builder.create_task("Postprocess")
    .device(DeviceType::CPU)
    .cpu_func(postprocess_func)
    .build();

// Set dependencies
graph.add_dependency(load_data->id(), preprocess_1->id());
graph.add_dependency(load_data->id(), preprocess_2->id());
graph.add_dependency(preprocess_1->id(), gpu_process->id());
graph.add_dependency(preprocess_2->id(), gpu_process->id());
graph.add_dependency(gpu_process->id(), postprocess->id());

/*
    Graph structure:
    
    LoadData
      ├──► Preprocess_1 ──┐
      └──► Preprocess_2 ──┼► GPU_Process ──► Postprocess
                            ┘
*/
```

## Next Steps

- [Scheduling](/en/guide/scheduling) — How tasks are scheduled
- [Memory](/en/guide/memory) — GPU memory management
- [Error Handling](/en/guide/error-handling) — Handling failures
- [API Reference](/en/api/task-graph) — Complete TaskGraph API
