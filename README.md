# Heterogeneous Task Scheduler (HTS)

[![Docs](https://img.shields.io/badge/Docs-GitHub%20Pages-blue?logo=github)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)

English | [简体中文](README.zh-CN.md)

A high-performance C++ framework for scheduling and executing task DAGs (Directed Acyclic Graphs) across CPU and GPU devices.

## ✨ Features

| Feature | Description |
|---------|-------------|
| **DAG Management** | Automatic cycle detection, topological sorting, dependency tracking |
| **GPU Memory Pool** | Buddy system allocator, avoids frequent cudaMalloc/cudaFree |
| **Async Execution** | CPU thread pool + CUDA streams for maximum hardware utilization |
| **Load Balancing** | Automatic task assignment based on device load |
| **Performance Monitoring** | Execution stats, timeline visualization, profiling reports |
| **Flexible Scheduling** | Multiple scheduling policies (GPU-first, CPU-first, round-robin) |
| **Error Handling** | Error callbacks, failure propagation, retry policies |
| **Thread-Safe** | Safe concurrent task submission and execution |

## 📋 Requirements

| Requirement | Version |
|-------------|---------|
| CMake | >= 3.18 |
| CUDA Toolkit | >= 11.0 |
| C++ Compiler | C++17 compatible (GCC 8+, Clang 7+, MSVC 2019+) |
| Google Test | Auto-downloaded |
| RapidCheck | Auto-downloaded |

## 🚀 Quick Start

### Build

```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Run Tests

```bash
cd build
ctest --output-on-failure
```

### Basic Usage

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // Create scheduler with configuration
    hts::SchedulerConfig config;
    config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;

    hts::Scheduler scheduler(config);

    // Create tasks
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);
    auto task3 = scheduler.graph().add_task(hts::DeviceType::Any);

    // Set task functions
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 1 on CPU\n";
    });

    task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // Launch CUDA kernel here
    });

    // Task with both CPU and GPU functions - scheduler decides
    task3->set_cpu_function([](hts::TaskContext& ctx) { /* CPU version */ });
    task3->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t s) { /* GPU version */ });

    // Add dependencies: task2 and task3 depend on task1
    scheduler.graph().add_dependency(task1->id(), task2->id());
    scheduler.graph().add_dependency(task1->id(), task3->id());

    // Execute with error handling
    scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
        std::cerr << "Task " << id << " failed: " << msg << "\n";
    });

    scheduler.execute();

    return 0;
}
```

## 📖 Documentation

- **API Reference**: See header files in [`include/hts/`](include/hts/)
- **Examples**: See [`examples/`](examples/) directory
- **Changelog**: See [CHANGELOG.md](CHANGELOG.md)

## 🎯 Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                          │
├─────────────────────────────────────────────────────────────────┤
│                      TaskGraph Builder API                       │
│    TaskBuilder │ TaskGroup │ TaskBarrier │ TaskFuture            │
├─────────────────────────────────────────────────────────────────┤
│                          Scheduler                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ Dependency  │  │  Scheduling │  │  Profiler   │              │
│  │  Manager    │  │   Policy    │  │  & Logger   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                      Execution Engine                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ CPU Thread  │  │ GPU Stream  │  │  Resource   │              │
│  │   Pool      │  │  Manager    │  │   Limiter   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                       Memory Pool                                │
│            Buddy System Allocator (GPU Memory)                   │
└─────────────────────────────────────────────────────────────────┘
```

## 📚 Core Components

### Scheduler

The main entry point for task graph execution:

```cpp
hts::Scheduler scheduler;
scheduler.set_policy(std::make_unique<hts::GpuFirstPolicy>());
scheduler.set_profiling(true);
scheduler.execute();
```

### TaskBuilder (Fluent API)

```cpp
hts::TaskBuilder builder(scheduler.graph());

auto init = builder
    .name("Initialize")
    .device(hts::DeviceType::CPU)
    .priority(hts::TaskPriority::High)
    .cpu([](hts::TaskContext& ctx) { /* init */ })
    .build();

auto compute = builder
    .name("Compute")
    .after(init)
    .cpu([](hts::TaskContext& ctx) { /* compute */ })
    .build();
```

### TaskGroup

Batch task management:

```cpp
hts::TaskGroup workers("Workers", scheduler.graph());

for (int i = 0; i < 4; ++i) {
    auto task = workers.add_task(hts::DeviceType::CPU);
    task->set_cpu_function([i](hts::TaskContext& ctx) { /* process i */ });
}

workers.depends_on(init_task);
workers.then(final_task);
workers.set_priority(hts::TaskPriority::High);
```

### Scheduling Policies

```cpp
// Available policies:
scheduler.set_policy(std::make_unique<hts::DefaultSchedulingPolicy>());   // Load-based
scheduler.set_policy(std::make_unique<hts::GpuFirstPolicy>());            // GPU preferred
scheduler.set_policy(std::make_unique<hts::CpuFirstPolicy>());            // CPU preferred
scheduler.set_policy(std::make_unique<hts::RoundRobinPolicy>());          // Alternating
scheduler.set_policy(std::make_unique<hts::ShortestJobFirstPolicy>());    // Priority-based
```

### Profiling

```cpp
scheduler.set_profiling(true);
scheduler.execute();

auto summary = scheduler.profiler().generate_summary();
std::cout << "Total time: " << summary.total_time.count() / 1e6 << " ms\n";
std::cout << "Parallelism: " << summary.parallelism << "x\n";

// Or generate full report
std::cout << scheduler.profiler().generate_report();
```

### Event System

```cpp
hts::EventSystem events;

events.subscribe(hts::EventType::TaskCompleted, [](const hts::Event& e) {
    std::cout << "Task " << e.task_id << " completed\n";
});

events.subscribe_all([](const hts::Event& e) {
    std::cout << hts::EventSystem::event_type_name(e.type) << "\n";
});
```

### Retry Policies

```cpp
// Fixed retry with 100ms delay, max 3 attempts
auto fixed = hts::RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100});

