# Heterogeneous Task Scheduler (HTS)

[![Docs](https://img.shields.io/badge/Docs-GitHub%20Pages-blue?logo=github)](https://lessup.github.io/heterogeneous-task-scheduler/)

[English](README.md) | 简体中文

一个 C++ 框架，用于在 CPU 和 GPU 之间调度和执行任务 DAG（有向无环图）。

## 特性

- **DAG 任务依赖管理**：自动检测循环依赖，确保正确的执行顺序
- **GPU 内存池**：使用伙伴系统算法，避免频繁的 cudaMalloc/cudaFree
- **异步并发执行**：CPU 线程池 + CUDA 流，最大化硬件利用率
- **负载均衡**：基于设备负载自动分配任务
- **性能监控**：执行时间统计和时间线可视化

## 依赖

- CMake >= 3.18
- CUDA Toolkit
- C++17 编译器
- Google Test (自动下载)
- RapidCheck (自动下载)

## 构建

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 运行测试

```bash
cd build
ctest --output-on-failure
```

## 使用示例

### 基本用法

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 创建调度器
    hts::SchedulerConfig config;
    config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;
    
    hts::Scheduler scheduler(config);
    
    // 创建任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);
    auto task3 = scheduler.graph().add_task(hts::DeviceType::Any);
    
    // 设置 CPU 任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 1 running on CPU\n";
    });
    
    // 设置 GPU 任务函数
    task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // 在这里启动 CUDA kernel
        std::cout << "Task 2 running on GPU\n";
    });
    
    // 设置可在任意设备运行的任务
    task3->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 3 running on CPU\n";
    });
    task3->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        std::cout << "Task 3 running on GPU\n";
    });
    
    // 添加依赖关系
    scheduler.graph().add_dependency(task1->id(), task2->id());
    scheduler.graph().add_dependency(task1->id(), task3->id());
    
    // 执行
    scheduler.execute();
    
    return 0;
}
```

### Fluent Builder API

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;
    hts::TaskBuilder builder(scheduler.graph());
    
    auto init = builder
        .name("Initialize")
        .device(hts::DeviceType::CPU)
        .priority(hts::TaskPriority::High)
        .cpu([](hts::TaskContext& ctx) {
            // 初始化逻辑
        })
        .build();
    
    auto compute = builder
        .name("Compute")
        .after(init)  // 依赖 init 任务
        .cpu([](hts::TaskContext& ctx) {
            // 计算逻辑
        })
        .build();
    
    scheduler.execute();
    return 0;
}
```

### 错误处理

```cpp
scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
    std::cerr << "Task " << id << " failed: " << msg << "\n";
});
```

### 日志系统

```cpp
#include <hts/logger.hpp>

// 设置日志级别
hts::Logger::instance().set_level(hts::LogLevel::Debug);

// 使用宏记录日志
HTS_LOG_INFO("Processing {} items", count);
HTS_LOG_ERROR("Failed to allocate memory");

// 自定义日志回调
hts::Logger::instance().set_callback([](hts::LogLevel level, const std::string& msg) {
    // 自定义日志处理
});
```

## 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                          │
├─────────────────────────────────────────────────────────────────┤
│                      TaskGraph Builder API                       │
├─────────────────────────────────────────────────────────────────┤
│                          Scheduler                               │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ Dependency      │  │ Device          │  │ Performance     │  │
│  │ Manager         │  │ Manager         │  │ Monitor         │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                      Execution Engine                            │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ CPU Executor    │  │ GPU Executor    │  │ Stream Manager  │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                       Memory Pool                                │
└─────────────────────────────────────────────────────────────────┘
```

## 许可证

MIT License

## 示例程序

构建后可以运行以下示例：

```bash
./build/simple_dag            # 简单 DAG 执行
./build/parallel_pipeline     # 并行流水线
./build/error_handling        # 错误处理演示
./build/fluent_api            # Fluent Builder API
./build/task_groups           # 任务组管理
./build/profiling             # 性能分析
./build/scheduling_policies   # 调度策略对比
./build/graph_visualization  # 任务图可视化
./build/gpu_computation      # GPU 计算示例
```

## 高级功能

### 任务组

```cpp
hts::TaskGroup workers("Workers", scheduler.graph());

// 批量创建任务
for (int i = 0; i < 4; ++i) {
    auto task = workers.add_task(hts::DeviceType::CPU);
    task->set_cpu_function([i](hts::TaskContext& ctx) {
        // 处理第 i 个分片
    });
}

// 设置依赖关系
workers.depends_on(init_task);  // 所有 worker 依赖 init
workers.then(final_task);       // final 依赖所有 worker

