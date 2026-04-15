# Heterogeneous Task Scheduler (HTS)

[![Docs](https://img.shields.io/badge/Docs-GitHub%20Pages-blue?logo=github)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green.svg)](https://developer.nvidia.com/cuda-toolkit)

[English](README.md) | 简体中文

一个高性能 C++ 框架，用于在 CPU 和 GPU 设备之间调度和执行任务 DAG（有向无环图）。

## ✨ 特性

| 特性 | 描述 |
|------|------|
| **DAG 管理** | 自动循环检测、拓扑排序、依赖追踪 |
| **GPU 内存池** | 伙伴系统分配器，避免频繁的 cudaMalloc/cudaFree |
| **异步执行** | CPU 线程池 + CUDA 流，最大化硬件利用率 |
| **负载均衡** | 基于设备负载自动分配任务 |
| **性能监控** | 执行统计、时间线可视化、性能分析报告 |
| **灵活调度** | 多种调度策略（GPU优先、CPU优先、轮询） |
| **错误处理** | 错误回调、失败传播、重试策略 |
| **线程安全** | 安全的并发任务提交和执行 |

## 📋 依赖要求

| 依赖 | 版本 |
|------|------|
| CMake | >= 3.18 |
| CUDA Toolkit | >= 11.0 |
| C++ 编译器 | 支持 C++17 (GCC 8+, Clang 7+, MSVC 2019+) |
| Google Test | 自动下载 |
| RapidCheck | 自动下载 |

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
cd build
ctest --output-on-failure
```

### 基本用法

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 创建调度器并配置
    hts::SchedulerConfig config;
    config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;

    hts::Scheduler scheduler(config);

    // 创建任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);
    auto task3 = scheduler.graph().add_task(hts::DeviceType::Any);

    // 设置任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "任务 1 在 CPU 上执行\n";
    });

    task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // 在这里启动 CUDA kernel
    });

    // 同时支持 CPU 和 GPU 函数的任务 - 调度器自动选择
    task3->set_cpu_function([](hts::TaskContext& ctx) { /* CPU 版本 */ });
    task3->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t s) { /* GPU 版本 */ });

    // 添加依赖：task2 和 task3 依赖于 task1
    scheduler.graph().add_dependency(task1->id(), task2->id());
    scheduler.graph().add_dependency(task1->id(), task3->id());

    // 设置错误处理
    scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
        std::cerr << "任务 " << id << " 失败: " << msg << "\n";
    });

    scheduler.execute();

    return 0;
}
```

## 📖 文档

- **API 参考**：参见 [`include/hts/`](include/hts/) 目录下的头文件
- **示例代码**：参见 [`examples/`](examples/) 目录
- **更新日志**：参见 [CHANGELOG.md](CHANGELOG.md)

## 🎯 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        用户应用程序                               │
├─────────────────────────────────────────────────────────────────┤
│                      任务图构建 API                               │
│    TaskBuilder │ TaskGroup │ TaskBarrier │ TaskFuture            │
├─────────────────────────────────────────────────────────────────┤
│                          调度器                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │   依赖      │  │   调度      │  │  性能分析   │              │
│  │   管理器    │  │   策略      │  │  与日志     │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                        执行引擎                                   │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ CPU 线程    │  │ GPU 流      │  │   资源      │              │
│  │   池        │  │   管理器    │  │   限制器    │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                        内存池                                     │
│              伙伴系统分配器（GPU 内存）                            │
└─────────────────────────────────────────────────────────────────┘
```

## 📚 核心组件

### Scheduler（调度器）

任务图执行的主要入口：

```cpp
hts::Scheduler scheduler;
scheduler.set_policy(std::make_unique<hts::GpuFirstPolicy>());
scheduler.set_profiling(true);
scheduler.execute();
```

### TaskBuilder（流式 API）

```cpp
hts::TaskBuilder builder(scheduler.graph());

auto init = builder
    .name("初始化")
    .device(hts::DeviceType::CPU)
    .priority(hts::TaskPriority::High)
    .cpu([](hts::TaskContext& ctx) { /* 初始化 */ })
    .build();

auto compute = builder
    .name("计算")
    .after(init)
    .cpu([](hts::TaskContext& ctx) { /* 计算 */ })
    .build();
```

### TaskGroup（任务组）

批量任务管理：

```cpp
hts::TaskGroup workers("工作线程组", scheduler.graph());

for (int i = 0; i < 4; ++i) {
    auto task = workers.add_task(hts::DeviceType::CPU);
    task->set_cpu_function([i](hts::TaskContext& ctx) { /* 处理 i */ });
}

workers.depends_on(init_task);
workers.then(final_task);
workers.set_priority(hts::TaskPriority::High);
```

### 调度策略

```cpp
// 可用的调度策略：
scheduler.set_policy(std::make_unique<hts::DefaultSchedulingPolicy>());   // 基于负载
scheduler.set_policy(std::make_unique<hts::GpuFirstPolicy>());            // GPU 优先
scheduler.set_policy(std::make_unique<hts::CpuFirstPolicy>());            // CPU 优先
scheduler.set_policy(std::make_unique<hts::RoundRobinPolicy>());          // 轮询
scheduler.set_policy(std::make_unique<hts::ShortestJobFirstPolicy>());    // 基于优先级
```

### 性能分析

```cpp
scheduler.set_profiling(true);
scheduler.execute();

auto summary = scheduler.profiler().generate_summary();
std::cout << "总时间: " << summary.total_time.count() / 1e6 << " ms\n";
std::cout << "并行度: " << summary.parallelism << "x\n";

// 或生成完整报告
std::cout << scheduler.profiler().generate_report();
```

### 事件系统

```cpp
hts::EventSystem events;

events.subscribe(hts::EventType::TaskCompleted, [](const hts::Event& e) {
    std::cout << "任务 " << e.task_id << " 已完成\n";
});

events.subscribe_all([](const hts::Event& e) {
    std::cout << hts::EventSystem::event_type_name(e.type) << "\n";
});
```

### 重试策略

```cpp
// 固定延迟重试，最多 3 次，每次间隔 100ms
auto fixed = hts::RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100});

