# Quick Start

This tutorial will walk you through creating and executing your first heterogeneous task graph with HTS.

## Step 1: Include Headers

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <hts/task_graph.hpp>
#include <hts/task_builder.hpp>
#include <iostream>
```

## Step 2: Create a Simple DAG

Let's create a simple pipeline where:
1. A CPU task preprocesses data
2. A GPU task processes the data on the GPU
3. Another CPU task post-processes the results

```cpp
using namespace hts;

int main() {
    // Create a task graph
    TaskGraph graph;
    
    // Add CPU preprocessing task
    auto cpu_preprocess = graph.add_task(DeviceType::CPU, "CPU_Preprocess");
    cpu_preprocess->set_cpu_function([](TaskContext& ctx) {
        std::cout << "Preprocessing data on CPU..." << std::endl;
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    });
    
    // Add GPU processing task
    auto gpu_process = graph.add_task(DeviceType::GPU, "GPU_Process");
    gpu_process->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
        std::cout << "Processing data on GPU..." << std::endl;
        // Simulate GPU work
        cudaStreamSynchronize(stream);
    });
    
    // Add CPU postprocessing task
    auto cpu_postprocess = graph.add_task(DeviceType::CPU, "CPU_Postprocess");
    cpu_postprocess->set_cpu_function([](TaskContext& ctx) {
        std::cout << "Post-processing results on CPU..." << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    });
    
    // Set up dependencies: CPU -> GPU -> CPU
    graph.add_dependency(cpu_preprocess->id(), gpu_process->id());
    graph.add_dependency(gpu_process->id(), cpu_postprocess->id());
    
    std::cout << "Task graph created with " << graph.num_tasks() << " tasks" << std::endl;
    std::cout << "Dependencies: " << graph.num_dependencies() << std::endl;
    
    return 0;
}
```

## Step 3: Execute with Scheduler

Now let's execute the graph using the scheduler:

```cpp
int main() {
    // ... (create task graph as above) ...
    
    // Create scheduler
    Scheduler scheduler;
    
    // Initialize with the graph
    scheduler.init(&graph);
    
    // Execute the graph
    std::cout << "Executing task graph..." << std::endl;
    scheduler.execute();
    
    // Wait for completion
    scheduler.wait_for_completion();
    
    std::cout << "Execution completed successfully!" << std::endl;
    
    return 0;
}
```

## Step 4: Add TaskBuilder for Convenience

Use TaskBuilder for cleaner code:

```cpp
#include <hts/task_builder.hpp>

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Build tasks with fluent API
    auto cpu_task = builder
        .create_task("CPU_Work")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "CPU work executed!" << std::endl;
        })
        .priority(10)
        .build();
    
    auto gpu_task = builder
        .create_task("GPU_Work")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            std::cout << "GPU work executed!" << std::endl;
        })
        .priority(5)
        .build();
    
    graph.add_dependency(cpu_task->id(), gpu_task->id());
    
    // Execute
    Scheduler scheduler;
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

## Step 5: Configure Scheduling Policy

HTS supports multiple scheduling policies:

```cpp
#include <hts/scheduling_policy.hpp>

int main() {
    Scheduler scheduler;
    
    // Configure policy (before init)
    scheduler.set_policy(std::make_unique<GPUPriorityPolicy>());
    // Or: CPUPriorityPolicy, RoundRobinPolicy, LoadBasedPolicy
    
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

## Complete Example

Here's the complete, minimal example:

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    using namespace hts;
    
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    auto t1 = builder.create_task("Task1")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext&) { 
            std::cout << "Task 1 (CPU)" << std::endl; 
        })
        .build();
    
    auto t2 = builder.create_task("Task2")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext&, cudaStream_t) { 
            std::cout << "Task 2 (GPU)" << std::endl; 
        })
        .build();
    
    graph.add_dependency(t1->id(), t2->id());
    
    Scheduler scheduler;
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

## Compile and Run

```bash
g++ -std=c++17 -O2 main.cpp -o main -I../include -Lbuild -lhts_lib -lpthread
./main
```

Output:
```
Task 1 (CPU)
Task 2 (GPU)
Execution completed successfully!
```

## Next Steps

- [Architecture](/en/guide/architecture) — Learn about HTS internals
- [Task Graph](/en/guide/task-graph) — Deep dive into DAG management
- [Scheduling](/en/guide/scheduling) — Understand scheduling policies
- [Examples](/en/examples/) — Explore more complex examples
