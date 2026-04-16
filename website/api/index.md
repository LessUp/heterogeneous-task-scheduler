# API Reference

Complete API documentation for HTS.

## Core Classes

### Scheduler

The main entry point for task graph execution.

```cpp
class Scheduler {
public:
    Scheduler(const SchedulerConfig& config = {});
    TaskGraph& graph();
    void execute();
    void set_profiling(bool enabled);
    Profiler& profiler();
};
```

### TaskGraph

Manages tasks and their dependencies.

```cpp
class TaskGraph {
public:
    TaskPtr add_task(DeviceType device = DeviceType::Any);
    void add_dependency(TaskId from, TaskId to);
    void clear();
};
```

## Type Definitions

```cpp
using TaskId = uint64_t;
using TaskPtr = std::shared_ptr<Task>;

enum class DeviceType { CPU, GPU, Any };
enum class TaskPriority { Low, Normal, High, Critical };
```
