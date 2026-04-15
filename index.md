---
layout: default
title: Heterogeneous Task Scheduler
---

# Heterogeneous Task Scheduler (HTS)

[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)

**C++ 异构任务调度框架** — 在 CPU 和 GPU 之间高效调度和执行 DAG 任务图

## ✨ 核心特性

| 特性 | 描述 |
|------|------|
| **DAG 管理** | 自动循环检测、拓扑排序、依赖追踪 |
| **GPU 内存池** | 伙伴系统分配器，高效管理 GPU 内存 |
| **异步执行** | CPU 线程池 + CUDA 流并发执行 |
| **负载均衡** | 基于设备负载自动任务分配 |
| **性能监控** | 执行统计、时间线可视化、性能报告 |

## 🚀 快速开始

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

## 📖 使用示例

### 基本用法

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::SchedulerConfig config;
    config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;

    hts::Scheduler scheduler(config);

    // 创建任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);

    // 添加依赖
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // 执行
    scheduler.execute();

    return 0;
}
```

### 流式 API

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

## 📚 文档

- **完整 README**: [README.md](README.md) | [README.zh-CN.md](README.zh-CN.md)
- **API 参考**: 参见 `include/hts/` 目录下的头文件
- **示例代码**: `examples/` 目录
- **更新日志**: [CHANGELOG.md](CHANGELOG.md)

## 🛠️ 技术栈

| 类别 | 技术 |
|------|------|
| 语言 | C++17, CUDA |
| 构建 | CMake 3.18+ |
| 测试 | Google Test, RapidCheck |
| 文档 | GitHub Pages (Jekyll) |

## 🧪 示例程序

| 示例 | 描述 |
|------|------|
| `simple_dag` | 基本 DAG 执行 |
| `parallel_pipeline` | 并行流水线 |
| `error_handling` | 错误处理 |
| `fluent_api` | 流式 API |
| `advanced_features` | 高级功能 |

## 📦 安装

```bash
# 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git

# 构建
mkdir build && cd build
cmake .. && make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

## 🔗 链接

- **GitHub**: [LessUp/heterogeneous-task-scheduler](https://github.com/LessUp/heterogeneous-task-scheduler)
- **Issues**: [提交问题](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- **Releases**: [下载发布版本](https://github.com/LessUp/heterogeneous-task-scheduler/releases)

---

<p align="center">
  由 HTS 贡献者用 ❤️ 制作
</p>
