# API Overview

The authoritative API is the public header set under `include/hts/`. This page keeps a small map of the
stable surface instead of duplicating every signature in a second, drift-prone format.

## Primary headers

| Header | Main types | Common entry points |
|--------|------------|---------------------|
| `hts/heterogeneous_task_scheduler.hpp` | Umbrella include | Pulls in the public library surface |
| `hts/scheduler.hpp` | `Scheduler` | `graph()`, `execute()`, `execute_async()`, `set_policy()`, `set_error_callback()`, `get_stats()`, `generate_timeline_json()` |
| `hts/task_graph.hpp` | `TaskGraph` | `add_task()`, `add_dependency()`, `validate()`, `topological_sort()`, `get_task()`, `clear()` |
| `hts/task_builder.hpp` | `TaskBuilder` | `name()`, `device()`, `priority()`, `cpu()`, `gpu()`, `after()`, `build()` |
| `hts/task.hpp` | `Task` | `set_name()`, `set_priority()`, `set_cpu_function()`, `set_gpu_function()` |
| `hts/task_context.hpp` | `TaskContext` | `allocate_gpu_memory()`, `free_gpu_memory()`, `set_output()`, `get_input()`, `report_error()` |

## Scheduling policies

`include/hts/scheduling_policy.hpp` provides the built-in policies currently implemented by the runtime:

- `DefaultSchedulingPolicy`
- `GpuFirstPolicy`
- `CpuFirstPolicy`
- `RoundRobinPolicy`
- `ShortestJobFirstPolicy`

## Runtime utilities

Additional public headers expose focused helpers that are tested independently:

- `hts/profiler.hpp`
- `hts/graph_serializer.hpp`
- `hts/retry_policy.hpp`
- `hts/event_system.hpp`
- `hts/resource_limiter.hpp`
- `hts/task_barrier.hpp`
- `hts/task_future.hpp`
- `hts/task_group.hpp`

## Minimal usage sketch

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

## When in doubt

Read the headers first. They are small, tested, and more reliable than a long prose API mirror.
