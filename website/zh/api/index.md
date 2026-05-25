# API 概览

权威 API 以 `include/hts/` 下的公共头文件为准。这里不再维护一套冗长且容易漂移的“第二份接口文档”，只给出稳定表面地图。

## 主要头文件

| 头文件 | 核心类型 | 常用入口 |
|--------|----------|----------|
| `hts/heterogeneous_task_scheduler.hpp` | 聚合头 | 一次性引入公共库表面 |
| `hts/scheduler.hpp` | `Scheduler` | `graph()`、`execute()`、`execute_async()`、`set_policy()`、`set_error_callback()`、`get_stats()`、`generate_timeline_json()` |
| `hts/task_graph.hpp` | `TaskGraph` | `add_task()`、`add_dependency()`、`validate()`、`topological_sort()`、`get_task()`、`clear()` |
| `hts/task_builder.hpp` | `TaskBuilder` | `name()`、`device()`、`priority()`、`cpu()`、`gpu()`、`after()`、`build()` |
| `hts/task.hpp` | `Task` | `set_name()`、`set_priority()`、`set_cpu_function()`、`set_gpu_function()` |
| `hts/task_context.hpp` | `TaskContext` | `allocate_gpu_memory()`、`free_gpu_memory()`、`set_output()`、`get_input()`、`report_error()` |

## 调度策略

`include/hts/scheduling_policy.hpp` 当前提供以下内建策略：

- `DefaultSchedulingPolicy`
- `GpuFirstPolicy`
- `CpuFirstPolicy`
- `RoundRobinPolicy`
- `ShortestJobFirstPolicy`

## 运行时工具

以下公共头文件提供独立测试的辅助能力：

- `hts/profiler.hpp`
- `hts/graph_serializer.hpp`
- `hts/retry_policy.hpp`
- `hts/event_system.hpp`
- `hts/resource_limiter.hpp`
- `hts/task_barrier.hpp`
- `hts/task_future.hpp`
- `hts/task_group.hpp`

## 最小使用示意

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    hts::Scheduler scheduler;
    auto task = scheduler.graph().add_task(hts::DeviceType::CPU);
    task->set_name("hello");
    task->set_cpu_function([](hts::TaskContext &) {});

    scheduler.execute();
    return 0;
}
```

## 遇到不确定时

优先直接读头文件。它们体量小、测试覆盖足够，而且比另一份手写 API 镜像更可靠。
