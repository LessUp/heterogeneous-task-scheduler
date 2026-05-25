# 指南

站点现在只保留可维护的内容：构建、验证、运行时结构和仓库内真实存在的示例。凡是不由当前代码面支撑的说明都已删除。

## 开发基线

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

这套 CPU-first 基线适用于本地开发和 CI 风格验证。CUDA 场景是在此基础上的增量能力，不再作为默认入口。

## 稳定概念

| 概念 | 作用 | 主要文件 |
|------|------|----------|
| `TaskGraph` | 保存任务与依赖关系 DAG | `include/hts/task_graph.hpp`, `src/core/task_graph.cpp` |
| `TaskBuilder` | 在 `TaskGraph` 之上的流式建图接口 | `include/hts/task_builder.hpp` |
| `Scheduler` | 校验图、调度 ready task、记录统计和时间线 | `include/hts/scheduler.hpp`, `src/core/scheduler.cpp` |
| `ExecutionEngine` | 将任务分发到 CPU worker 与 CUDA stream | `include/hts/execution_engine.hpp`, `src/core/execution_engine.cpp` |
| `MemoryPool` / `StreamManager` | GPU 运行时服务，CPU-only 时有 stub | `include/hts/memory_pool.hpp`, `include/hts/stream_manager.hpp`, `src/cuda/` |

## 文档边界

- 公共 API 概览只对应 `include/hts/` 下的真实头文件。
- 示例页只记录 `examples/` 目录中实际存在并可构建的程序。
- 发布历史只保留在根目录 [`CHANGELOG.md`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/CHANGELOG.md)。

## 继续阅读

1. [安装](./installation) 了解构建模式与前置依赖。
2. [快速开始](./quickstart) 运行一个最小 CPU-only DAG。
3. [架构](./architecture) 查看执行流程与代码布局。