// 批量操作
workers.set_priority(hts::TaskPriority::High);
workers.cancel();  // 取消所有任务
```

### 调度策略

```cpp
// 使用 GPU 优先策略
scheduler.set_policy(std::make_unique<hts::GpuFirstPolicy>());

// 可用策略:
// - DefaultSchedulingPolicy: 基于负载的默认策略
// - GpuFirstPolicy: GPU 优先
// - CpuFirstPolicy: CPU 优先
// - RoundRobinPolicy: 轮询
// - ShortestJobFirstPolicy: 短作业优先
```

### 性能分析

```cpp
// 启用性能分析
scheduler.set_profiling(true);

scheduler.execute();

// 生成报告
std::cout << scheduler.profiler().generate_report();

// 程序化访问
auto summary = scheduler.profiler().generate_summary();
std::cout << "Parallelism: " << summary.parallelism << "x\n";
```

### 任务图序列化

```cpp
#include <hts/graph_serializer.hpp>

// 导出为 JSON
std::string json = hts::GraphSerializer::to_json(scheduler.graph());
hts::GraphSerializer::save_to_file(scheduler.graph(), "graph.json");

// 导出为 DOT (Graphviz)
std::string dot = hts::GraphSerializer::to_dot(scheduler.graph());
hts::GraphSerializer::save_dot_file(scheduler.graph(), "graph.dot");
// 可视化: dot -Tpng graph.dot -o graph.png
```

### CUDA 工具

```cpp
#include <hts/cuda_utils.hpp>

// 检查 CUDA 可用性
if (hts::CudaUtils::is_available()) {
    std::cout << hts::CudaUtils::device_info_string();
}

// RAII 设备内存
hts::DeviceMemory<float> d_data(1024);
d_data.copy_from_host(host_data.data());

// RAII 锁页内存
hts::PinnedMemory<float> pinned(1024);

// 作用域设备切换
{
    hts::ScopedDevice device(0);
    // 在 GPU 0 上操作
}
```

### 事件系统

```cpp
#include <hts/event_system.hpp>

hts::EventSystem events;

// 订阅任务事件
auto sub = events.subscribe(hts::EventType::TaskCompleted, [](const hts::Event& e) {
    std::cout << "Task " << e.task_id << " completed\n";
});

// 订阅所有事件
events.subscribe_all([](const hts::Event& e) {
    std::cout << hts::EventSystem::event_type_name(e.type) << "\n";
});

// RAII 订阅管理
{
    hts::ScopedSubscription scoped(events, sub);
    // 订阅在作用域结束时自动取消
}
```

### 任务屏障

```cpp
#include <hts/task_barrier.hpp>

// 创建屏障同步点
hts::TaskBarrier barrier("phase1_complete", scheduler.graph());

// 添加前置任务
for (auto& task : phase1_tasks) {
    barrier.add_predecessor(task);
}

// 添加后续任务
for (auto& task : phase2_tasks) {
    barrier.add_successor(task);
}

// 等待屏障
barrier.wait();
```

### 重试策略

```cpp
#include <hts/retry_policy.hpp>

// 固定延迟重试
auto fixed = hts::RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100});

// 指数退避
auto exponential = hts::RetryPolicyFactory::exponential(5);

// 带抖动的指数退避
auto jittered = hts::RetryPolicyFactory::jittered(5);

// 条件重试（仅重试瞬态错误）
auto conditional = hts::ConditionalRetryPolicy::transient_errors(
    hts::RetryPolicyFactory::exponential(5));
```

### 资源限制

```cpp
#include <hts/resource_limiter.hpp>

hts::ResourceLimiter::Limits limits;
limits.max_concurrent_cpu_tasks = 4;
limits.max_concurrent_gpu_tasks = 2;
limits.max_memory_bytes = 1024 * 1024 * 1024;  // 1 GB

hts::ResourceLimiter limiter(limits);

// 获取资源槽
if (limiter.acquire_cpu_slot()) {
    // 执行任务
    limiter.release_cpu_slot();
}

// RAII 资源管理
{
    hts::ResourceSlotGuard guard(limiter, hts::DeviceType::CPU);
    // 资源在作用域结束时自动释放
}
```

### 任务结果获取

```cpp
#include <hts/task_future.hpp>

hts::TaskFuture<int> future(task);

// 在任务中设置结果
task->set_cpu_function([&future](hts::TaskContext& ctx) {
    int result = compute();
    future.set_value(result);
});

// 获取结果
scheduler.execute();
int value = future.get();  // 阻塞直到结果可用

// 非阻塞检查
if (auto result = future.try_get()) {
    std::cout << "Result: " << *result << "\n";
}
```
