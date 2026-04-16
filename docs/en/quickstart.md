# Quick Start Guide

> Get started with HTS in 5 minutes

---

## Table of Contents

- [Prerequisites](#prerequisites)
- [Basic Concepts](#basic-concepts)
- [Your First Program](#your-first-program)
- [Fluent API](#fluent-api)
- [CPU + GPU Workflow](#cpu--gpu-workflow)
- [Error Handling](#error-handling)
- [Next Steps](#next-steps)

---

## Prerequisites

Before starting, ensure you have:
- [ ] Completed [installation](installation.md)
- [ ] Working C++17 compiler
- [ ] CMake 3.18+
- [ ] CUDA Toolkit 11.0+ (optional, for GPU features)

---

## Basic Concepts

### Core Components

```
┌─────────────────────────────────────────────────────────────┐
│                         Scheduler                            │
│  ┌─────────────────┐  ┌─────────────────────────────────┐   │
│  │    TaskGraph    │→ │      Execution Engine           │   │
│  │    (DAG)        │  │  (CPU Thread Pool + GPU Streams)│   │
│  └─────────────────┘  └─────────────────────────────────┘   │
│                         ┌─────────────────────────────┐      │
│                         │      Memory Pool            │      │
│                         │  (Buddy System Allocator)   │      │
│                         └─────────────────────────────┘      │
└─────────────────────────────────────────────────────────────┘
```

| Component | Purpose |
|-----------|---------|
| `Scheduler` | Main orchestrator for task execution |
| `TaskGraph` | DAG representing task dependencies |
| `Task` | Individual unit of work |
| `TaskContext` | Execution context (memory, I/O, status) |

---

## Your First Program

Create a simple task dependency graph:

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 1. Create scheduler with default configuration
    hts::Scheduler scheduler;

    // 2. Add tasks
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);

    // 3. Set task functions
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 1 executed on CPU\n";
        ctx.set_output(" result", 42);
    });

    task2->set_cpu_function([](hts::TaskContext& ctx) {
        auto result = ctx.get_input<int>(" result");
        std::cout << "Task 2 received: " << result << "\n";
    });

    // 4. Define dependencies (task2 depends on task1)
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // 5. Execute the graph
    scheduler.execute();

    return 0;
}
```

**Output:**
```
Task 1 executed on CPU
Task 2 received: 42
```

### Build and Run

```bash
# Save as first_program.cpp
g++ -std=c++17 -I./include first_program.cpp -o first_program -lpthread
./first_program
```

---

## Fluent API

The TaskBuilder provides a more intuitive way to create tasks:

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;
    hts::TaskBuilder builder(scheduler.graph());

    // Chain tasks together
    auto init = builder
        .name("Initialize")
        .device(hts::DeviceType::CPU)
        .priority(hts::TaskPriority::High)
        .cpu([](hts::TaskContext& ctx) {
            std::cout << "Initializing...\n";
            ctx.set_output("config", std::string("ready"));
        })
        .build();

    auto process = builder
        .name("Process")
        .after(init)                    // Depends on init
        .device(hts::DeviceType::CPU)
        .cpu([](hts::TaskContext& ctx) {
            auto config = ctx.get_input<std::string>("config");
            std::cout << "Processing with: " << config << "\n";
        })
        .build();

    auto finalize = builder
        .name("Finalize")
        .after(process)                 // Depends on process
        .device(hts::DeviceType::Any)   // Let scheduler decide
        .cpu([](hts::TaskContext& ctx) {
            std::cout << "Finalizing...\n";
        })
        .build();

    scheduler.execute();
    return 0;
}
```

**Benefits of Fluent API:**
- Clear dependency chains via `.after()`
- Self-documenting task names
- Easy to add/remove tasks

---

## CPU + GPU Workflow

A common pattern: CPU preprocessing → GPU computation → CPU postprocessing:

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

// Simple CUDA kernel example
__global__ void multiply_kernel(float* data, float factor, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        data[idx] *= factor;
    }
}

int main() {
    hts::Scheduler scheduler;
    const int data_size = 1024;
    const float factor = 2.5f;

    // CPU: Allocate and initialize data
    auto preprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
    preprocess->set_cpu_function([data_size](hts::TaskContext& ctx) {
        // Allocate GPU memory through context
        void* d_data = ctx.allocate_gpu_memory(data_size * sizeof(float));
        
        // Initialize data (in real app, would copy from host)
        std::vector<float> host_data(data_size, 1.0f);
        cudaMemcpy(d_data, host_data.data(), data_size * sizeof(float), 
                   cudaMemcpyHostToDevice);
        
        ctx.set_output("data", d_data, data_size * sizeof(float));
        std::cout << "Data prepared on GPU\n";
    });

    // GPU: Compute
    auto compute = scheduler.graph().add_task(hts::DeviceType::GPU);
    compute->set_gpu_function([data_size, factor](hts::TaskContext& ctx, 
                                                   cudaStream_t stream) {
        auto d_data = ctx.get_input<void*>("data");
        
        int block_size = 256;
        int grid_size = (data_size + block_size - 1) / block_size;
        
        multiply_kernel<<<grid_size, block_size, 0, stream>>>(
            static_cast<float*>(d_data), factor, data_size);
        
        ctx.set_output("result", d_data, data_size * sizeof(float));
        std::cout << "GPU computation complete\n";
    });

    // CPU: Post-process
    auto postprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
    postprocess->set_cpu_function([](hts::TaskContext& ctx) {
        auto d_data = ctx.get_input<void*>("result");
        
        // Copy back to host
        std::vector<float> host_data(1024);
        cudaMemcpy(host_data.data(), d_data, 1024 * sizeof(float),
                   cudaMemcpyDeviceToHost);
        
        float sum = 0;
        for (auto v : host_data) sum += v;
        std::cout << "Sum after processing: " << sum << "\n";
    });

    // Chain dependencies
    scheduler.graph().add_dependency(preprocess->id(), compute->id());
    scheduler.graph().add_dependency(compute->id(), postprocess->id());

    scheduler.execute();
    return 0;
}
```

---

## Error Handling

Handle task failures gracefully:

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    hts::Scheduler scheduler;

    // Set global error callback
    scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
        std::cerr << "[ERROR] Task " << id << " failed: " << msg << "\n";
    });

    // Task that might fail
    auto risky_task = scheduler.graph().add_task(hts::DeviceType::CPU);
    risky_task->set_cpu_function([](hts::TaskContext& ctx) {
        // Simulate error condition
        if (random() % 2 == 0) {
            throw std::runtime_error("Random failure occurred");
        }
        std::cout << "Task succeeded!\n";
    });

    // Add retry policy
    risky_task->set_retry_policy(
        hts::RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100})
    );

    scheduler.execute();
    return 0;
}
```

**Key Points:**
- Errors propagate to dependent tasks
- Retry policies can be configured per-task
- Error callbacks provide visibility

---

## Next Steps

| Topic | Learn About |
|-------|-------------|
| [Architecture](architecture.md) | Deep dive into system design |
| [API Reference](api-reference.md) | Complete API documentation |
| [Scheduling Policies](scheduling-policies.md) | Control task placement |
| [Memory Management](memory-management.md) | Optimize GPU memory usage |
| [Examples](examples.md) | More complex use cases |

---

## Quick Reference Card

```cpp
// Create scheduler
hts::Scheduler scheduler;
scheduler.set_profiling(true);

// Add task (three ways)
auto t1 = scheduler.graph().add_task(hts::DeviceType::CPU);
hts::TaskBuilder builder(scheduler.graph());
auto t2 = builder.name("Task2").cpu(...).build();

// Task function types
t1->set_cpu_function([](hts::TaskContext& ctx) { /* CPU work */ });
t1->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t s) { /* GPU work */ });

// Dependencies
scheduler.graph().add_dependency(t1->id(), t2->id());
builder.after(t1);  // Fluent API

// Execute
scheduler.execute();
```
