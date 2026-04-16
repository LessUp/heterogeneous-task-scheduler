# API 参考

> HTS 完整 API 文档

---

## 目录

- [Scheduler](#scheduler)
- [TaskGraph](#taskgraph)
- [Task](#task)
- [TaskBuilder](#taskbuilder)
- [TaskContext](#taskcontext)
- [TaskGroup](#taskgroup)
- [Scheduling Policies](#scheduling-policies)
- [Memory Pool](#memory-pool)
- [Profiler](#profiler)
- [Event System](#event-system)
- [Retry Policies](#retry-policies)

---

## Scheduler

任务调度的主入口。

```cpp
#include <hts/scheduler.hpp>
```

### 构造

```cpp
// 默认配置
Scheduler scheduler;

// 自定义配置
SchedulerConfig config;
config.memory_pool_size = 512 * 1024 * 1024;  // 512 MB
config.cpu_thread_count = 8;
config.gpu_stream_count = 4;

Scheduler scheduler(config);
```

### 配置结构

```cpp
struct SchedulerConfig {
    // 内存设置
    size_t memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    bool allow_memory_growth = true;
    
    // 并发设置
    size_t cpu_thread_count = std::thread::hardware_concurrency();
    size_t gpu_stream_count = 4;
    
    // 重试设置
    size_t max_retry_count = 3;
    std::chrono::milliseconds retry_delay{100};
};
```

### 核心方法

| 方法 | 签名 | 描述 |
|------|------|------|
| `graph()` | `TaskGraph&` | 访问任务图 |
| `execute()` | `void` | 执行图中的所有任务 |
| `reset()` | `void` | 清空任务图 |
| `set_policy()` | `void(std::unique_ptr<SchedulingPolicy>)` | 设置调度策略 |
| `set_profiling()` | `void(bool)` | 启用/禁用性能分析 |
| `profiler()` | `Profiler&` | 访问分析器数据 |
| `set_error_callback()` | `void(ErrorCallback)` | 设置错误处理器 |

### 示例

```cpp
Scheduler scheduler;

// 构建任务图
auto task1 = scheduler.graph().add_task(DeviceType::CPU);
auto task2 = scheduler.graph().add_task(DeviceType::GPU);
scheduler.graph().add_dependency(task1->id(), task2->id());

// 配置
scheduler.set_profiling(true);
scheduler.set_error_callback([](TaskId id, const std::string& msg) {
    std::cerr << "任务 " << id << " 失败: " << msg << "\n";
});

// 执行
scheduler.execute();

// 获取分析数据
auto summary = scheduler.profiler().generate_summary();
```

---

## TaskGraph

任务及其依赖的容器。

```cpp
#include <hts/task_graph.hpp>
```

### 核心方法

| 方法 | 签名 | 描述 |
|------|------|------|
| `add_task()` | `TaskPtr(DeviceType)` | 添加新任务 |
| `add_dependency()` | `void(TaskId, TaskId)` | 添加边：from → to |
| `remove_dependency()` | `void(TaskId, TaskId)` | 移除边 |
| `get_task()` | `TaskPtr(TaskId)` | 按 ID 获取任务 |
| `tasks()` | `const std::vector<TaskPtr>&` | 所有任务 |
| `clear()` | `void` | 移除所有任务 |

### 依赖

```cpp
// 创建依赖：task1 → task2（task2 依赖于 task1）
scheduler.graph().add_dependency(task1->id(), task2->id());

// 多个依赖
scheduler.graph().add_dependency(parent->id(), child1->id());
scheduler.graph().add_dependency(parent->id(), child2->id());

// 链式
scheduler.graph().add_dependency(a->id(), b->id());
scheduler.graph().add_dependency(b->id(), c->id());
```

---

## Task

工作单元。

```cpp
#include <hts/task.hpp>
```

### 属性

| 属性 | 类型 | 描述 |
|------|------|------|
| `id()` | `TaskId` | 唯一标识符 |
| `name()` | `const std::string&` | 任务名称 |
| `state()` | `TaskState` | 当前状态 |
| `device_type()` | `DeviceType` | 优先设备 |
| `priority()` | `TaskPriority` | 执行优先级 |

### 设置函数

```cpp
// CPU 函数
task->set_cpu_function([](TaskContext& ctx) {
    // CPU 工作
    ctx.set_output("key", value);
});

// GPU 函数
task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // GPU 工作
    my_kernel<<<grid, block, 0, stream>>>(...);
});

// 两者都设置（调度器根据负载选择）
task->set_cpu_function(...);
task->set_gpu_function(...);
```

### 配置

```cpp
// 设置属性
task->set_name("MyTask");
task->set_priority(TaskPriority::High);
task->set_retry_policy(RetryPolicyFactory::exponential(3));
```

### DeviceType 枚举

```cpp
enum class DeviceType {
    CPU,    // 仅 CPU 执行
    GPU,    // 仅 GPU 执行
    Any     // 调度器根据负载决定
};
```

### TaskPriority 枚举

```cpp
enum class TaskPriority {
    Low = 0,
    Normal = 1,
    High = 2,
    Critical = 3
};
```

---

## TaskBuilder

任务构建的流式 API。

```cpp
#include <hts/task_builder.hpp>
```

### 方法链

```cpp
TaskBuilder builder(scheduler.graph());

auto task = builder
    .name("ProcessData")
    .device(DeviceType::GPU)
    .priority(TaskPriority::High)
    .after(parent_task)           // 添加依赖
    .cpu([](TaskContext& ctx) {   // CPU 实现
        // CPU 后备
    })
    .gpu([](TaskContext& ctx, cudaStream_t stream) {
        // GPU 实现
        // 启动内核
    })
    .retry(RetryPolicyFactory::fixed(3))
    .build();
```

### 可用方法

| 方法 | 描述 |
|------|------|
| `name(string)` | 设置任务名称 |
| `device(DeviceType)` | 设置目标设备 |
| `priority(TaskPriority)` | 设置优先级 |
| `after(TaskPtr)` | 添加依赖 |
| `after_all(vector<TaskPtr>)` | 添加多个依赖 |
| `cpu(function)` | 设置 CPU 函数 |
| `gpu(function)` | 设置 GPU 函数 |
| `retry(policy)` | 设置重试策略 |
| `build()` | 创建任务 |

---

## TaskContext

任务函数的执行上下文。

```cpp
#include <hts/task_context.hpp>
```

### 数据 I/O

```cpp
// 为下游任务设置输出
ctx.set_output("result", 42);
ctx.set_output("data", buffer, size);

// 从上游任务获取输入
auto value = ctx.get_input<int>("result");
auto data = ctx.get_input<void*>("data");
auto size = ctx.get_input_size("data");
```

### 内存管理

```cpp
// GPU 内存分配
void* gpu_mem = ctx.allocate_gpu_memory(size);

// 任务完成时自动释放
//（除非显式保留）
```

### 错误处理

```cpp
// 报告错误
ctx.set_error("出错了");

// 检查是否有错误
if (ctx.has_error()) {
    // 处理错误
}
```

### 状态查询

```cpp
TaskId id = ctx.task_id();
Task& task = ctx.get_task();
```

---

## TaskGroup

相关任务的批量操作。

```cpp
#include <hts/task_group.hpp>
```

### 构造

```cpp
TaskGroup workers("DataWorkers", scheduler.graph());

// 添加多个任务
for (int i = 0; i < 4; ++i) {
    auto task = workers.add_task(DeviceType::CPU);
    task->set_cpu_function([i](TaskContext& ctx) {
        process_chunk(i);
    });
}
```

### 依赖

```cpp
// 组内所有任务依赖于外部任务
workers.depends_on(init_task);

// 外部任务依赖于组内所有任务完成
workers.then(cleanup_task);

// 为所有任务设置优先级
workers.set_priority(TaskPriority::High);
```

---

## Scheduling Policies

```cpp
#include <hts/scheduling_policy.hpp>
```

### 内置策略

```cpp
// 基于负载的选择（默认）
scheduler.set_policy(std::make_unique<DefaultSchedulingPolicy>());

// 优先 GPU
scheduler.set_policy(std::make_unique<GpuFirstPolicy>());

// 优先 CPU
scheduler.set_policy(std::make_unique<CpuFirstPolicy>());

// CPU/GPU 交替
scheduler.set_policy(std::make_unique<RoundRobinPolicy>());

// 基于优先级（高优先级优先）
scheduler.set_policy(std::make_unique<ShortestJobFirstPolicy>());
```

### 自定义策略

```cpp
class MyPolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task, 
                             const SystemStatus& status) override {
        // 自定义逻辑
        if (task.priority() == TaskPriority::Critical) {
            return DeviceType::GPU;  // 关键任务走 GPU 快速通道
        }
        return DeviceType::CPU;
    }
    
    std::string name() const override {
        return "MyCustomPolicy";
    }
};

scheduler.set_policy(std::make_unique<MyPolicy>());
```

---

## Memory Pool

```cpp
#include <hts/memory_pool.hpp>
```

### 直接访问（高级）

```cpp
auto& pool = scheduler.memory_pool();

// 手动分配
void* mem = pool.allocate(1024);

// 手动释放
pool.deallocate(mem, 1024);

// 获取统计
auto stats = pool.get_stats();
std::cout << "已用: " << stats.used_bytes << "\n";
std::cout << "空闲: " << stats.free_bytes << "\n";
```

---

## Profiler

```cpp
#include <hts/profiler.hpp>
```

### 使用

```cpp
// 启用性能分析
scheduler.set_profiling(true);
scheduler.execute();

// 获取分析器
Profiler& prof = scheduler.profiler();

// 摘要
auto summary = prof.generate_summary();
std::cout << "总时间: " << summary.total_time.count() / 1e6 << " ms\n";
std::cout << "并行度: " << summary.parallelism << "x\n";
std::cout << "任务数: " << summary.task_count << "\n";

// 详细报告
std::cout << prof.generate_report();

// 导出时间线（Chrome 追踪格式）
prof.export_timeline("timeline.json");
```

### 摘要结构

```cpp
struct ProfileSummary {
    std::chrono::nanoseconds total_time;
    std::chrono::nanoseconds cpu_time;
    std::chrono::nanoseconds gpu_time;
    double parallelism;  // 理想时间 / 实际时间
    size_t task_count;
    size_t cpu_task_count;
    size_t gpu_task_count;
};
```

---

## Event System

```cpp
#include <hts/event_system.hpp>
```

### 使用

```cpp
EventSystem events;

// 订阅特定事件类型
auto sub1 = events.subscribe(EventType::TaskCompleted, [](const Event& e) {
    std::cout << "任务 " << e.task_id << " 完成\n";
});

// 订阅所有事件
auto sub2 = events.subscribe_all([](const Event& e) {
    std::cout << EventSystem::event_type_name(e.type) << "\n";
});

// 析构时自动取消订阅（RAII）
// 或手动：events.unsubscribe(sub1);
```

### 事件类型

```cpp
enum class EventType {
    TaskCreated, TaskStarted, TaskCompleted, TaskFailed,
    TaskCancelled, TaskRetrying,
    GraphStarted, GraphCompleted,
    MemoryAllocated, MemoryFreed,
    StreamAcquired, StreamReleased
};
```

---

## Retry Policies

```cpp
#include <hts/retry_policy.hpp>
```

### 工厂方法

```cpp
// 不重试（默认）
auto no_retry = RetryPolicyFactory::no_retry();

// 固定延迟
auto fixed = RetryPolicyFactory::fixed(
    3,                              // 最多 3 次尝试
    std::chrono::milliseconds{100}  // 100ms 延迟
);

// 指数退避
auto exp_backoff = RetryPolicyFactory::exponential(5);  // 5 次尝试

// 抖动退避（添加随机性）
auto jittered = RetryPolicyFactory::jittered(5);
```

### 条件重试

```cpp
// 仅重试瞬时错误
auto conditional = ConditionalRetryPolicy::transient_errors(
    RetryPolicyFactory::exponential(5)
);

task->set_retry_policy(std::move(conditional));
```

---

## 类型别名

```cpp
using TaskId = uint64_t;
using TaskPtr = std::shared_ptr<Task>;
```

---

## 头文件汇总

| 头文件 | 内容 |
|--------|------|
| `heterogeneous_task_scheduler.hpp` | 所有头文件组合 |
| `scheduler.hpp` | Scheduler 类 |
| `task_graph.hpp` | TaskGraph 类 |
| `task.hpp` | Task 类 |
| `task_builder.hpp` | TaskBuilder 类 |
| `task_context.hpp` | TaskContext 类 |
| `task_group.hpp` | TaskGroup 类 |
| `scheduling_policy.hpp` | 策略类 |
| `memory_pool.hpp` | MemoryPool 类 |
| `profiler.hpp` | Profiler 类 |
| `event_system.hpp` | EventSystem 类 |
| `retry_policy.hpp` | 重试策略 |
| `types.hpp` | 通用类型和枚举 |