// 指数退避（100ms, 200ms, 400ms, ...）
auto exp = hts::RetryPolicyFactory::exponential(5);

// 带抖动的指数退避（添加随机性避免惊群效应）
auto jittered = hts::RetryPolicyFactory::jittered(5);

// 条件重试（仅重试瞬态错误）
auto conditional = hts::ConditionalRetryPolicy::transient_errors(
    hts::RetryPolicyFactory::exponential(5));
```

### 图序列化

```cpp
// 导出为 JSON
std::string json = hts::GraphSerializer::to_json(scheduler.graph());
hts::GraphSerializer::save_to_file(scheduler.graph(), "graph.json");

// 导出为 DOT（Graphviz）
hts::GraphSerializer::save_dot_file(scheduler.graph(), "graph.dot");
// 可视化: dot -Tpng graph.dot -o graph.png
```

## 🧪 示例程序

构建并运行内置示例：

| 示例 | 描述 |
|------|------|
| `simple_dag` | 基本 DAG 执行 |
| `parallel_pipeline` | 并行处理流水线 |
| `error_handling` | 错误传播演示 |
| `fluent_api` | TaskBuilder 使用方法 |
| `task_groups` | TaskGroup 管理 |
| `profiling` | 性能分析器演示 |
| `scheduling_policies` | 策略对比 |
| `graph_visualization` | 图导出为 DOT/JSON |
| `gpu_computation` | CUDA kernel 执行 |
| `advanced_features` | 事件、屏障、重试策略 |

```bash
./build/simple_dag
./build/parallel_pipeline
./build/advanced_features
```

## 🔧 配置选项

```cpp
hts::SchedulerConfig config;

// 内存
config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB GPU 内存池
config.allow_memory_growth = true;             // 允许池扩展

// 并发
config.cpu_thread_count = 4;                   // CPU 工作线程数
config.gpu_stream_count = 4;                   // CUDA 流数量

// 重试
config.max_retry_count = 3;                    // 失败任务最大重试次数
config.retry_delay = std::chrono::milliseconds{100};
```

## 🤝 贡献

欢迎贡献！请参阅 [CONTRIBUTING.md](CONTRIBUTING.md) 了解贡献指南。

1. Fork 仓库
2. 创建特性分支 (`git checkout -b feature/amazing-feature`)
3. 提交更改 (`git commit -m 'feat: add amazing feature'`)
4. 推送到分支 (`git push origin feature/amazing-feature`)
5. 创建 Pull Request

## 📄 许可证

本项目采用 MIT 许可证 - 详情参见 [LICENSE](LICENSE) 文件。

## 🔗 链接

- **文档**：[GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/)
- **仓库**：[GitHub](https://github.com/LessUp/heterogeneous-task-scheduler)
- **问题反馈**：[GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)

---

<p align="center">
  由 HTS 贡献者用 ❤️ 制作
</p>
