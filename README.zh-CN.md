# 异构任务调度器 (HTS)

[![Build Status](https://img.shields.io/badge/build-通过-brightgreen)]()
[![Tests](https://img.shields.io/badge/tests-通过-brightgreen)]()
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)
[![Version](https://img.shields.io/badge/version-1.2.0-blue.svg)](CHANGELOG.md)

[English](README.md) | 简体中文

> 一个高性能 C++ 框架，用于在 CPU 和 GPU 设备之间调度和执行任务 DAG（有向无环图）。

---

## 📋 目录

- [特性](#-特性)
- [快速开始](#-快速开始)
- [安装](#-安装)
- [文档](#-文档)
- [架构](#-架构)
- [示例](#-示例)
- [性能](#-性能)
- [贡献](#-贡献)
- [许可](#-许可)

---

## ✨ 特性

### 核心能力

| 特性 | 描述 | 优势 |
|------|------|------|
| **DAG 管理** | 自动环检测、拓扑排序、依赖追踪 | 自然地表达复杂工作流 |
| **GPU 内存池** | 伙伴系统分配器，消除 cudaMalloc/cudaFree 开销 | 内存操作速度提升 50-100 倍 |
| **异步执行** | CPU 线程池 + CUDA 流，最大化硬件利用率 | CPU/GPU 并行执行 |
| **负载均衡** | 基于设备负载自动分配任务 | 优化资源利用 |
| **性能监控** | 执行统计、时间线可视化、分析报告 | 数据驱动的优化 |
| **灵活调度** | 多种策略（GPU 优先、CPU 优先、轮询）| 针对工作负载优化 |
| **错误处理** | 错误回调、失败传播、重试策略 | 健壮的生产工作流 |
| **线程安全** | 安全的并发任务提交和执行 | 多线程应用 |

### 为什么选择 HTS？

```cpp
// 简单、直观的 API
auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);

// 定义依赖
scheduler.graph().add_dependency(task1->id(), task2->id());

// 自动调度执行
scheduler.execute();
```

---

## 🚀 快速开始

### 安装

```bash
# 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

### 第一个程序

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 创建调度器
    hts::Scheduler scheduler;

    // 创建任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);

    // 设置任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "来自任务 1 的问候\n";
        ctx.set_output("message", std::string("你好任务 2"));
    });

    task2->set_cpu_function([](hts::TaskContext& ctx) {
        auto msg = ctx.get_input<std::string>("message");
        std::cout << msg << "\n";
    });

    // 添加依赖：task2 依赖于 task1
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // 执行
    scheduler.execute();
    return 0;
}
```

**输出:**
```
来自任务 1 的问候
你好任务 2
```

---

## 📥 安装

### 要求

| 要求 | 版本 | 说明 |
|------|------|------|
| CMake | >= 3.18 | 构建系统 |
| CUDA Toolkit | >= 11.0 | GPU 支持（可选）|
| C++ 编译器 | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |
| GPU | 计算能力 5.0+ | 用于 GPU 任务 |

### 平台特定说明

**Ubuntu/Debian:**
```bash
sudo apt-get install build-essential cmake git
# 从 https://developer.nvidia.com/cuda-downloads 安装 CUDA
```

**macOS:**
```bash
brew install cmake git
# 注意：macOS 不支持 GPU 功能
```

**Windows:**
- 安装 Visual Studio 2019+ 及 C++ 支持
- 安装 CUDA Toolkit
- 安装 CMake

详细说明请参见[安装指南](docs/zh-CN/installation.md)。

---

## 📖 文档

### 完整文档

📚 **完整文档请访问 [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)**

| 资源 | 描述 | 链接 |
|------|------|------|
| 安装指南 | 详细设置说明 | [docs/zh-CN/installation.md](docs/zh-CN/installation.md) |
| 快速入门 | 5 分钟入门 | [docs/zh-CN/quickstart.md](docs/zh-CN/quickstart.md) |
| 架构概览 | 系统设计概述 | [docs/zh-CN/architecture.md](docs/zh-CN/architecture.md) |
| API 参考 | 完整 API 文档 | [docs/zh-CN/api-reference.md](docs/zh-CN/api-reference.md) |
| 示例教程 | 所有示例详解 | [docs/zh-CN/examples.md](docs/zh-CN/examples.md) |

### English Documentation

| Document | Description | Link |
|----------|-------------|------|
| Installation Guide | Detailed setup | [docs/en/installation.md](docs/en/installation.md) |
| Quick Start | 5-minute intro | [docs/en/quickstart.md](docs/en/quickstart.md) |
| Architecture | Design overview | [docs/en/architecture.md](docs/en/architecture.md) |
| API Reference | API docs | [docs/en/api-reference.md](docs/en/api-reference.md) |
| Examples | Walkthroughs | [docs/en/examples.md](docs/en/examples.md) |

---

## 🎯 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        用户应用程序                              │
├─────────────────────────────────────────────────────────────────┤
│                        任务图构建 API                            │
│    TaskBuilder │ TaskGroup │ TaskBarrier │ TaskFuture            │
├─────────────────────────────────────────────────────────────────┤
│                           调度器                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │    依赖     │  │    调度     │  │   性能分析   │              │
│  │   管理器    │  │    策略     │  │   与日志     │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                         执行引擎                                 │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │   CPU 线程  │  │   GPU 流    │  │    资源     │              │
│  │     池      │  │   管理器    │  │    限制器   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                          内存池                                  │
│              伙伴系统分配器（GPU 内存）                           │
└─────────────────────────────────────────────────────────────────┘
```

### 主要组件

1. **TaskGraph（任务图）**: 带环检测的 DAG 表示
2. **Scheduler（调度器）**: 带插件策略的中央协调器
3. **Execution Engine（执行引擎）**: 线程池 + CUDA 流
4. **Memory Pool（内存池）**: 高效的 GPU 内存管理
5. **Profiler（分析器）**: 性能监控和报告

---

## 💡 示例

### CPU + GPU 流水线

```cpp
// CPU 预处理 → GPU 计算 → CPU 后处理

// 第 1 步：CPU 预处理
auto preprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
preprocess->set_cpu_function([](hts::TaskContext& ctx) {
    void* d_data = ctx.allocate_gpu_memory(1024);
    // 上传数据...
    ctx.set_output("data", d_data, 1024);
});

// 第 2 步：GPU 计算
auto compute = scheduler.graph().add_task(hts::DeviceType::GPU);
compute->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
    auto data = ctx.get_input<void*>("data");
    my_kernel<<<blocks, threads, 0, stream>>>(data);
});

// 第 3 步：CPU 后处理
auto postprocess = scheduler.graph().add_task(hts::DeviceType::CPU);

// 链式连接
scheduler.graph().add_dependency(preprocess->id(), compute->id());
scheduler.graph().add_dependency(compute->id(), postprocess->id());
scheduler.execute();
```

### 流式 API

```cpp
hts::TaskBuilder builder(scheduler.graph());

auto result = builder
    .name("ProcessData")
    .device(hts::DeviceType::GPU)
    .after(load_task)
    .gpu([](hts::TaskContext& ctx, cudaStream_t stream) {
        // GPU 处理
    })
    .retry(hts::RetryPolicyFactory::exponential(3))
    .build();
```

完整示例请参见 [examples/](examples/) 目录。

---

## 📊 性能

### 内存分配

| 操作 | cudaMalloc | HTS 内存池 | 加速比 |
|------|------------|-----------|--------|
| 分配 1 MB | ~50 μs | ~1 μs | **50x** |
| 释放 1 MB | ~25 μs | ~1 μs | **25x** |

### 调度开销

| 操作 | 延迟 |
|------|------|
| 添加任务 | ~50 ns |
| 添加依赖 | ~30 ns |
| 调度任务 | ~100 ns |

### 典型工作负载

| 工作负载 | 仅 CPU | HTS (CPU+GPU) | 加速比 |
|----------|--------|---------------|--------|
| 图像处理 | 1.0x | 3.5x | **3.5x** |
| ML 推理 | 1.0x | 8.2x | **8.2x** |
| 数据流水线 | 1.0x | 2.1x | **2.1x** |

分析指南请参见 [docs/zh-CN/profiling.md](docs/zh-CN/profiling.md)。

---

## 🛣️ 路线图

### 当前 (v1.2.0)

- ✅ 双语文档（英文/中文）
- ✅ 全面的 API 文档
- ✅ 专业的变更日志结构

### 计划 (v1.3.0)

- 🔄 多 GPU 支持
- 🔄 分布式执行
- 🔄 Python 绑定

### 未来 (v2.0.0)

- 📋 WebAssembly 支持
- 📋 云原生调度
- 📋 自动调优策略

---

## 🤝 贡献

我们欢迎贡献！请参阅 [CONTRIBUTING.md](CONTRIBUTING.md) 了解指南。

### 贡献者快速开始

```bash
# Fork 并克隆
git clone https://github.com/YOUR_USERNAME/heterogeneous-task-scheduler.git

# 创建分支
git checkout -b feature/amazing-feature

# 修改并测试
mkdir build && cd build
cmake .. && make -j$(nproc) && ctest

# 提交并推送
git commit -m "feat: add amazing feature"
git push origin feature/amazing-feature

# 创建 Pull Request
```

---

## 📄 许可

本项目采用 MIT 许可证 - 详情请参见 [LICENSE](LICENSE) 文件。

---

## 🙏 鸣谢

- 受现代任务调度系统启发
- 采用现代 C++ 最佳实践构建
- 感谢所有贡献者

---

## 🔗 链接

- **文档**: [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)
- **仓库**: [GitHub](https://github.com/LessUp/heterogeneous-task-scheduler)
- **问题**: [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- **变更日志**: [CHANGELOG.md](CHANGELOG.md)

---

<p align="center">
  <strong>HTS</strong> — 让高性能异构计算变得简单。
  <br>
  由 HTS 贡献者用 ❤️ 制作
</p>
