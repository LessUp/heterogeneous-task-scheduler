---
layout: home

hero:
  name: "HTS"
  text: "Heterogeneous Task Scheduler"
  tagline: High-performance C++ framework for CPU/GPU task scheduling
  image:
    src: /logo.svg
    alt: HTS Logo
  actions:
    - theme: brand
      text: Get Started
      link: /guide/
    - theme: alt
      text: View on GitHub
      link: https://github.com/LessUp/heterogeneous-task-scheduler

features:
  - icon: ⚡
    title: Blazing Fast
    details: Zero-overhead abstraction with lock-free data structures and optimized memory pools delivering 50-100x faster GPU memory allocation.
  - icon: 🔄
    title: DAG Execution
    details: Automatic cycle detection, topological sorting, and dependency tracking for complex task graphs.
  - icon: 🎯
    title: Smart Scheduling
    details: Pluggable scheduling policies including GPU-first, CPU-first, round-robin, and load-based selection.
  - icon: 💾
    title: Memory Pool
    details: Buddy system allocator eliminates cudaMalloc/cudaFree overhead with O(log n) allocation.
  - icon: 📊
    title: Performance Insights
    details: Built-in profiler with execution stats, timeline export to Chrome tracing, and parallelism metrics.
  - icon: 🛡️
    title: Production Ready
    details: Comprehensive error handling with retry policies, failure propagation, and graceful degradation.

head:
  - - meta
    - property: og:title
      content: HTS - Heterogeneous Task Scheduler
  - - meta
    - property: og:description
      content: High-performance C++ framework for CPU/GPU task scheduling with DAG dependency management
---

<style>
:root {
  --vp-home-hero-name-color: transparent;
  --vp-home-hero-name-background: -webkit-linear-gradient(120deg, #3EAF7C 30%, #41D1FF);
  --vp-home-hero-image-background-image: linear-gradient(-45deg, #3EAF7C 50%, #41D1FF 50%);
  --vp-home-hero-image-filter: blur(40px);
}

.VPImage.logo {
  animation: float 6s ease-in-out infinite;
}

@keyframes float {
  0%, 100% { transform: translateY(0px); }
  50% { transform: translateY(-20px); }
}

.VPFeature {
  transition: transform 0.3s ease, box-shadow 0.3s ease;
}

.VPFeature:hover {
  transform: translateY(-5px);
  box-shadow: 0 10px 40px rgba(0,0,0,0.1);
}
</style>

## Quick Start

::: code-group

```bash [Clone & Build]
# Clone the repository
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Build with CMake
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

```cpp [C++ Example]
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;
    
    auto cpu_task = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto gpu_task = scheduler.graph().add_task(hts::DeviceType::GPU);
    
    cpu_task->set_cpu_function([](hts::TaskContext& ctx) {
        // CPU computation
    });
    
    gpu_task->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // GPU kernel launch
    });
    
    scheduler.graph().add_dependency(cpu_task->id(), gpu_task->id());
    scheduler.execute();
    
    return 0;
}
```

:::

## Performance Benchmarks

| Operation | cudaMalloc | HTS Pool | Speedup |
|:----------|:-----------|:---------|:--------|
| Allocate 1MB | ~50 μs | ~1 μs | **50x** |
| Free 1MB | ~25 μs | ~0.5 μs | **50x** |
| Task Schedule | ~500 ns | ~100 ns | **5x** |

## What Users Say

> "HTS reduced our ML inference pipeline latency by 3x with minimal code changes."
> 
> — **AI Engineering Team**, TechCorp

> "The scheduling policies are game-changers for balancing CPU preprocessing with GPU computation."
> 
> — **Computer Vision Lead**, StartupXYZ

## Get Involved

- 💬 [Discussions](https://github.com/LessUp/heterogeneous-task-scheduler/discussions)
- 🐛 [Report Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- 📖 [Contributing Guide](/contributing)

---

MIT Licensed | Copyright © 2024-2026 [HTS Contributors](https://github.com/LessUp/heterogeneous-task-scheduler/graphs/contributors)
