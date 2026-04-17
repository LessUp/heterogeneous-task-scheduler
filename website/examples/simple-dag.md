# Simple DAG Example

This example demonstrates creating and executing a simple Directed Acyclic Graph (DAG) with HTS.

## Overview

We'll create a data processing pipeline:

```
Load Data (CPU) → Preprocess (CPU) → GPU Compute → Postprocess (CPU) → Save Results (CPU)
```

## Complete Code

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>
#include <vector>

using namespace hts;

// Simulated data loading function
void load_data(TaskContext& ctx) {
    std::cout << "[CPU] Loading data from disk..." << std::endl;
    // Simulate I/O operation
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

// Simulated preprocessing function
void preprocess_data(TaskContext& ctx) {
    std::cout << "[CPU] Preprocessing data (normalization, filtering)..." << std::endl;
    // Simulate CPU computation
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
}

// Simulated GPU computation
void gpu_compute(TaskContext& ctx, cudaStream_t stream) {
    std::cout << "[GPU] Running neural network inference..." << std::endl;
    
    // Simulate GPU kernel execution
    // In real code: my_kernel<<<blocks, threads, 0, stream>>>(data);
    
    cudaStreamSynchronize(stream);
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
}

// Simulated postprocessing
void postprocess_data(TaskContext& ctx) {
    std::cout << "[CPU] Post-processing results..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

// Simulated save function
void save_results(TaskContext& ctx) {
    std::cout << "[CPU] Saving results to disk..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
}

int main() {
    std::cout << "=== Simple DAG Example ===" << std::endl;
    
    // Step 1: Create task graph
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Step 2: Create tasks
    auto load_task = builder
        .create_task("LoadData")
        .device(DeviceType::CPU)
        .cpu_func(load_data)
        .priority(10)
        .tag("io")
        .build();
    
    auto preprocess_task = builder
        .create_task("Preprocess")
        .device(DeviceType::CPU)
        .cpu_func(preprocess_data)
        .priority(8)
        .tag("cpu-bound")
        .build();
    
    auto gpu_task = builder
        .create_task("GPUCompute")
        .device(DeviceType::GPU)
        .gpu_func(gpu_compute)
        .priority(15)
        .memory(256 * 1024 * 1024)  // 256 MB
        .build();
    
    auto postprocess_task = builder
        .create_task("Postprocess")
        .device(DeviceType::CPU)
        .cpu_func(postprocess_data)
        .priority(8)
        .build();
    
    auto save_task = builder
        .create_task("SaveResults")
        .device(DeviceType::CPU)
        .cpu_func(save_results)
        .priority(10)
        .tag("io")
        .build();
    
    // Step 3: Set up dependencies (create the DAG)
    graph.add_dependency(load_task->id(), preprocess_task->id());
    graph.add_dependency(preprocess_task->id(), gpu_task->id());
    graph.add_dependency(gpu_task->id(), postprocess_task->id());
    graph.add_dependency(postprocess_task->id(), save_task->id());
    
    // Step 4: Validate the graph
    Error err = graph.validate();
    if (!err.ok()) {
        std::cerr << "Graph validation failed: " << err.message() << std::endl;
        return 1;
    }
    
    std::cout << "\nGraph Statistics:" << std::endl;
    std::cout << "  Tasks: " << graph.num_tasks() << std::endl;
    std::cout << "  Dependencies: " << graph.num_dependencies() << std::endl;
    
    // Step 5: Create and configure scheduler
    Scheduler scheduler;
    
    SchedulerConfig config;
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 2;
    config.enable_profiling = true;
    scheduler.configure(config);
    
    // Use GPU priority policy
    scheduler.set_policy(std::make_unique<GPUPriorityPolicy>());
    
    // Step 6: Initialize and execute
    err = scheduler.init(&graph);
    if (!err.ok()) {
        std::cerr << "Scheduler initialization failed: " << err.message() << std::endl;
        return 1;
    }
    
    std::cout << "\nExecuting DAG..." << std::endl;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    scheduler.execute();
    scheduler.wait_for_completion();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time
    );
    
    // Step 7: Print results
    std::cout << "\n=== Execution Complete ===" << std::endl;
    std::cout << "Total time: " << duration.count() << " ms" << std::endl;
    
    const auto& stats = scheduler.get_stats();
    std::cout << "Tasks scheduled: " << stats.tasks_scheduled << std::endl;
    std::cout << "CPU tasks: " << stats.cpu_tasks << std::endl;
    std::cout << "GPU tasks: " << stats.gpu_tasks << std::endl;
    std::cout << "Parallelism factor: " << stats.parallelism_factor << "x" << std::endl;
    
    return 0;
}
```

## Expected Output

```
=== Simple DAG Example ===

Graph Statistics:
  Tasks: 5
  Dependencies: 4

Executing DAG...
[CPU] Loading data from disk...
[CPU] Preprocessing data (normalization, filtering)...
[GPU] Running neural network inference...
[CPU] Post-processing results...
[CPU] Saving results to disk...

=== Execution Complete ===
Total time: 580 ms
Tasks scheduled: 5
CPU tasks: 4
GPU tasks: 1
Parallelism factor: 1.0x
```

## Explanation

### 1. Task Creation

We use `TaskBuilder` for clean, fluent syntax:

```cpp
auto task = builder
    .create_task("TaskName")
    .device(DeviceType::CPU)  // or DeviceType::GPU
    .cpu_func(cpu_function)   // for CPU tasks
    .gpu_func(gpu_function)   // for GPUtasks
    .priority(10)             // relative priority
    .build();                 // finalize creation
```

### 2. Dependencies

Dependencies form the DAG structure:

```cpp
// Task B depends on Task A (A must complete before B starts)
graph.add_dependency(A->id(), B->id());
```

### 3. Validation

Always validate before execution:

```cpp
Error err = graph.validate();
if (!err.ok()) {
    std::cerr << "Graph validation failed: " << err.message() << std::endl;
    return 1;
}
```

### 4. Execution

Three-step execution:

```cpp
scheduler.init(&graph);          // Prepare
scheduler.execute();             // Start (non-blocking)
scheduler.wait_for_completion(); // Wait for done
```

## Building and Running

### CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.18)
project(simple_dag_example)

set(CMAKE_CXX_STANDARD 17)

find_package(hts REQUIRED)

add_executable(simple_dag main.cpp)
target_link_libraries(simple_dag PRIVATE hts_lib)
```

### Build Commands

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
./simple_dag
```

## Variations

### Parallel Branches

Create parallel execution branches:

```cpp
auto load = builder.create_task("Load").cpu_func(load_data).build();

// Two parallel preprocessing tasks
auto preprocess1 = builder.create_task("Preprocess1").cpu_func(preprocess_chunk1).build();
auto preprocess2 = builder.create_task("Preprocess2").cpu_func(preprocess_chunk2).build();

auto gpu = builder.create_task("GPU").gpu_func(gpu_compute).build();

// Both preprocessing tasks must complete before GPU
graph.add_dependency(load->id(), preprocess1->id());
graph.add_dependency(load->id(), preprocess2->id());
graph.add_dependency(preprocess1->id(), gpu->id());
graph.add_dependency(preprocess2->id(), gpu->id());

/*
    Graph:
    Load ──► Preprocess1 ──┐
           └► Preprocess2 ──┼► GPU
                             ┘
*/
```

### With Retry Policy

Add fault tolerance:

```cpp
auto gpu_task = builder
    .create_task("GPUCompute")
    .device(DeviceType::GPU)
    .gpu_func(risky_kernel)
    .retry_policy(RetryPolicy{
        .max_retries = 3,
        .backoff_ms = 100,
        .backoff_multiplier = 2.0f
    })
    .build();
```

## Next Steps

- [Pipeline Example](/examples/pipeline) — More complex pipeline with error handling
- [Quick Start Guide](/guide/quickstart) — Tutorial
- [Task Graph Guide](/guide/task-graph) — Deep dive into DAGs
