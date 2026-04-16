# Architecture Overview

> Understanding the internal design of HTS

---

## Table of Contents

- [System Overview](#system-overview)
- [Component Architecture](#component-architecture)
- [Data Flow](#data-flow)
- [Execution Model](#execution-model)
- [Threading Model](#threading-model)
- [Design Decisions](#design-decisions)

---

## System Overview

HTS follows a layered architecture that separates policy from mechanism:

```
┌─────────────────────────────────────────────────────────────────┐
│                      User Application Layer                      │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐   │
│  │ TaskBuilder  │  │  TaskGroup   │  │    TaskBarrier       │   │
│  │ (Fluent API) │  │ (Batch Ops)  │  │ (Synchronization)    │   │
│  └──────────────┘  └──────────────┘  └──────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│                      Scheduling Layer                            │
│  ┌─────────────────┐  ┌───────────────────────────────────────┐  │
│  │  TaskGraph      │  │         Scheduler                     │  │
│  │  (DAG Storage)  │→ │  ┌───────────┐ ┌───────────────────┐  │  │
│  │                 │  │  │  Policy   │ │ Dependency Manager│  │  │
│  └─────────────────┘  │  └───────────┘ └───────────────────┘  │  │
│                       └───────────────────────────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                      Execution Layer                             │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐   │
│  │  CPU Thread     │  │  GPU Stream     │  │   Resource      │   │
│  │     Pool        │  │    Manager      │  │   Limiter       │   │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│                      Memory Management Layer                     │
│  ┌───────────────────────────────────────────────────────────┐   │
│  │              Buddy System Memory Pool                     │   │
│  │         (Efficient GPU memory allocation)                 │   │
│  └───────────────────────────────────────────────────────────┘   │
├─────────────────────────────────────────────────────────────────┤
│                      Observability Layer                         │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────────────┐   │
│  │   Profiler   │  │    Logger    │  │   Event System       │   │
│  └──────────────┘  └──────────────┘  └──────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

---

## Component Architecture

### 1. TaskGraph & DAG Management

```cpp
class TaskGraph {
    // Stores tasks and their dependencies
    std::unordered_map<TaskId, TaskPtr> tasks_;
    std::unordered_map<TaskId, std::vector<TaskId>> dependencies_;
    std::unordered_map<TaskId, std::vector<TaskId>> dependents_;
    
public:
    TaskPtr add_task(DeviceType device);
    void add_dependency(TaskId from, TaskId to);
    std::vector<TaskId> topological_sort() const;
    bool has_cycle() const;
};
```

**Key Features:**
- Cycle detection before execution
- Topological sorting for execution order
- Automatic dependency tracking
- Thread-safe task addition

### 2. Scheduler

The central coordinator that orchestrates execution:

```cpp
class Scheduler {
    TaskGraph graph_;
    std::unique_ptr<SchedulingPolicy> policy_;
    ExecutionEngine executor_;
    Profiler profiler_;
    
public:
    void execute();
    void set_policy(std::unique_ptr<SchedulingPolicy> policy);
    void set_profiling(bool enabled);
};
```

**Responsibilities:**
1. Validate task graph (cycle detection)
2. Select scheduling policy
3. Coordinate execution engine
4. Manage profiling and logging

### 3. Execution Engine

```
┌─────────────────────────────────────────────┐
│           Execution Engine                   │
│  ┌─────────────────────┐ ┌───────────────┐  │
│  │   CPU Thread Pool   │ │ Stream Manager│  │
│  │  ┌───┐ ┌───┐ ┌───┐  │ │ ┌───┐ ┌───┐   │  │
│  │  │ T │ │ T │ │ T │  │ │ │S0 │ │S1 │   │  │
│  │  └───┘ └───┘ └───┘  │ │ └───┘ └───┘   │  │
│  └─────────────────────┘ └───────────────┘  │
└─────────────────────────────────────────────┘
```

**CPU Thread Pool:**
- Work-stealing queue for load balancing
- Configurable thread count (default: hardware concurrency)
- Thread-local allocation caches

**GPU Stream Manager:**
- Pool of CUDA streams for concurrent execution
- Automatic stream assignment
- Synchronization primitives for cross-stream dependencies

### 4. Scheduling Policies

| Policy | Algorithm | Use Case |
|--------|-----------|----------|
| `DefaultPolicy` | Load-based selection | General purpose |
| `GpuFirstPolicy` | Prefer GPU | GPU-bound workloads |
| `CpuFirstPolicy` | Prefer CPU | CPU-bound workloads |
| `RoundRobinPolicy` | Alternating | Balanced utilization |
| `ShortestJobFirstPolicy` | Priority queue | Latency-sensitive |

Policy Interface:
```cpp
class SchedulingPolicy {
public:
    virtual DeviceType select_device(
        const Task& task,
        const SystemStatus& status
    ) = 0;
};
```

### 5. Memory Pool

Buddy system allocator for efficient GPU memory:

```
┌─────────────────────────────────────────────────────────┐
│              Buddy System Allocator                      │
├─────────────────────────────────────────────────────────┤
│  Order 0 │ 64B   │ ██ │ ██ │ ██ │ ██ │ ██ │ ██ │ ██ │   │
├──────────┼───────┼────┼────┼────┼────┼────┼────┼────┤   │
│  Order 1 │ 128B  │    ████████    │    ████████    │   │
├──────────┼───────┼────────────────┼────────────────┤   │
│  Order 2 │ 256B  │           █████████████████      │   │
├──────────┼───────┼──────────────────────────────────┤   │
│  Order 3 │ 512B  │ █████████████████████████████████ │   │
└─────────────────────────────────────────────────────────┘
        █ = 已分配   (██ = allocated)
```

**Benefits:**
- Eliminates cudaMalloc/cudaFree overhead
- Reduces memory fragmentation
- O(log n) allocation/deallocation
- Automatic coalescing of free blocks

### 6. Dependency Manager

Tracks task readiness and scheduling:

```cpp
class DependencyManager {
    std::unordered_map<TaskId, size_t> remaining_deps_;
    std::queue<TaskId> ready_queue_;
    std::mutex mutex_;
    
public:
    void initialize(const TaskGraph& graph);
    std::optional<TaskId> next_ready_task();
    void mark_completed(TaskId id);
};
```

---

## Data Flow

### Task Execution Pipeline

```
┌─────────┐    ┌─────────────┐    ┌─────────────────┐    ┌──────────┐
│  User   │    │  Scheduler  │    │  Exec Engine    │    │  Device  │
│  Code   │───→│             │───→│                 │───→│          │
└─────────┘    └─────────────┘    └─────────────────┘    └──────────┘
      │              │                    │                  │
      │     ┌───────┴────────┐      ┌────┴────┐             │
      │     │  1. Validate   │      │ 3. Pick │             │
      │     │     Graph      │      │  Policy │             │
      │     └───────┬────────┘      └────┬────┘             │
      │             │                    │                  │
      │     ┌───────▼────────┐      ┌────▼────┐             │
      │     │  2. Topo Sort  │      │ 4. Exec │             │
      │     └───────┬────────┘      └────┬────┘             │
      │             │                    │                  │
      │             └────────────────────┘                  │
      │                          │                          │
      └──────────────────────────┼──────────────────────────┘
                                 ▼
                    [Complete / Error / Retry]
```

1. **Validation**: Check for cycles in dependency graph
2. **Topological Sort**: Determine execution order
3. **Policy Selection**: Choose CPU or GPU for each task
4. **Execution**: Dispatch to appropriate execution queue
5. **Completion**: Trigger dependent tasks or handle errors

---

## Execution Model

### State Machine

```
                    ┌─────────┐
                    │ Created │
                    └────┬────┘
                         │ add_dependency
                         ▼
              ┌──────────────────────┐
              │       Ready          │
              │ (deps satisfied)     │
              └────┬─────────────────┘
                   │ schedule
                   ▼
    ┌───────────────────────────────┐
    │           Running             │
    │  ┌───────────┐ ┌──────────┐   │
    │  │  On CPU   │ │  On GPU  │   │
    │  └───────────┘ └──────────┘   │
    └────┬──────────────────────────┘
         │
    ┌────┴────┬──────────┐
    │         │          │
    ▼         ▼          ▼
┌───────┐ ┌───────┐ ┌─────────┐
│Success│ │Failed │ │Cancelled│
└───┬───┘ └───┬───┘ └─────────┘
    │         │
    │    ┌────┘
    │    │ retry
    │    ▼
    │ ┌─────────────┐
    └→│  Retrying   │
      └─────────────┘
```

---

## Threading Model

### CPU Execution

```
Main Thread                    Worker Threads
     │                              │
     │  submit(task)                │
     │──────────────┐               │
     │              │               │
     │              ▼               ▼
     │         ┌─────────────────────────┐
     │         │      Task Queue         │
     │         │  ┌─────┐ ┌─────┐ ┌─────┐│
     │         │  │ T1  │ │ T2  │ │ T3  ││
     │         │  └─────┘ └─────┘ └─────┘│
     │         └─────────────────────────┘
     │                     │
     │            ┌────────┼────────┐
     │            │        │        │
     │            ▼        ▼        ▼
     │         ┌───┐   ┌───┐   ┌───┐
     │         │ W1│   │ W2│   │ W3│  ← Worker Loop
     │         └───┘   └───┘   └───┘
     │            │        │        │
     │            └────────┼────────┘
     │                     │
     ▼                     ▼
 wait_for_completion()  execute(task)
```

### GPU Execution

```
CPU Thread                    GPU Stream
     │                            │
     │  launch_kernel(stream)     │
     │───────────────────────────→│
     │                            │
     │  cudaStreamSynchronize     │
     │───────────────────────────→│
     │                            │ kernel execution
     │                            │
     │◄───────────────────────────│ callback
     │                            │ (notify completion)
```

---

## Design Decisions

### 1. Why DAG-based?

- **Expressiveness**: Can represent any parallel pattern
- **Optimization**: Static analysis enables better scheduling
- **Determinism**: Same graph always produces same execution

### 2. Why Buddy System for Memory?

| Allocator | Pros | Cons |
|-----------|------|------|
| cudaMalloc | Simple | High overhead, fragmentation |
| Pool (fixed) | Fast | Wasted space, size limitations |
| **Buddy** | Fast, flexible | Internal fragmentation (bounded) |

### 3. Why Policy-based Scheduling?

- Flexibility for different workloads
- Easy to extend with custom policies
- A/B testing different strategies

### 4. Thread Safety Model

| Component | Thread Safety | Notes |
|-----------|---------------|-------|
| TaskGraph | Read: Safe | Write: Before execute() only |
| Scheduler | Safe | All public methods |
| ExecutionEngine | Internal | Locked internally |
| MemoryPool | Safe | Fine-grained locking |

---

## Performance Characteristics

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Task addition | O(1) | Amortized |
| Dependency addition | O(1) | 
| Cycle detection | O(V + E) | One-time at execute() |
| Topological sort | O(V + E) | One-time at execute() |
| Memory allocation | O(log n) | n = pool size |
| Task scheduling | O(1) | Per task |

---

## Further Reading

- [Scheduling Policies](scheduling-policies.md) - Policy details
- [Memory Management](memory-management.md) - Memory system
- [API Reference](api-reference.md) - Component APIs
