# Quick Start Guide

> 5 分钟快速上手 HTS

---

## 基本概念

### 核心组件

```
┌─────────────────────────────────────────────┐
│                  Scheduler                   │
│  ┌─────────────┐  ┌─────────────────────┐   │
│  │  TaskGraph  │→ │ Execution Engine    │   │
│  │  (DAG)      │  │ (CPU Pool + GPU)    │   │
│  └─────────────┘  └─────────────────────┘   │
│                  ┌─────────────────────┐     │
│                  │    Memory Pool      │     │
│                  └─────────────────────┘     │
└─────────────────────────────────────────────┘
```

| 组件 | 作用 |
|------|------|
| `Scheduler` | 主调度器，协调执行 |
| `TaskGraph` | 任务图，定义 DAG |
| `Task` | 任务单元 |
| `TaskContext` | 执行上下文 |

---

## 第一个程序

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 1. 创建调度器
    hts::Scheduler scheduler;

    // 2. 添加任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);

    // 3. 设置任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 1 executed\n";
    });

    task2->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 2 executed\n";
    });

    // 4. 添加依赖
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // 5. 执行
    scheduler.execute();

    return 0;
}
```

**输出:**
```
Task 1 executed
Task 2 executed
```

---

## 使用 Fluent API

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;
    hts::TaskBuilder builder(scheduler.graph());

    // 链式创建任务
    auto init = builder
        .name("Init")
        .device(hts::DeviceType::CPU)
        .cpu([](hts::TaskContext&) { /* init */ })
        .build();

    auto process = builder
        .name("Process")
        .after(init)  // 依赖 init
        .cpu([](hts::TaskContext&) { /* process */ })
        .build();

    scheduler.execute();
    return 0;
}
```

---

## CPU + GPU 任务

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;

    // CPU 预处理
    auto preprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
    preprocess->set_cpu_function([](hts::TaskContext& ctx) {
        // 分配 GPU 内存
        void* d_data = ctx.allocate_gpu_memory(1024);
        ctx.set_output("data", d_data, 1024);
    });

    // GPU 计算
    auto compute = scheduler.graph().add_task(hts::DeviceType::GPU);
    compute->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // 获取上游数据
        auto data = ctx.get_input<void>("data");
        // 启动 kernel
        // my_kernel<<<grid, block, 0, stream>>>(data);
    });

    // 添加依赖
    scheduler.graph().add_dependency(preprocess->id(), compute->id());

    // 执行
    scheduler.execute();

    return 0;
}
```

---

## 错误处理

```cpp
hts::Scheduler scheduler;

// 设置错误回调
scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
    std::cerr << "Task " << id << " failed: " << msg << "\n";
});

// 任务可能失败
auto task = scheduler.graph().add_task(hts::DeviceType::CPU);
task->set_cpu_function([](hts::TaskContext& ctx) {
    throw std::runtime_error("Something went wrong");
});

scheduler.execute();
```

---

## 下一步

- [安装指南](installation.md)
- [API 参考](api/scheduler.md)
- [示例代码](../examples/)
