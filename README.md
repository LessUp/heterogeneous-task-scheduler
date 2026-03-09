# Heterogeneous Task Scheduler (HTS)

English | [简体中文](README.zh-CN.md)

A C++ framework for scheduling and executing task DAGs (Directed Acyclic Graphs) across CPU and GPU.

## Features

- **DAG Task Dependency Management** — Automatic cycle detection, correct execution order
- **GPU Memory Pool** — Buddy system allocator, avoiding frequent cudaMalloc/cudaFree
- **Async Concurrent Execution** — CPU thread pool + CUDA streams, maximizing hardware utilization
- **Load Balancing** — Automatic task assignment based on device load
- **Performance Monitoring** — Execution time stats and timeline visualization

## Requirements

- CMake >= 3.18, CUDA Toolkit, C++17 compiler
- Google Test & RapidCheck (auto-downloaded)

## Quick Start

```bash
mkdir build && cd build
cmake .. && make -j$(nproc)
ctest --output-on-failure
```

## Usage

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

hts::SchedulerConfig config;
config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
config.cpu_thread_count = 4;
config.gpu_stream_count = 4;

hts::Scheduler scheduler(config);

auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);

task1->set_cpu_function([](hts::TaskContext& ctx) { /* ... */ });
task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) { /* ... */ });

scheduler.graph().add_dependency(task1->id(), task2->id());
scheduler.execute();
```

### Fluent Builder API

```cpp
auto scheduler = hts::ScheduleBuilder()
    .with_memory_pool(256_MB)
    .with_cpu_threads(4)
    .with_gpu_streams(4)
    .add_cpu_task("preprocess", preprocess_fn)
    .add_gpu_task("compute", compute_fn)
    .add_dependency("preprocess", "compute")
    .build();
scheduler.execute();
```

## License

MIT License
