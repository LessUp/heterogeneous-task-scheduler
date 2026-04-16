# API Reference

> Complete API documentation for HTS

---

## Table of Contents

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

The main entry point for task scheduling.

```cpp
#include <hts/scheduler.hpp>
```

### Construction

```cpp
// Default configuration
Scheduler scheduler;

// Custom configuration
SchedulerConfig config;
config.memory_pool_size = 512 * 1024 * 1024;  // 512 MB
config.cpu_thread_count = 8;
config.gpu_stream_count = 4;

Scheduler scheduler(config);
```

### Configuration Structure

```cpp
struct SchedulerConfig {
    // Memory settings
    size_t memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    bool allow_memory_growth = true;
    
    // Concurrency settings
    size_t cpu_thread_count = std::thread::hardware_concurrency();
    size_t gpu_stream_count = 4;
    
    // Retry settings
    size_t max_retry_count = 3;
    std::chrono::milliseconds retry_delay{100};
};
```

### Core Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `graph()` | `TaskGraph&` | Access the task graph |
| `execute()` | `void` | Execute all tasks in the graph |
| `reset()` | `void` | Clear the task graph |
| `set_policy()` | `void(std::unique_ptr<SchedulingPolicy>)` | Set scheduling policy |
| `set_profiling()` | `void(bool)` | Enable/disable profiling |
| `profiler()` | `Profiler&` | Access profiler data |
| `set_error_callback()` | `void(ErrorCallback)` | Set error handler |

### Example

```cpp
Scheduler scheduler;

// Build task graph
auto task1 = scheduler.graph().add_task(DeviceType::CPU);
auto task2 = scheduler.graph().add_task(DeviceType::GPU);
scheduler.graph().add_dependency(task1->id(), task2->id());

// Configure
scheduler.set_profiling(true);
scheduler.set_error_callback([](TaskId id, const std::string& msg) {
    std::cerr << "Task " << id << " failed: " << msg << "\n";
});

// Execute
scheduler.execute();

// Get profiling data
auto summary = scheduler.profiler().generate_summary();
```

---

## TaskGraph

Container for tasks and their dependencies.

```cpp
#include <hts/task_graph.hpp>
```

### Core Methods

| Method | Signature | Description |
|--------|-----------|-------------|
| `add_task()` | `TaskPtr(DeviceType)` | Add a new task |
| `add_dependency()` | `void(TaskId, TaskId)` | Add edge: from → to |
| `remove_dependency()` | `void(TaskId, TaskId)` | Remove edge |
| `get_task()` | `TaskPtr(TaskId)` | Get task by ID |
| `tasks()` | `const std::vector<TaskPtr>&` | All tasks |
| `clear()` | `void` | Remove all tasks |

### Dependencies

```cpp
// Create dependency: task1 → task2 (task2 depends on task1)
scheduler.graph().add_dependency(task1->id(), task2->id());

// Multiple dependencies
scheduler.graph().add_dependency(parent->id(), child1->id());
scheduler.graph().add_dependency(parent->id(), child2->id());

// Chain
scheduler.graph().add_dependency(a->id(), b->id());
scheduler.graph().add_dependency(b->id(), c->id());
```

---

## Task

Individual unit of work.

```cpp
#include <hts/task.hpp>
```

### Properties

| Property | Type | Description |
|----------|------|-------------|
| `id()` | `TaskId` | Unique identifier |
| `name()` | `const std::string&` | Task name |
| `state()` | `TaskState` | Current state |
| `device_type()` | `DeviceType` | Preferred device |
| `priority()` | `TaskPriority` | Execution priority |

### Setting Functions

```cpp
// CPU function
task->set_cpu_function([](TaskContext& ctx) {
    // CPU work
    ctx.set_output("key", value);
});

// GPU function
task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // GPU work
    my_kernel<<<grid, block, 0, stream>>>(...);
});

// Both (scheduler chooses based on load)
task->set_cpu_function(...);
task->set_gpu_function(...);
```

### Configuration

```cpp
// Set properties
task->set_name("MyTask");
task->set_priority(TaskPriority::High);
task->set_retry_policy(RetryPolicyFactory::exponential(3));
```

### DeviceType Enum

```cpp
enum class DeviceType {
    CPU,    // CPU execution only
    GPU,    // GPU execution only
    Any     // Scheduler decides based on load
};
```

### TaskPriority Enum

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

Fluent API for task construction.

```cpp
#include <hts/task_builder.hpp>
```

### Method Chain

```cpp
TaskBuilder builder(scheduler.graph());

auto task = builder
    .name("ProcessData")
    .device(DeviceType::GPU)
    .priority(TaskPriority::High)
    .after(parent_task)           // Add dependency
    .cpu([](TaskContext& ctx) {   // CPU implementation
        // CPU fallback
    })
    .gpu([](TaskContext& ctx, cudaStream_t stream) {
        // GPU implementation
        // Launch kernels here
    })
    .retry(RetryPolicyFactory::fixed(3))
    .build();
```

### Available Methods

| Method | Description |
|--------|-------------|
| `name(string)` | Set task name |
| `device(DeviceType)` | Set target device |
| `priority(TaskPriority)` | Set priority |
| `after(TaskPtr)` | Add dependency |
| `after_all(vector<TaskPtr>)` | Add multiple dependencies |
| `cpu(function)` | Set CPU function |
| `gpu(function)` | Set GPU function |
| `retry(policy)` | Set retry policy |
| `build()` | Create task |

---

## TaskContext

Execution context for task functions.

```cpp
#include <hts/task_context.hpp>
```

### Data I/O

```cpp
// Set output for downstream tasks
ctx.set_output("result", 42);
ctx.set_output("data", buffer, size);

// Get input from upstream tasks
auto value = ctx.get_input<int>("result");
auto data = ctx.get_input<void*>("data");
auto size = ctx.get_input_size("data");
```

