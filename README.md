# Heterogeneous Task Scheduler (HTS)

[![GitHub Release](https://img.shields.io/github/v/release/LessUp/heterogeneous-task-scheduler?include_prereleases&logo=github)](https://github.com/LessUp/heterogeneous-task-scheduler/releases)
[![CI](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml/badge.svg)](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

[English](README.md) | [简体中文](README.zh-CN.md)

> **High-performance C++ framework for scheduling and executing task DAGs across CPU and GPU devices.**

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

### Why Choose HTS?

| Feature | Benefit |
|---------|---------|
| 🚀 **Blazing Fast** | Zero-overhead abstractions, lock-free data structures, 50-100x faster GPU memory allocation |
| 🔄 **DAG Execution** | Automatic cycle detection, topological sorting, dependency tracking |
| 🎯 **Smart Scheduling** | Pluggable policies: GPU-first, CPU-first, round-robin, load-based |
| 💾 **Memory Pool** | Buddy system allocator eliminates cudaMalloc/cudaFree overhead |
| 📊 **Performance Insights** | Built-in profiler with Chrome tracing export and parallelism metrics |
| 🛡️ **Production Ready** | Retry policies, failure propagation, graceful degradation |

### Quick Example

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Create CPU task
    auto cpu_task = builder
        .create_task("Preprocess")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "Preprocessing on CPU..." << std::endl;
        })
        .build();
    
    // Create GPU task
    auto gpu_task = builder
        .create_task("Compute")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            my_kernel<<<256, 128, 0, stream>>>(data);
            cudaStreamSynchronize(stream);
        })
        .build();
    
    // Set dependency
    graph.add_dependency(cpu_task->id(), gpu_task->id());
    
    // Execute
    Scheduler scheduler;
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

---

## 🚀 Quick Start

### Build from Source

```bash
# Clone repository
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Build (using scripts)
scripts/build.sh --cpu-only  # or scripts/build.sh for CUDA support

# Run tests
scripts/test.sh
```

### Use in Your Project

**With CMake FetchContent:**

```cmake
include(FetchContent)
FetchContent_Declare(
    hts
    GIT_REPOSITORY https://github.com/LessUp/heterogeneous-task-scheduler.git
    GIT_TAG        v1.2.0
)
FetchContent_MakeAvailable(hts)

target_link_libraries(your_target PRIVATE hts_lib)
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

### 🌐 Complete Website

**📚 Full documentation is available at [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)**

The website includes:

- 📖 **Getting Started Guides** - Installation, quickstart, architecture
- 📘 **API Reference** - Complete Scheduler, TaskGraph, TaskBuilder documentation
- 💡 **Examples** - Working code examples from simple to complex
- 📊 **Performance Guides** - Profiling and optimization tips
- 🛡️ **Error Handling** - Retry policies, fallbacks, best practices

### Key Pages

| Topic | Link |
|-------|------|
| Installation Guide | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/guide/installation) |
| Quick Start Tutorial | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/guide/quickstart) |
| Architecture Overview | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/guide/architecture) |
| Scheduler API | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/api/scheduler) |
| TaskGraph API | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/api/task-graph) |
| Examples | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/examples/) |
| Changelog | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/changelog) |
| Contributing Guide | [Website →](https://lessup.github.io/heterogeneous-task-scheduler/contributing) |

### Specifications

Technical design documents and product requirements are in the [`/specs`](specs/) directory:

| Resource | Link |
|----------|------|
| Product Requirements | [specs/product/](specs/product/) |
| Architecture RFC | [specs/rfc/](specs/rfc/) |
| Test Specifications | [specs/testing/](specs/testing/) |

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
#include <hts/heterogeneous_task_scheduler.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // CPU preprocessing
    auto preprocess = builder
        .create_task("Preprocess")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "Preprocessing data..." << std::endl;
        })
        .build();
    
    // GPU computation
    auto compute = builder
        .create_task("GPUCompute")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            my_kernel<<<256, 128, 0, stream>>>(data);
            cudaStreamSynchronize(stream);
        })
        .priority(10)
        .build();
    
    // CPU postprocessing
    auto postprocess = builder
        .create_task("Postprocess")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "Postprocessing results..." << std::endl;
        })
        .build();
    
    // Set dependencies: preprocess → compute → postprocess
    graph.add_dependency(preprocess->id(), compute->id());
    graph.add_dependency(compute->id(), postprocess->id());
    
    // Execute
    Scheduler scheduler;
    scheduler.set_policy(std::make_unique<GPUPriorityPolicy>());
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

### With Retry Policy

```cpp
// GPU task with automatic retry on failure
auto unreliable_task = builder
    .create_task("RiskyGPUTask")
    .device(DeviceType::GPU)
    .gpu_func(risky_kernel)
    .retry_policy(RetryPolicy{
        .max_retries = 3,
        .backoff_ms = 100,
        .backoff_multiplier = 2.0f
    })
    .fallback([](TaskContext& ctx) {
        std::cout << "GPU failed, using CPU fallback" << std::endl;
        cpu_fallback(ctx);
    })
    .build();
```

**More examples:**

- [Simple DAG](https://lessup.github.io/heterogeneous-task-scheduler/examples/simple-dag) - Basic pipeline
- [Pipeline](https://lessup.github.io/heterogeneous-task-scheduler/examples/pipeline) - Complex ML pipeline with error handling
- [examples/](examples/) directory for complete examples

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

We welcome contributions! See our [Contributing Guide](CONTRIBUTING.md) for details.

### Quick Start for Contributors

```bash
# 1. Fork and clone
git clone https://github.com/YOUR_USERNAME/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 2. Create feature branch
git checkout -b feature/amazing-feature

# 3. Build and test
scripts/build.sh --cpu-only
scripts/test.sh

# 4. Format code
scripts/format.sh

# 5. Commit and push
git commit -m "feat: add amazing feature"
git push origin feature/amazing-feature

# 6. Open Pull Request
```

### Contribution Types

- 🐛 **Bug Fixes** - Always welcome!
- 📝 **Documentation** - Guides, examples, API docs
- ✨ **New Features** - Please discuss in Issues first
- 🎨 **Code Quality** - Refactoring, style improvements
- 🧪 **Tests** - Increase coverage, add edge cases
- 💡 **Examples** - Real-world use cases

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

- **📚 Documentation**: [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)
- **💻 Repository**: [GitHub](https://github.com/LessUp/heterogeneous-task-scheduler)
- **🐛 Issue Tracker**: [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- 💬 **Discussions**: [GitHub Discussions](https://github.com/LessUp/heterogeneous-task-scheduler/discussions)
- 📝 **Changelog**: [Website](https://lessup.github.io/heterogeneous-task-scheduler/changelog)

---

## 🌟 Star History

If you find HTS useful, please consider giving it a ⭐️ on GitHub!

---

<p align="center">
  <strong>HTS</strong> — High-performance heterogeneous computing made simple.
  <br>
  Made with ❤️ by the HTS Contributors
</p>
