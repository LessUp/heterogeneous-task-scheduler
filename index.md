---
layout: default
title: Heterogeneous Task Scheduler
---

# Heterogeneous Task Scheduler (HTS)

C++ 异构任务调度框架，支持在 CPU 和 GPU 之间调度和执行 DAG 任务图。

## 核心特性

- **DAG 任务依赖管理** — 自动检测循环依赖，确保正确的执行顺序
- **GPU 内存池** — 使用伙伴系统算法，避免频繁的 cudaMalloc/cudaFree
- **异步并发执行** — CPU 线程池 + CUDA 流，最大化硬件利用率
- **负载均衡** — 基于设备负载自动分配任务
- **性能监控** — 执行时间统计和时间线可视化

## 快速开始

```bash
# 构建
mkdir build && cd build
cmake ..
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

## 使用示例

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

hts::SchedulerConfig config;
config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
config.cpu_thread_count = 4;
config.gpu_stream_count = 4;

hts::Scheduler scheduler(config);

auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);
scheduler.graph().add_dependency(task1, task2);

scheduler.execute();
```

## 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++17, CUDA |
| 构建 | CMake 3.18+ |
| 测试 | Google Test, RapidCheck |

## 链接

- [GitHub 仓库](https://github.com/LessUp/heterogeneous-task-scheduler)
- [README](README.md)