### Memory Management

```cpp
// GPU memory allocation
void* gpu_mem = ctx.allocate_gpu_memory(size);

// The memory is automatically freed when task completes
// (unless explicitly retained)
```

### Error Handling

```cpp
// Report error
ctx.set_error("Something went wrong");

// Check if error occurred
if (ctx.has_error()) {
    // Handle error
}
```

### State Queries

```cpp
TaskId id = ctx.task_id();
Task& task = ctx.get_task();
```

---

## TaskGroup

Batch operations for related tasks.

```cpp
#include <hts/task_group.hpp>
```

### Construction

```cpp
TaskGroup workers("DataWorkers", scheduler.graph());

// Add multiple tasks
for (int i = 0; i < 4; ++i) {
    auto task = workers.add_task(DeviceType::CPU);
    task->set_cpu_function([i](TaskContext& ctx) {
        process_chunk(i);
    });
}
```

### Dependencies

```cpp
// All tasks in group depend on external task
workers.depends_on(init_task);

// External task depends on all group tasks completing
workers.then(cleanup_task);

// Set priority for all tasks
workers.set_priority(TaskPriority::High);
```

---

## Scheduling Policies

```cpp
#include <hts/scheduling_policy.hpp>
```

### Built-in Policies

```cpp
// Load-based selection (default)
scheduler.set_policy(std::make_unique<DefaultSchedulingPolicy>());

// Prefer GPU
scheduler.set_policy(std::make_unique<GpuFirstPolicy>());

// Prefer CPU
scheduler.set_policy(std::make_unique<CpuFirstPolicy>());

// Alternate between CPU/GPU
scheduler.set_policy(std::make_unique<RoundRobinPolicy>());

// Priority-based (higher priority first)
scheduler.set_policy(std::make_unique<ShortestJobFirstPolicy>());
```

### Custom Policy

```cpp
class MyPolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task, 
                             const SystemStatus& status) override {
        // Custom logic
        if (task.priority() == TaskPriority::Critical) {
            return DeviceType::GPU;  // Fast path for critical tasks
        }
        return DeviceType::CPU;
    }
};

scheduler.set_policy(std::make_unique<MyPolicy>());
```

---

## Memory Pool

```cpp
#include <hts/memory_pool.hpp>
```

### Direct Access (Advanced)

```cpp
auto& pool = scheduler.memory_pool();

// Manual allocation
void* mem = pool.allocate(1024);

// Manual deallocation
pool.deallocate(mem, 1024);

// Get statistics
auto stats = pool.get_stats();
std::cout << "Used: " << stats.used_bytes << "\n";
std::cout << "Free: " << stats.free_bytes << "\n";
```

---

## Profiler

```cpp
#include <hts/profiler.hpp>
```

### Usage

```cpp
// Enable profiling
scheduler.set_profiling(true);
scheduler.execute();

// Get profiler
Profiler& prof = scheduler.profiler();

// Summary
auto summary = prof.generate_summary();
std::cout << "Total time: " << summary.total_time.count() / 1e6 << " ms\n";
std::cout << "Parallelism: " << summary.parallelism << "x\n";
std::cout << "Tasks: " << summary.task_count << "\n";

// Detailed report
std::cout << prof.generate_report();

// Export timeline (Chrome tracing format)
prof.export_timeline("timeline.json");
```

### Summary Structure

```cpp
struct ProfileSummary {
    std::chrono::nanoseconds total_time;
    std::chrono::nanoseconds cpu_time;
    std::chrono::nanoseconds gpu_time;
    double parallelism;  // Ideal time / actual time
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

### Usage

```cpp
EventSystem events;

// Subscribe to specific event type
auto sub1 = events.subscribe(EventType::TaskCompleted, [](const Event& e) {
    std::cout << "Task " << e.task_id << " completed\n";
});

// Subscribe to all events
auto sub2 = events.subscribe_all([](const Event& e) {
    std::cout << EventSystem::event_type_name(e.type) << "\n";
});

// Unsubscribe on destruction (RAII)
// Or manually: events.unsubscribe(sub1);
```

### Event Types

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

### Factory Methods

```cpp
// No retry (default)
auto no_retry = RetryPolicyFactory::no_retry();

// Fixed delay
auto fixed = RetryPolicyFactory::fixed(
    3,                              // max 3 attempts
    std::chrono::milliseconds{100}  // 100ms delay
);

// Exponential backoff
auto exp_backoff = RetryPolicyFactory::exponential(5);  // 5 attempts

// Jittered backoff (adds randomness)
auto jittered = RetryPolicyFactory::jittered(5);
```

### Conditional Retry

```cpp
// Only retry transient errors
auto conditional = ConditionalRetryPolicy::transient_errors(
    RetryPolicyFactory::exponential(5)
);

task->set_retry_policy(std::move(conditional));
```

---

## Type Aliases

```cpp
using TaskId = uint64_t;
using TaskPtr = std::shared_ptr<Task>;
```

---

## Header Summary

| Header | Contents |
|--------|----------|
| `heterogeneous_task_scheduler.hpp` | All headers combined |
| `scheduler.hpp` | Scheduler class |
| `task_graph.hpp` | TaskGraph class |
| `task.hpp` | Task class |
| `task_builder.hpp` | TaskBuilder class |
| `task_context.hpp` | TaskContext class |
| `task_group.hpp` | TaskGroup class |
| `scheduling_policy.hpp` | Policy classes |
| `memory_pool.hpp` | MemoryPool class |
| `profiler.hpp` | Profiler class |
| `event_system.hpp` | EventSystem class |
| `retry_policy.hpp` | Retry policies |
| `types.hpp` | Common types and enums |
