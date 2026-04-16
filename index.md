---
layout: default
title: Heterogeneous Task Scheduler
description: C++ 异构任务调度框架，支持 CPU/GPU 任务 DAG 执行
---

{%- if jekyll.environment == "production" -%}
{%- endif -%}

# Heterogeneous Task Scheduler (HTS)

[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)
[![Docs](https://img.shields.io/badge/Docs-GitHub%20Pages-blue.svg)](https://lessup.github.io/heterogeneous-task-scheduler/)

**高性能 C++ 异构任务调度框架** — 在 CPU 和 GPU 之间高效调度和执行 DAG 任务图

---

## 🎯 为什么选择 HTS？

| 特性 | 描述 |
|------|------|
| 🔄 **DAG 任务管理** | 自动循环检测、拓扑排序、依赖追踪 |
| 💾 **GPU 内存池** | 伙伴系统分配器，避免频繁 cudaMalloc |
| ⚡ **异步并发** | CPU 线程池 + CUDA 流，最大化硬件利用率 |
| ⚖️ **负载均衡** | 基于设备负载自动任务分配 |
| 📊 **性能监控** | 执行统计、时间线可视化、性能报告 |
| 🛡️ **容错机制** | 错误回调、失败传播、重试策略 |

---

## 🚀 快速开始 {#quick-start}

### 安装依赖

- CMake >= 3.18
- CUDA Toolkit >= 11.0
- C++17 编译器 (GCC 8+, Clang 7+, MSVC 2019+)

### 构建

```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### 运行测试

```bash
ctest --output-on-failure
```

---

## 💡 使用示例

### 基本用法

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::SchedulerConfig config;
    config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB

    hts::Scheduler scheduler(config);

    // 创建任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);

    // 设置任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        // CPU 计算
    });

    task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // GPU kernel 启动
    });

    // 添加依赖
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // 执行
    scheduler.execute();

    return 0;
}
```

### Fluent API

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

scheduler.execute();
```

---

## 📖 API 参考 {#api-reference}

### 核心组件

| 类 | 描述 |
|----|------|
| `Scheduler` | 主调度器，协调任务图执行 |
| `TaskGraph` | DAG 构建、验证、遍历 |
| `Task` | 任务定义，支持 CPU/GPU 函数 |
| `TaskBuilder` | 流式任务构建 API |
| `TaskGroup` | 批量任务管理 |
| `TaskFuture<T>` | 异步结果获取 |

### 调度策略

| 策略 | 描述 |
|------|------|
| `DefaultSchedulingPolicy` | 基于负载自动选择设备 |
| `GpuFirstPolicy` | GPU 优先 |
| `CpuFirstPolicy` | CPU 优先 |
| `RoundRobinPolicy` | 轮询调度 |

### 高级功能

| 组件 | 描述 |
|------|------|
| `MemoryPool` | GPU 内存池 |
| `Profiler` | 性能分析器 |
| `EventSystem` | 事件订阅系统 |
| `ResourceLimiter` | 资源限制器 |
| `RetryPolicy` | 重试策略 |

---

## 🧪 示例程序 {#examples}

| 示例 | 描述 | 文件 |
|------|------|------|
| 基本 DAG | 基础任务图执行 | `simple_dag.cpp` |
| 并行流水线 | 并行处理模式 | `parallel_pipeline.cpp` |
| 错误处理 | 错误回调演示 | `error_handling.cpp` |
| Fluent API | 流式构建器 | `fluent_api.cpp` |
| 任务组 | 批量操作 | `task_groups.cpp` |
| 性能分析 | Profiler 使用 | `profiling.cpp` |
| 调度策略 | 策略对比 | `scheduling_policies.cpp` |
| 图可视化 | DOT/JSON 导出 | `graph_visualization.cpp` |
| GPU 计算 | CUDA kernel | `gpu_computation.cu` |
| 高级功能 | 事件/屏障/重试 | `advanced_features.cpp` |

---

## 📚 文档

| 文档 | 描述 |
|------|------|
| [README.md](README.md) | 英文文档 |
| [README.zh-CN.md](README.zh-CN.md) | 中文文档 |
| [CHANGELOG.md](CHANGELOG.md) | 更新日志 |
| [CONTRIBUTING.md](CONTRIBUTING.md) | 贡献指南 |

---

## 🛠️ 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++17, CUDA |
| 构建 | CMake 3.18+ |
| 测试 | Google Test, RapidCheck |
| 文档 | GitHub Pages (Jekyll) |

---

## 🤝 贡献

欢迎贡献！请查看 [CONTRIBUTING.md](CONTRIBUTING.md) 了解详情。

---

## 📄 许可证

[MIT License](LICENSE)

---

## 🔗 链接

- **GitHub**: [LessUp/heterogeneous-task-scheduler](https://github.com/LessUp/heterogeneous-task-scheduler)
- **Issues**: [提交问题](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- **Releases**: [下载发布版本](https://github.com/LessUp/heterogeneous-task-scheduler/releases)

---

<p align="center">
  由 HTS 贡献者用 ❤️ 制作
</p>
