# Heterogeneous Task Scheduler (HTS)

[![Build Status](https://img.shields.io/badge/build-passing-brightgreen)]()
[![Tests](https://img.shields.io/badge/tests-passing-brightgreen)]()
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)
[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](CHANGELOG.md)

[English](README.md) | [简体中文](README.zh-CN.md)

> A high-performance C++ framework for scheduling and executing task DAGs (Directed Acyclic Graphs) across CPU and GPU devices.

---

## 📋 Table of Contents

- [Features](#-features)
- [Quick Start](#-quick-start)
- [Installation](#-installation)
- [Documentation](#-documentation)
- [Architecture](#-architecture)
- [Examples](#-examples)
- [Performance](#-performance)
- [Contributing](#-contributing)
- [License](#-license)

---

## ✨ Features

### Core Capabilities

| Feature | Description | Benefit |
|---------|-------------|---------|
| **DAG Management** | Automatic cycle detection, topological sorting, dependency tracking | Express complex workflows naturally |
| **GPU Memory Pool** | Buddy system allocator eliminates cudaMalloc/cudaFree overhead | 50-100x faster memory operations |
| **Async Execution** | CPU thread pool + CUDA streams for maximum hardware utilization | Parallel CPU/GPU execution |
| **Load Balancing** | Automatic task assignment based on device load | Optimal resource utilization |
| **Performance Monitoring** | Execution stats, timeline visualization, profiling reports | Data-driven optimization |
| **Flexible Scheduling** | Multiple policies (GPU-first, CPU-first, round-robin) | Workload-specific optimization |
| **Error Handling** | Error callbacks, failure propagation, retry policies | Robust production workloads |
| **Thread-Safe** | Safe concurrent task submission and execution | Multi-threaded applications |

### Why HTS?

```cpp
// Simple, intuitive API
auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);

// Define dependencies
scheduler.graph().add_dependency(task1->id(), task2->id());

// Execute with automatic scheduling
scheduler.execute();
```

---

## 🚀 Quick Start

### Installation

```bash
# Clone repository
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Build
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# Run tests
ctest --output-on-failure
```

### Your First Program

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // Create scheduler
    hts::Scheduler scheduler;

    // Create tasks
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);

    // Set task functions
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Hello from Task 1\n";
        ctx.set_output("message", std::string("Hello Task 2"));
    });

    task2->set_cpu_function([](hts::TaskContext& ctx) {
        auto msg = ctx.get_input<std::string>("message");
        std::cout << msg << "\n";
    });

    // Add dependency: task2 depends on task1
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // Execute
    scheduler.execute();
    return 0;
}
```

**Output:**
```
Hello from Task 1
Hello Task 2
```

---

## 📥 Installation

### Requirements

| Requirement | Version | Notes |
|-------------|---------|-------|
| CMake | >= 3.18 | Build system |
| CUDA Toolkit | >= 11.0 | GPU support (optional) |
| C++ Compiler | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |
| GPU | Compute Capability 5.0+ | For GPU tasks |

### Platform-Specific Instructions

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential cmake git
# Install CUDA from https://developer.nvidia.com/cuda-downloads
```

**macOS:**
```bash
brew install cmake git
# Note: GPU features not supported on macOS
```

**Windows:**
- Install Visual Studio 2019+ with C++ support
- Install CUDA Toolkit
- Install CMake

See [Installation Guide](docs/en/installation.md) for detailed instructions.

---

## 📖 Documentation

### Full Documentation

📚 **Complete documentation is available at [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)**

| Resource | Description | Link |
|----------|-------------|------|
| Installation Guide | Detailed setup instructions | [docs/en/installation.md](docs/en/installation.md) |
| Quick Start | 5-minute introduction | [docs/en/quickstart.md](docs/en/quickstart.md) |
| Architecture | System design overview | [docs/en/architecture.md](docs/en/architecture.md) |
| API Reference | Complete API documentation | [docs/en/api-reference.md](docs/en/api-reference.md) |
| Examples | All example walkthroughs | [docs/en/examples.md](docs/en/examples.md) |

### 中文文档

| 文档 | 描述 | 链接 |
|------|------|------|
| 安装指南 | 详细安装说明 | [docs/zh-CN/installation.md](docs/zh-CN/installation.md) |
| 快速入门 | 5 分钟入门 | [docs/zh-CN/quickstart.md](docs/zh-CN/quickstart.md) |
| 架构概览 | 系统设计概述 | [docs/zh-CN/architecture.md](docs/zh-CN/architecture.md) |
| API 参考 | 完整 API 文档 | [docs/zh-CN/api-reference.md](docs/zh-CN/api-reference.md) |
| 示例教程 | 所有示例详解 | [docs/zh-CN/examples.md](docs/zh-CN/examples.md) |

---

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

### Key Components

1. **TaskGraph**: DAG representation with cycle detection
2. **Scheduler**: Central coordinator with pluggable policies
3. **Execution Engine**: Thread pool + CUDA streams
4. **Memory Pool**: Efficient GPU memory management
5. **Profiler**: Performance monitoring and reporting

---

## 💡 Examples

### CPU + GPU Pipeline

```cpp
// CPU preprocessing → GPU computation → CPU postprocessing

// Step 1: CPU preprocess
auto preprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
preprocess->set_cpu_function([](hts::TaskContext& ctx) {
    void* d_data = ctx.allocate_gpu_memory(1024);
    // Upload data...
    ctx.set_output("data", d_data, 1024);
});

// Step 2: GPU compute
auto compute = scheduler.graph().add_task(hts::DeviceType::GPU);
compute->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
    auto data = ctx.get_input<void*>("data");
    my_kernel<<<blocks, threads, 0, stream>>>(data);
});

// Step 3: CPU postprocess
auto postprocess = scheduler.graph().add_task(hts::DeviceType::CPU);

// Chain them
scheduler.graph().add_dependency(preprocess->id(), compute->id());
scheduler.graph().add_dependency(compute->id(), postprocess->id());
scheduler.execute();
```

### Fluent API

```cpp
hts::TaskBuilder builder(scheduler.graph());

auto result = builder
    .name("ProcessData")
    .device(hts::DeviceType::GPU)
    .after(load_task)
    .gpu([](hts::TaskContext& ctx, cudaStream_t stream) {
        // GPU processing
    })
    .retry(hts::RetryPolicyFactory::exponential(3))
    .build();
```

See [examples/](examples/) directory for complete examples.

---

## 📊 Performance

### Memory Allocation

| Operation | cudaMalloc | HTS Memory Pool | Speedup |
|-----------|------------|-----------------|---------|
| Allocate 1 MB | ~50 μs | ~1 μs | **50x** |
| Free 1 MB | ~25 μs | ~1 μs | **25x** |

### Scheduling Overhead

| Operation | Latency |
|-----------|---------|
| Add task | ~50 ns |
| Add dependency | ~30 ns |
| Schedule task | ~100 ns |

### Typical Workloads

| Workload | CPU-only | HTS (CPU+GPU) | Speedup |
|----------|----------|---------------|---------|
| Image Processing | 1.0x | 3.5x | **3.5x** |
| ML Inference | 1.0x | 8.2x | **8.2x** |
| Data Pipeline | 1.0x | 2.1x | **2.1x** |

See [docs/en/profiling.md](docs/en/profiling.md) for profiling guide.

---

## 🛣️ Roadmap

### Current (v1.2.0)

- ✅ Bilingual documentation (English/Chinese)
- ✅ Comprehensive API documentation
- ✅ Professional changelog structure

### Planned (v1.3.0)

- 🔄 Multi-GPU support
- 🔄 Distributed execution
- 🔄 Python bindings

### Future (v2.0.0)

- 📋 WebAssembly support
- 📋 Cloud-native scheduling
- 📋 Auto-tuning policies

---

## 🤝 Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

### Quick Start for Contributors

```bash
# Fork and clone
git clone https://github.com/YOUR_USERNAME/heterogeneous-task-scheduler.git

# Create branch
git checkout -b feature/amazing-feature

# Make changes and test
mkdir build && cd build
cmake .. && make -j$(nproc) && ctest

# Commit and push
git commit -m "feat: add amazing feature"
git push origin feature/amazing-feature

# Open Pull Request
```

---

## 📄 License

This project is licensed under the MIT License - see [LICENSE](LICENSE) file for details.

---

## 🙏 Acknowledgments

- Inspired by modern task scheduling systems
- Built with modern C++ best practices
- Thanks to all contributors

---

## 🔗 Links

- **Documentation**: [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)
- **Repository**: [GitHub](https://github.com/LessUp/heterogeneous-task-scheduler)
- **Issues**: [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- **Changelog**: [CHANGELOG.md](CHANGELOG.md)

---

<p align="center">
  <strong>HTS</strong> — High-performance heterogeneous computing made simple.
  <br>
  Made with ❤️ by the HTS Contributors
</p>
