# RFC-001: Core Architecture

[![Spec](https://img.shields.io/badge/Spec-RFC-blue)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

> Architecture design document for the Heterogeneous Task Scheduler core framework.

---

## Overview

HTS adopts a layered architecture that decouples task graph management, dependency tracking, memory management, and execution engine, achieving high cohesion and low coupling.

### Design Principles

| Principle | Description |
|-----------|-------------|
| **Zero-Copy Memory Management** | Avoid frequent GPU memory allocations via memory pool |
| **Async-First** | Default async execution for maximum CPU/GPU parallelism |
| **Type Safety** | C++ templates + RAII for resource safety |
| **Extensibility** | Support multi-GPU and custom scheduling policies |

---

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                          │
├─────────────────────────────────────────────────────────────────┤
│                      TaskGraph Builder API                       │
│    TaskBuilder │ TaskGroup │ TaskBarrier │ TaskFuture            │
├─────────────────────────────────────────────────────────────────┤
│                          Scheduler                               │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ Dependency  │  │  Scheduling │  │  Profiler   │              │
│  │  Manager    │  │   Policy    │  │  & Logger   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                      Execution Engine                            │
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐              │
│  │ CPU Thread  │  │ GPU Stream  │  │  Resource   │              │
│  │   Pool      │  │  Manager    │  │   Limiter   │              │
│  └─────────────┘  └─────────────┘  └─────────────┘              │
├─────────────────────────────────────────────────────────────────┤
│                       Memory Pool                                │
│            Buddy System Allocator (GPU Memory)                   │
├─────────────────────────────────────────────────────────────────┤
│                    CUDA Runtime / CPU Threads                    │
└─────────────────────────────────────────────────────────────────┘
```

---

## Components

### 1. Task

Task is the basic unit of scheduling, encapsulating computation logic and metadata.

```cpp
enum class DeviceType { CPU, GPU, Any };
enum class TaskState { Pending, Ready, Running, Completed, Failed, Blocked, Cancelled };

class Task {
public:
    using TaskId = uint64_t;
    using CpuFunction = std::function<void(TaskContext&)>;
    using GpuFunction = std::function<void(TaskContext&, cudaStream_t)>;

    Task(TaskId id, DeviceType preferred_device);

    void set_cpu_function(CpuFunction func);
    void set_gpu_function(GpuFunction func);

    TaskId id() const;
    TaskState state() const;
    DeviceType preferred_device() const;
    std::chrono::nanoseconds execution_time() const;
};
```

### 2. TaskContext

Task execution context providing memory allocation and data access interfaces.

```cpp
class TaskContext {
public:
    void* allocate_gpu_memory(size_t bytes);
    void free_gpu_memory(void* ptr);

    template<typename T> T* get_input(const std::string& name);
    template<typename T> void set_output(const std::string& name, T* data, size_t size);

    void report_error(const std::string& message);
    bool has_error() const;
};
```

### 3. TaskGraph

Task graph manager responsible for building and validating DAG.

```cpp
class TaskGraph {
public:
    using TaskPtr = std::shared_ptr<Task>;

    TaskPtr add_task(DeviceType device = DeviceType::Any);
    bool add_dependency(TaskId from, TaskId to);  // Returns false if cycle

    bool validate() const;
    std::vector<TaskPtr> topological_sort() const;
    std::vector<TaskPtr> get_root_tasks() const;
    std::vector<TaskPtr> get_successors(TaskId id) const;
    std::vector<TaskPtr> get_predecessors(TaskId id) const;
};
```

### 4. DependencyManager

Dependency manager tracking task completion status and notifications.

```cpp
class DependencyManager {
public:
    explicit DependencyManager(const TaskGraph& graph);

    void mark_completed(TaskId id);
    void mark_failed(TaskId id);

    bool is_ready(TaskId id) const;
    std::vector<TaskId> get_ready_tasks() const;
    size_t pending_dependency_count(TaskId id) const;
};
```

### 5. MemoryPool

GPU memory pool using buddy system algorithm.

```cpp
struct MemoryStats {
    size_t total_bytes;
    size_t used_bytes;
    size_t peak_bytes;
    size_t allocation_count;
    double fragmentation_ratio;
};

class MemoryPool {
public:
    explicit MemoryPool(size_t initial_size_bytes, bool allow_growth = true);

    void* allocate(size_t bytes);
    void free(void* ptr);
    MemoryStats get_stats() const;
    void reset();
};
```

### 6. Scheduler

Core scheduler coordinating all components.

```cpp
struct SchedulerConfig {
    size_t memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    size_t cpu_thread_count = 4;
    size_t gpu_stream_count = 4;
    bool allow_memory_growth = true;
};

class Scheduler {
public:
    explicit Scheduler(const SchedulerConfig& config = {});

    TaskGraph& graph();
    void execute();
    std::future<void> execute_async();

    void set_error_callback(ErrorCallback callback);
    ExecutionStats get_stats() const;
    std::string generate_timeline_json() const;
};
```

---

## Correctness Properties

| # | Property | Validates |
|---|----------|-----------|
| 1 | Task Creation Invariants | REQ-1.1, REQ-1.4, REQ-5.1 |
| 2 | Cycle Detection | REQ-1.3, REQ-1.5 |
| 3 | Dependency Recording Round Trip | REQ-1.2 |
| 4 | Dependency Execution Order | REQ-2.1-2.3 |
| 5 | Failure Propagation | REQ-2.5 |
| 6 | Memory Pool Round Trip | REQ-3.1-3.3 |
| 7 | Memory Pool Growth | REQ-3.4 |
| 8 | Concurrent Execution | REQ-4.2-4.4 |
| 9 | Load Balancing | REQ-5.2-5.3 |
| 10 | Error Propagation | REQ-6.1-6.4 |
| 11 | Graceful Shutdown | REQ-6.5 |
| 12 | Statistics Accuracy | REQ-7.1-7.3 |
| 13 | Timeline Generation | REQ-7.4 |
| 14 | Execution Mode Correctness | REQ-8.3-8.5 |

---

## Error Handling

| Category | Handling |
|----------|----------|
| **Graph Construction** | Cycle: return false; Invalid ID: throw |
| **Memory** | Allocation failure: return nullptr or throw |
| **CUDA** | Capture error, wrap in TaskError, propagate |
| **Execution** | Catch exceptions, mark task as Failed |

---

## Testing Strategy

### Unit Tests
- Task creation, graph construction, memory pool operations
- Framework: Google Test

### Property-Based Tests
- Verify universal properties across random inputs
- Framework: RapidCheck
- Minimum 100 iterations per property

### Integration Tests
- End-to-end execution, CPU-GPU concurrency, error propagation

---

## Related Documents

- [Product Requirements](../product/001-heterogeneous-task-scheduler.md)
- [Implementation Tasks](../product/tasks-core.md)
