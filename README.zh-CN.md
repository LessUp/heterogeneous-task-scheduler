# 异构任务调度器 (HTS)

[![GitHub Release](https://img.shields.io/github/v/release/LessUp/heterogeneous-task-scheduler?include_prereleases&logo=github)](https://github.com/LessUp/heterogeneous-task-scheduler/releases)
[![CI](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml/badge.svg?event=push)](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)
[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)](CONTRIBUTING.md)

[English](README.md) | 简体中文

> **高性能 C++ 框架，用于跨 CPU 和 GPU 设备调度和执行任务 DAG（有向无环图）。**

---

## 📋 目录

- [特性](#-特性)
- [快速开始](#-快速开始)
- [文档](#-文档)
- [架构](#-架构)
- [示例](#-示例)
- [性能](#-性能)
- [路线图](#️-路线图)
- [贡献](#-贡献)
- [许可](#-许可)

---

## ✨ 特性

### 为什么选择 HTS？

| 特性 | 优势 |
|------|------|
| 🚀 **极速性能** | 零开销抽象、无锁数据结构，GPU 内存分配快 50-100 倍 |
| 🔄 **DAG 执行** | 自动环检测、拓扑排序、依赖追踪 |
| 🎯 **智能调度** | 可插拔策略：GPU 优先、CPU 优先、轮询、基于负载 |
| 💾 **内存池** | 伙伴系统分配器消除 cudaMalloc/cudaFree 开销 |
| 📊 **性能洞察** | 内置性能分析器，支持 Chrome tracing 导出和并行度指标 |
| 🛡️ **生产就绪** | 重试策略、失败传播、优雅降级 |

### 快速示例

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // 创建 CPU 任务
    auto cpu_task = builder
        .create_task("预处理")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "在 CPU 上预处理..." << std::endl;
        })
        .build();
    
    // 创建 GPU 任务
    auto gpu_task = builder
        .create_task("计算")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            my_kernel<<<256, 128, 0, stream>>>(data);
            cudaStreamSynchronize(stream);
        })
        .build();
    
    // 设置依赖
    graph.add_dependency(cpu_task->id(), gpu_task->id());
    
    // 执行
    Scheduler scheduler;
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

---

## 🚀 快速开始

### 从源码构建

```bash
# 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 构建（使用脚本）
scripts/build.sh --cpu-only  # 或使用 scripts/build.sh 支持 CUDA

# 运行测试
scripts/test.sh
```

### 在你的项目中使用

**使用 CMake FetchContent：**

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

## 📖 文档

### 🌐 完整网站

**📚 完整文档请访问 [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)**

网站包含：

- 📖 **入门指南** - 安装、快速开始、架构
- 📘 **API 参考** - 完整的 Scheduler、TaskGraph、TaskBuilder 文档
- 💡 **示例** - 从简单到复杂的工作代码示例
- 📊 **性能指南** - 性能分析和优化技巧
- 🛡️ **错误处理** - 重试策略、回退方案、最佳实践

### 关键页面

| 主题 | 链接 |
|------|------|
| 安装指南 | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/guide/installation) |
| 快速开始教程 | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/guide/quickstart) |
| 架构概览 | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/guide/architecture) |
| Scheduler API | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/api/scheduler) |
| TaskGraph API | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/api/task-graph) |
| 示例 | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/examples/) |
| 更新日志 | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/changelog) |
| 贡献指南 | [访问网站 →](https://lessup.github.io/heterogeneous-task-scheduler/contributing) |

### 技术规范

技术设计文档和产品需求在 [`/specs`](specs/) 目录中：

| 资源 | 链接 |
|------|------|
| 产品需求 | [specs/product/](specs/product/) |
| 架构 RFC | [specs/rfc/](specs/rfc/) |
| 测试规范 | [specs/testing/](specs/testing/) |

---

## 🎯 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        用户应用程序                                │
├─────────────────────────────────────────────────────────────────┤
│                      TaskGraph 构建器 API                         │
│    TaskBuilder │ TaskGroup │ TaskBarrier │ TaskFuture            │
├─────────────────────────────────────────────────────────────────┤
│                          调度器                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ 依赖管理     │  │  调度策略    │  │  性能分析    │              │
│  │             │  │             │  │  与日志      │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                      执行引擎                                    │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ CPU 线程池   │  │ GPU 流管理   │  │  资源限制    │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                       内存池                                     │
│            伙伴系统分配器 (GPU 内存)                               │
└─────────────────────────────────────────────────────────────────┘
```

### 核心组件

1. **TaskGraph** - DAG 表示，支持环检测
2. **Scheduler** - 中央协调器，可插拔策略
3. **Execution Engine** - 线程池 + CUDA 流
4. **Memory Pool** - 高效的 GPU 内存管理
5. **Profiler** - 性能监控和报告

---

## 💡 示例

### CPU + GPU 流水线

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // CPU 预处理
    auto preprocess = builder
        .create_task("预处理")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "预处理数据..." << std::endl;
        })
        .build();
    
    // GPU 计算
    auto compute = builder
        .create_task("GPU计算")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            my_kernel<<<256, 128, 0, stream>>>(data);
            cudaStreamSynchronize(stream);
        })
        .priority(10)
        .build();
    
    // CPU 后处理
    auto postprocess = builder
        .create_task("后处理")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "后处理结果..." << std::endl;
        })
        .build();
    
    // 设置依赖：预处理 → 计算 → 后处理
    graph.add_dependency(preprocess->id(), compute->id());
    graph.add_dependency(compute->id(), postprocess->id());
    
    // 执行
    Scheduler scheduler;
    scheduler.set_policy(std::make_unique<GPUPriorityPolicy>());
    scheduler.init(&graph);
    scheduler.execute();
    scheduler.wait_for_completion();
    
    return 0;
}
```

### 带重试策略

```cpp
// GPU 任务，失败时自动重试
auto unreliable_task = builder
    .create_task("风险GPU任务")
    .device(DeviceType::GPU)
    .gpu_func(risky_kernel)
    .retry_policy(RetryPolicy{
        .max_retries = 3,
        .backoff_ms = 100,
        .backoff_multiplier = 2.0f
    })
    .fallback([](TaskContext& ctx) {
        std::cout << "GPU 失败，使用 CPU 回退方案" << std::endl;
        cpu_fallback(ctx);
    })
    .build();
```

**更多示例：**

- [简单 DAG](https://lessup.github.io/heterogeneous-task-scheduler/examples/simple-dag) - 基础流水线
- [复杂流水线](https://lessup.github.io/heterogeneous-task-scheduler/examples/pipeline) - 带错误处理的 ML 流水线
- [examples/](examples/) 目录包含完整示例

---

## 📊 性能

### 内存分配

| 操作 | cudaMalloc | HTS 内存池 | 加速 |
|------|------------|------------|------|
| 分配 1 MB | ~50 μs | ~1 μs | **50x** |
| 释放 1 MB | ~25 μs | ~1 μs | **25x** |

### 调度开销

| 操作 | 延迟 |
|------|------|
| 添加任务 | ~50 ns |
| 添加依赖 | ~30 ns |
| 调度任务 | ~100 ns |

### 典型工作负载

| 工作负载 | 仅 CPU | HTS (CPU+GPU) | 加速 |
|----------|--------|---------------|------|
| 图像处理 | 1.0x | 3.5x | **3.5x** |
| ML 推理 | 1.0x | 8.2x | **8.2x** |
| 数据流水线 | 1.0x | 2.1x | **2.1x** |

---

## 🛣️ 路线图

### 当前版本 (v1.2.0)

- ✅ 双语文档（英文/中文）
- ✅ 完整的 API 文档
- ✅ 专业的更新日志结构

### 计划中 (v1.3.0)

- 🔄 多 GPU 支持
- 🔄 分布式执行
- 🔄 Python 绑定

### 未来 (v2.0.0)

- 📋 WebAssembly 支持
- 📋 云原生调度
- 📋 自动调优策略

---

## 🤝 贡献

我们欢迎各种贡献！详见 [贡献指南](CONTRIBUTING.md)。

### 贡献者快速入门

```bash
# 1. Fork 并克隆
git clone https://github.com/YOUR_USERNAME/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 2. 创建特性分支
git checkout -b feature/amazing-feature

# 3. 构建和测试
scripts/build.sh --cpu-only
scripts/test.sh

# 4. 格式化代码
scripts/format.sh

# 5. 提交并推送
git commit -m "feat: 添加超赞特性"
git push origin feature/amazing-feature

# 6. 提交 Pull Request
```

### 贡献类型

- 🐛 **Bug 修复** - 总是欢迎！
- 📝 **文档** - 指南、示例、API 文档
- ✨ **新功能** - 请先在 Issues 中讨论
- 🎨 **代码质量** - 重构、风格改进
- 🧪 **测试** - 提高覆盖率、添加边界用例
- 💡 **示例** - 实际使用场景

---

## 📄 许可

本项目基于 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件。

---

## 🔗 链接

- **📚 文档**: [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)
- **💻 仓库**: [GitHub](https://github.com/LessUp/heterogeneous-task-scheduler)
- **🐛 问题追踪**: [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- **💬 讨论**: [GitHub Discussions](https://github.com/LessUp/heterogeneous-task-scheduler/discussions)
- **📝 更新日志**: [网站](https://lessup.github.io/heterogeneous-task-scheduler/changelog)

---

## 🌟 Star 历史

如果您觉得 HTS 有用，请在 GitHub 上给我们一个 ⭐️！

---

<p align="center">
  <strong>HTS</strong> — 让高性能异构计算变得简单。
  <br>
  由 HTS 贡献者用 ❤️ 打造
</p>
