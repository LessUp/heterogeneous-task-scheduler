# 示例教程

> 所有 HTS 示例程序的详细解释

---

## 目录

- [概览](#概览)
- [Simple DAG](#simple-dag)
- [Parallel Pipeline](#parallel-pipeline)
- [Fluent API](#fluent-api)
- [Task Groups](#task-groups)
- [GPU Computation](#gpu-computation)
- [Error Handling](#error-handling)
- [Profiling](#profiling)
- [Scheduling Policies](#scheduling-policies)
- [Graph Visualization](#graph-visualization)
- [Advanced Features](#advanced-features)

---

## 概览

HTS 包含 10 个示例程序，展示各种功能。每个示例都是可编译运行的完整程序。

### 构建示例

```bash
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 运行示例
./simple_dag
./parallel_pipeline
./fluent_api
# ... 等等
```

---

## Simple DAG

**文件**: `examples/simple_dag.cpp`

**概念**: 基础任务图构建、依赖

### 描述

展示最简单的任务依赖图：a → b → c，每个任务依赖于前一个。

### 关键代码

```cpp
// 创建三个任务
auto task_a = scheduler.graph().add_task(DeviceType::CPU);
auto task_b = scheduler.graph().add_task(DeviceType::CPU);
auto task_c = scheduler.graph().add_task(DeviceType::CPU);

// 设置函数
task_a->set_cpu_function([](TaskContext& ctx) {
    std::cout << "任务 A\n";
});

task_b->set_cpu_function([](TaskContext& ctx) {
    std::cout << "任务 B\n";
});

task_c->set_cpu_function([](TaskContext& ctx) {
    std::cout << "任务 C\n";
});

// 定义依赖: a → b → c
scheduler.graph().add_dependency(task_a->id(), task_b->id());
scheduler.graph().add_dependency(task_b->id(), task_c->id());

// 执行
scheduler.execute();
```

### 输出

```
任务 A
任务 B
任务 C
```

### 学习要点

1. 使用 `add_task()` 创建任务
2. 用 lambda 设置 CPU 函数
3. 用 `add_dependency()` 添加依赖
4. 执行顺序是拓扑序，非创建顺序

---

## Parallel Pipeline

**文件**: `examples/parallel_pipeline.cpp`

**概念**: 并行阶段、数据流、流水线模式

### 描述

经典的多阶段并行流水线：多个数据项同时流经 加载→处理→保存 阶段。

### 架构

```
数据项 1:  [加载] → [处理] → [保存]
数据项 2:  [加载] → [处理] → [保存]
数据项 3:  [加载] → [处理] → [保存]
          
时间 →     跨数据项并行执行
```

---

## Fluent API

**文件**: `examples/fluent_api.cpp`

**概念**: TaskBuilder、方法链、可读代码

### 描述

展示可读性更好的 TaskBuilder 流式 API。

---

## Task Groups

**文件**: `examples/task_groups.cpp`

**概念**: TaskGroup、批量操作、工作者模式

### 描述

展示如何管理具有共享依赖的相关任务组。

---

## GPU Computation

**文件**: `examples/gpu_computation.cu`

**概念**: CUDA 内核、CPU/GPU 工作流、内存管理

### 描述

完整的 CPU 预处理 → GPU 计算 → CPU 后处理工作流。

---

## Error Handling

**文件**: `examples/error_handling.cpp`

**概念**: 错误回调、重试策略、失败传播

### 描述

展示各种错误处理模式。

---

## Profiling

**文件**: `examples/profiling.cpp`

**概念**: 性能分析、性能指标、时间线导出

### 描述

展示如何启用分析并解读结果。

---

## Scheduling Policies

**文件**: `examples/scheduling_policies.cpp`

**概念**: 不同策略、策略对比

### 描述

在相同工作负载下对比不同调度策略。

---

## Graph Visualization

**文件**: `examples/graph_visualization.cpp`

**概念**: 图导出、DOT 格式、JSON 序列化

### 描述

展示如何导出任务图用于可视化。

---

## Advanced Features

**文件**: `examples/advanced_features.cpp`

**概念**: 任务屏障、futures、事件、资源限制

### 描述

展示高级同步和控制功能。

---

## 运行所有示例

```bash
#!/bin/bash
cd build

echo "=== Simple DAG ==="
./simple_dag

echo -e "\n=== Parallel Pipeline ==="
./parallel_pipeline

echo -e "\n=== Fluent API ==="
./fluent_api

echo -e "\n=== Task Groups ==="
./task_groups

echo -e "\n=== GPU Computation ==="
./gpu_computation

echo -e "\n=== Error Handling ==="
./error_handling

echo -e "\n=== Profiling ==="
./profiling

echo -e "\n=== Scheduling Policies ==="
./scheduling_policies

echo -e "\n=== Graph Visualization ==="
./graph_visualization
ls -la *.dot *.json 2>/dev/null

echo -e "\n=== Advanced Features ==="
./advanced_features
```

---

## 进一步阅读

- [快速入门](quickstart.md) - HTS 入门
- [架构概览](architecture.md) - 理解系统设计
- [API 参考](api-reference.md) - 完整 API 文档
