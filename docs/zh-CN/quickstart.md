# 快速入门指南

> 在 5 分钟内上手 HTS

---

## 目录

- [前提条件](#前提条件)
- [基本概念](#基本概念)
- [第一个程序](#第一个程序)
- [流式 API](#流式-api)
- [CPU + GPU 工作流](#cpu--gpu-工作流)
- [错误处理](#错误处理)
- [下一步](#下一步)

---

## 前提条件

开始前，请确保您已：
- [ ] 完成[安装](installation.md)
- [ ] 拥有可用的 C++17 编译器
- [ ] 安装 CMake 3.18+
- [ ] 安装 CUDA Toolkit 11.0+（可选，用于 GPU 功能）

---

## 基本概念

### 核心组件

```
┌─────────────────────────────────────────────────────────────┐
│                          调度器                              │
│  ┌─────────────────┐  ┌─────────────────────────────────┐   │
│  │     任务图      │→ │          执行引擎                │   │
│  │     (DAG)       │  │   (CPU 线程池 + GPU 流)          │   │
│  └─────────────────┘  └─────────────────────────────────┘   │
│                          ┌─────────────────────────────┐     │
│                          │        内存池                │     │
│                          │   (伙伴系统分配器)            │     │
│                          └─────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

| 组件 | 用途 |
|------|------|
| `Scheduler` | 主调度器，协调执行 |
| `TaskGraph` | 任务图，定义 DAG |
| `Task` | 任务单元 |
| `TaskContext` | 执行上下文 |

---

## 第一个程序

创建一个简单的任务依赖图：

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 1. 创建默认配置的调度器
    hts::Scheduler scheduler;

    // 2. 添加任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::CPU);

    // 3. 设置任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "任务 1 在 CPU 上执行\n";
        ctx.set_output("result", 42);
    });

    task2->set_cpu_function([](hts::TaskContext& ctx) {
        auto result = ctx.get_input<int>("result");
        std::cout << "任务 2 收到: " << result << "\n";
    });

    // 4. 定义依赖（task2 依赖于 task1）
    scheduler.graph().add_dependency(task1->id(), task2->id());

    // 5. 执行图
    scheduler.execute();

    return 0;
}
```

**输出:**
```
任务 1 在 CPU 上执行
任务 2 收到: 42
```

### 编译和运行

```bash
# 保存为 first_program.cpp
g++ -std=c++17 -I./include first_program.cpp -o first_program -lpthread
./first_program
```

---

## 流式 API

TaskBuilder 提供了更直观的任务创建方式：

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;
    hts::TaskBuilder builder(scheduler.graph());

    // 链式任务创建
    auto init = builder
        .name("Initialize")
        .device(hts::DeviceType::CPU)
        .priority(hts::TaskPriority::High)
        .cpu([](hts::TaskContext& ctx) {
            std::cout << "初始化...\n";
            ctx.set_output("config", std::string("ready"));
        })
        .build();

    auto process = builder
        .name("Process")
        .after(init)                    // 依赖于 init
        .device(hts::DeviceType::CPU)
        .cpu([](hts::TaskContext& ctx) {
            auto config = ctx.get_input<std::string>("config");
            std::cout << "处理中: " << config << "\n";
        })
        .build();

    auto finalize = builder
        .name("Finalize")
        .after(process)                 // 依赖于 process
        .device(hts::DeviceType::Any)   // 让调度器决定
        .cpu([](hts::TaskContext& ctx) {
            std::cout << "完成...\n";
        })
        .build();

    scheduler.execute();
    return 0;
}
```

**流式 API 的优势：**
- 通过 `.after()` 清晰表示依赖链
- 自描述的任务名称
- 易于添加/删除任务

---

## CPU + GPU 工作流

一个常见模式：CPU 预处理 → GPU 计算 → CPU 后处理：

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

// 简单的 CUDA 内核示例
__global__ void multiply_kernel(float* data, float factor, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        data[idx] *= factor;
    }
}

int main() {
    hts::Scheduler scheduler;
    const int data_size = 1024;
    const float factor = 2.5f;

    // CPU: 分配并初始化数据
    auto preprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
    preprocess->set_cpu_function([data_size](hts::TaskContext& ctx) {
        // 通过上下文分配 GPU 内存
        void* d_data = ctx.allocate_gpu_memory(data_size * sizeof(float));
        
        // 初始化数据（实际应用会从主机复制）
        std::vector<float> host_data(data_size, 1.0f);
        cudaMemcpy(d_data, host_data.data(), data_size * sizeof(float), 
                   cudaMemcpyHostToDevice);
        
        ctx.set_output("data", d_data, data_size * sizeof(float));
        std::cout << "数据已准备到 GPU\n";
    });

    // GPU: 计算
    auto compute = scheduler.graph().add_task(hts::DeviceType::GPU);
    compute->set_gpu_function([data_size, factor](hts::TaskContext& ctx, 
                                                   cudaStream_t stream) {
        auto d_data = ctx.get_input<void*>("data");
        
        int block_size = 256;
        int grid_size = (data_size + block_size - 1) / block_size;
        
        multiply_kernel<<<grid_size, block_size, 0, stream>>>(
            static_cast<float*>(d_data), factor, data_size);
        
        ctx.set_output("result", d_data, data_size * sizeof(float));
        std::cout << "GPU 计算完成\n";
    });

    // CPU: 后处理
    auto postprocess = scheduler.graph().add_task(hts::DeviceType::CPU);
    postprocess->set_cpu_function([](hts::TaskContext& ctx) {
        auto d_data = ctx.get_input<void*>("result");
        
        // 复制回主机
        std::vector<float> host_data(1024);
        cudaMemcpy(host_data.data(), d_data, 1024 * sizeof(float),
                   cudaMemcpyDeviceToHost);
        
        float sum = 0;
        for (auto v : host_data) sum += v;
        std::cout << "处理后的总和: " << sum << "\n";
    });

    // 链式依赖
    scheduler.graph().add_dependency(preprocess->id(), compute->id());
    scheduler.graph().add_dependency(compute->id(), postprocess->id());

    scheduler.execute();
    return 0;
}
```

---

## 错误处理

优雅地处理任务失败：

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    hts::Scheduler scheduler;

    // 设置全局错误回调
    scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
        std::cerr << "[错误] 任务 " << id << " 失败: " << msg << "\n";
    });

    // 可能失败的任务
    auto risky_task = scheduler.graph().add_task(hts::DeviceType::CPU);
    risky_task->set_cpu_function([](hts::TaskContext& ctx) {
        // 模拟错误条件
        if (random() % 2 == 0) {
            throw std::runtime_error("随机故障");
        }
        std::cout << "任务成功！\n";
    });

    // 添加重试策略
    risky_task->set_retry_policy(
        hts::RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100})
    );

    scheduler.execute();
    return 0;
}
```

**要点：**
- 错误会传播到依赖任务
- 可以按任务配置重试策略
- 错误回调提供可见性

---

## 下一步

| 主题 | 学习内容 |
|------|---------|
| [架构概览](architecture.md) | 深入系统架构 |
| [API 参考](api-reference.md) | 完整 API 文档 |
| [调度策略](scheduling-policies.md) | 控制任务放置 |
| [内存管理](memory-management.md) | 优化 GPU 内存使用 |
| [示例](examples.md) | 更复杂的用例 |

---

## 速查卡

```cpp
// 创建调度器
hts::Scheduler scheduler;
scheduler.set_profiling(true);

// 添加任务（三种方式）
auto t1 = scheduler.graph().add_task(hts::DeviceType::CPU);
hts::TaskBuilder builder(scheduler.graph());
auto t2 = builder.name("Task2").cpu(...).build();

// 任务函数类型
t1->set_cpu_function([](hts::TaskContext& ctx) { /* CPU 工作 */ });
t1->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t s) { /* GPU 工作 */ });

// 依赖
scheduler.graph().add_dependency(t1->id(), t2->id());
builder.after(t1);  // 流式 API

// 执行
scheduler.execute();
```
