# Design Document: Heterogeneous Task Scheduler

[![Spec](https://img.shields.io/badge/Spec-Design-blue)]()
[![Version](https://img.shields.io/badge/Version-1.1.0-green)]()

> 本设计文档描述异构任务图调度器的架构和实现细节。

## Overview

HTS 采用分层架构，将任务图管理、依赖跟踪、内存管理和执行引擎解耦，实现高内聚低耦合。

### Design Principles

| Principle | Description |
|-----------|-------------|
| **零拷贝内存管理** | 通过内存池避免频繁 GPU 内存分配 |
| **异步优先** | 默认异步执行，最大化 CPU/GPU 并行度 |
| **类型安全** | C++ 模板 + RAII 确保资源安全 |
| **可扩展性** | 支持多 GPU 和自定义调度策略 |

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

任务是调度的基本单元，封装计算逻辑和元数据。

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

任务执行上下文，提供内存分配和数据访问接口。

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

任务图管理器，负责构建和验证 DAG。

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

依赖管理器，跟踪任务完成状态。

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

GPU 内存池，采用伙伴系统算法。

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

核心调度器，协调所有组件。

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
| 1 | Task Creation Invariants | Req 1.1, 1.4, 5.1 |
| 2 | Cycle Detection | Req 1.3, 1.5 |
| 3 | Dependency Recording Round Trip | Req 1.2 |
| 4 | Dependency Execution Order | Req 2.1-2.3 |
| 5 | Failure Propagation | Req 2.5 |
| 6 | Memory Pool Round Trip | Req 3.1-3.3 |
| 7 | Memory Pool Growth | Req 3.4 |
| 8 | Concurrent Execution | Req 4.2-4.4 |
| 9 | Load Balancing | Req 5.2-5.3 |
| 10 | Error Propagation | Req 6.1-6.4 |
| 11 | Graceful Shutdown | Req 6.5 |
| 12 | Statistics Accuracy | Req 7.1-7.3 |
| 13 | Timeline Generation | Req 7.4 |
| 14 | Execution Mode Correctness | Req 8.3-8.5 |

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

- [Requirements](requirements.md)
- [Tasks](tasks.md)
