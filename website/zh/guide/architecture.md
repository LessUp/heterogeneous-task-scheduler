# 架构

本页只记录仓库当前实现出的结构，而不是“未来可能长成什么样”的设计草图。

## 运行时分层

| 层 | 核心类型 | 实现文件 |
|----|----------|----------|
| 图定义 | `Task`、`TaskGraph`、`TaskBuilder` | `include/hts/task.hpp`、`include/hts/task_graph.hpp`、`include/hts/task_builder.hpp`、`src/core/task.cpp`、`src/core/task_graph.cpp` |
| 调度 | `Scheduler`、`SchedulingPolicy`、`DependencyManager` | `include/hts/scheduler.hpp`、`include/hts/scheduling_policy.hpp`、`src/core/scheduler.cpp`、`src/core/scheduling_policy.cpp`、`src/core/dependency_manager.cpp` |
| 执行 | `ExecutionEngine`、`MemoryPool`、`StreamManager` | `include/hts/execution_engine.hpp`、`include/hts/memory_pool.hpp`、`include/hts/stream_manager.hpp`、`src/core/execution_engine.cpp`、`src/cuda/` |
| 工具 | `TaskContext`、`Profiler`、`GraphSerializer`、`RetryPolicy`、`EventSystem`、`ResourceLimiter` | `include/hts/` 下的公共头文件，以及 `tests/unit/` 中的覆盖用例 |

## 执行流程

当前 `src/core/scheduler.cpp` 中的 `Scheduler` 基本按以下顺序工作：

1. 校验内存中的 `TaskGraph`。
2. 为当前运行创建 `DependencyManager`。
3. 收集 ready task，并交给当前 `SchedulingPolicy` 排序。
4. 通过 `select_device(...)` 选择 CPU 或 GPU 执行。
5. 将任务提交给 `ExecutionEngine`。
6. 标记完成任务、解除后继阻塞，直到图完全排空。
7. 汇总 `ExecutionStats` 与时间线数据。

## CPU-only 与 CUDA 路径

- **CPU-only 模式** 会构建共享运行时以及 `src/cuda/cuda_stubs.cpp`。
- **CUDA 模式** 会启用 `src/cuda/memory_pool.cu` 和 `src/cuda/stream_manager.cu`。
- 公共头文件在 `include/hts/types.hpp` 中前置声明了 CUDA 类型，因此大部分代码不必直接包含 CUDA runtime。

## 为什么文档要明显收缩

旧站点把真实架构说明、理论论文、占位性能结论和已经不存在的 API 混在了一起。现在边界很简单：

- 本页只描述仓库今天真实存在的文件级结构。
- [API 概览](/zh/api/) 只映射公共头文件。
- [示例](/zh/examples/) 只指向当前仓库内确实能构建的示例程序。
