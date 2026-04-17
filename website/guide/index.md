# Introduction

Welcome to **HTS (Heterogeneous Task Scheduler)** — a high-performance C++ framework for scheduling and executing task DAGs across CPU and GPU devices.

## What is HTS?

HTS enables efficient parallel execution of complex task dependencies across heterogeneous computing environments. It provides:

- **DAG-based task management** with automatic dependency resolution
- **Intelligent scheduling** that optimizes CPU/GPU utilization
- **High-performance memory pooling** for GPU allocations (50-100x faster than cudaMalloc)
- **Comprehensive observability** with built-in profiling and metrics
- **Production-ready error handling** with retry policies and graceful degradation

## Core Features

### ⚡ Blazing Performance

Zero-overhead abstractions with lock-free data structures and optimized memory pools. Task scheduling overhead is just ~100 nanoseconds.

### 🔄 DAG Execution

Automatic cycle detection, topological sorting, and dependency tracking for complex task graphs with thousands of tasks.

### 🎯 Smart Scheduling

Pluggable scheduling policies:
- **GPU-First**: Prioritize GPU tasks for compute-heavy workloads
- **CPU-First**: Keep CPU busy for preprocessing pipelines
- **Round-Robin**: Balanced CPU/GPU utilization
- **Load-Based**: Dynamic selection based on current utilization

### 💾 Memory Pool

Buddy system allocator eliminates `cudaMalloc`/`cudaFree` overhead:
- O(log n) allocation time
- Automatic defragmentation
- Up to 50x faster than standard CUDA allocation

### 📊 Performance Insights

Built-in profiler with:
- Execution timeline export to Chrome tracing
- Device utilization metrics
- Parallelism analysis
- Memory usage statistics

### 🛡️ Production Ready

- Comprehensive error codes and exception safety
- Configurable retry policies with exponential backoff
- Task fallback functions for graceful degradation
- Detailed logging and monitoring support

## Core Architecture

```
User Application
       ↓
  TaskGraph Builder API
       ↓
    Scheduler
       ↓
Execution Engine (CPU Threads + GPU Streams)
       ↓
   Memory Pool (GPU)
```

### Key Components

| Component | Description | Documentation |
|-----------|-------------|---------------|
| **TaskGraph** | DAG management and dependency tracking | [Guide](/guide/task-graph) · [API](/api/task-graph) |
| **Scheduler** | Core execution orchestration | [Guide](/guide/scheduling) · [API](/api/scheduler) |
| **TaskBuilder** | Fluent API for task creation | [API](/api/task-builder) |
| **MemoryPool** | GPU memory management | [Guide](/guide/memory) |
| **Profiler** | Performance monitoring | Built-in to Scheduler |

## Quick Example

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

using namespace hts;

int main() {
    // Create task graph
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Create CPU task
    auto cpu_task = builder
        .create_task("PreprocessData")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "Preprocessing on CPU..." << std::endl;
        })
        .build();
    
    // Create GPU task
    auto gpu_task = builder
        .create_task("GPUCompute")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            std::cout << "Computing on GPU..." << std::endl;
            my_kernel<<<256, 128, 0, stream>>>(data);
            cudaStreamSynchronize(stream);
        })
        .build();
    
    // Set dependency: CPU task must complete before GPU task
    graph.add_dependency(cpu_task->id(), gpu_task->id());
    
    // Execute
    Scheduler scheduler;
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

## Getting Started

### 1. Installation

Choose your installation method:

**Build from source:**
```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

**Or use in your CMake project:**
```cmake
include(FetchContent)
FetchContent_Declare(
    hts
    GIT_REPOSITORY https://github.com/LessUp/heterogeneous-task-scheduler.git
    GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(hts)
```

→ [Full Installation Guide](/guide/installation)

### 2. Learn the Basics

Follow our step-by-step guides:

- [**Quick Start**](/guide/quickstart) — Build your first DAG (5 minutes)
- [**Architecture**](/guide/architecture) — Understand HTS internals
- [**Task Graph**](/guide/task-graph) — Master DAG creation
- [**Scheduling**](/guide/scheduling) — Optimize task execution

### 3. Explore Examples

Browse working examples:

- [**Simple DAG**](/examples/simple-dag) — Basic pipeline (beginner)
- [**Pipeline**](/examples/pipeline) — Complex DAG with error handling (intermediate)

## Performance Benchmarks

| Operation | Standard | HTS | Speedup |
|:----------|:---------|:----|:--------|
| GPU Memory Allocate (1MB) | ~50 μs | ~1 μs | **50x** |
| GPU Memory Free (1MB) | ~25 μs | ~0.5 μs | **50x** |
| Task Schedule | ~500 ns | ~100 ns | **5x** |

*Measured on NVIDIA V100 with CUDA 12.1*

## System Requirements

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake** 3.18 or higher
- **CUDA Toolkit** 11.0+ (optional, for GPU support)
- **Linux** (recommended), **Windows**, or **macOS**

## Community & Support

- 💬 [GitHub Discussions](https://github.com/LessUp/heterogeneous-task-scheduler/discussions)
- 🐛 [Report Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- 📖 [Contributing Guide](/contributing)
- 📝 [Changelog](/changelog)

## License

HTS is released under the [MIT License](https://github.com/LessUp/heterogeneous-task-scheduler/blob/main/LICENSE).

---

**Ready to start?** → [Quick Start Guide](/guide/quickstart)