// Exponential backoff (100ms, 200ms, 400ms, ...)
auto exp = hts::RetryPolicyFactory::exponential(5);

// Jittered backoff (adds randomness to avoid thundering herd)
auto jittered = hts::RetryPolicyFactory::jittered(5);

// Conditional retry (only retry transient errors)
auto conditional = hts::ConditionalRetryPolicy::transient_errors(
    hts::RetryPolicyFactory::exponential(5));
```

### Graph Serialization

```cpp
// Export to JSON
std::string json = hts::GraphSerializer::to_json(scheduler.graph());
hts::GraphSerializer::save_to_file(scheduler.graph(), "graph.json");

// Export to DOT (Graphviz)
hts::GraphSerializer::save_dot_file(scheduler.graph(), "graph.dot");
// Visualize: dot -Tpng graph.dot -o graph.png
```

## 🧪 Examples

Build and run the included examples:

| Example | Description |
|---------|-------------|
| `simple_dag` | Basic DAG execution |
| `parallel_pipeline` | Parallel processing pipeline |
| `error_handling` | Error propagation demo |
| `fluent_api` | TaskBuilder usage |
| `task_groups` | TaskGroup management |
| `profiling` | Performance profiler demo |
| `scheduling_policies` | Policy comparison |
| `graph_visualization` | Graph export to DOT/JSON |
| `gpu_computation` | CUDA kernel execution |
| `advanced_features` | Events, barriers, retry policies |

```bash
./build/simple_dag
./build/parallel_pipeline
./build/advanced_features
```

## 🔧 Configuration Options

```cpp
hts::SchedulerConfig config;

// Memory
config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB GPU memory pool
config.allow_memory_growth = true;             // Allow pool expansion

// Concurrency
config.cpu_thread_count = 4;                   // CPU worker threads
config.gpu_stream_count = 4;                   // CUDA streams

// Retry
config.max_retry_count = 3;                    // Max retries for failed tasks
config.retry_delay = std::chrono::milliseconds{100};
```

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit changes (`git commit -m 'feat: add amazing feature'`)
4. Push to branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🔗 Links

- **Documentation**: [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)
- **Repository**: [GitHub](https://github.com/LessUp/heterogeneous-task-scheduler)
- **Issues**: [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)

---

<p align="center">
  Made with ❤️ by the HTS Contributors
</p>
