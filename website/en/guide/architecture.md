# Architecture

This page explains the internal architecture and design of HTS.

## High-Level Architecture

```mermaid
flowchart TB
    subgraph Builder["Builder Layer"]
        TG[TaskGraph]
        TB[TaskBuilder]
    end

    subgraph Scheduler["Scheduler Layer"]
        SCH[Scheduler]
        POL[SchedulingPolicy]
        RET[RetryPolicy]
        EVT[EventSystem]
    end

    subgraph Execution["Execution Layer"]
        CPU[CPU Thread Pool]
        GPU[CUDA Streams]
        SM[StreamManager]
        MP[MemoryPool]
    end

    App[User Application] --> TG
    App --> TB
    TG --> SCH
    TB --> TG
    SCH --> POL
    SCH --> RET
    SCH --> EVT
    SCH --> CPU
    SCH --> GPU
    GPU --> SM
    GPU --> MP
```

## Core Components

### 1. TaskGraph

The `TaskGraph` class manages the DAG (Directed Acyclic Graph) of tasks:

- **Task Storage**: Maintains all tasks with their configurations
- **Dependency Tracking**: Tracks predecessor/successor relationships
- **Topological Sorting**: Provides execution order
- **Cycle Detection**: Validates DAG structure
- **Ready Queue**: Identifies tasks ready for execution

**Key Files:**
- `include/hts/task_graph.hpp`
- `src/core/task_graph.cpp`

### 2. Task and TaskContext

Each task is represented by a `Task` object with an associated `TaskContext`:

**Task Properties:**
```cpp
- id: uint64_t              // Unique identifier
- name: string              // Human-readable name
- device_type: DeviceType   // CPU or GPU
- priority: int            // Scheduling priority
- status: TaskStatus       // Current execution state
- retry_policy: RetryPolicy // Failure handling
```

**TaskContext:**
Provides runtime information and utilities to task functions:
```cpp
- get_task_id()
- get_device_type()
- get_execution_time()
- get_retry_count()
```

**Key Files:**
- `include/hts/task.hpp`
- `include/hts/task_context.hpp`
- `src/core/task.cpp`

### 3. Scheduler

The `Scheduler` orchestrates the entire execution:

**Responsibilities:**
- Initialize and validate TaskGraph
- Maintain execution state
- Select ready tasks based on policy
- Dispatch tasks to appropriate executors
- Track completion and handle failures
- Collect profiling information

**Execution Flow:**
1. `init(&graph)` - Validate and prepare
2. `execute()` - Start execution
3. Policy selects ready tasks
4. Tasks dispatched to CPU threads or GPU streams
5. `wait_for_completion()` - Block until done
6. Collect stats and profiling data

**Key Files:**
- `include/hts/scheduler.hpp`
- `src/cuda/scheduler.cu`

### 4. Scheduling Policies

HTS uses a pluggable policy architecture:

```cpp
class SchedulingPolicy {
    virtual Task* select_next(
        const std::vector<Task*>& ready_queue
    ) = 0;
};
```

**Built-in Policies:**

| Policy | Strategy | Use Case |
|--------|----------|----------|
| `GPUPriorityPolicy` | Prefer GPU tasks | GPU-heavy workloads |
| `CPUPriorityPolicy` | Prefer CPU tasks | CPU preprocessing |
| `RoundRobinPolicy` | Alternate CPU/GPU | Balanced workloads |
| `LoadBasedPolicy` | Select by current load | Dynamic workloads |

**Key Files:**
- `include/hts/scheduling_policy.hpp`

### 5. Memory Pool

GPU memory management uses a buddy system allocator:

**Features:**
- Eliminates `cudaMalloc`/`cudaFree` overhead
- O(log n) allocation time
- Automatic defragmentation
- Configurable pool size

**Allocation Flow:**
1. Task requests GPU memory
2. MemoryPool finds suitable block
3. Splits blocks if needed (buddy system)
4. Returns pointer
5. On free, merges with buddy if possible

**Key Files:**
- `include/hts/memory_pool.hpp`
- `src/cuda/memory_pool.cu`

### 6. Stream Manager

Manages CUDA streams for concurrent GPU execution:

**Capabilities:**
- Create and manage multiple streams
- Stream priority support
- Automatic stream reuse
- Synchronization primitives

**Key Files:**
- `include/hts/stream_manager.hpp`
- `src/cuda/stream_manager.cu`

### 7. Execution Engine

Dispatches tasks to CPU threads or GPU streams:

**CPU Execution:**
- Thread pool for parallel execution
- Work-stealing support
- Affinity configuration

**GPU Execution:**
- CUDA stream management
- Kernel launch coordination
- Memory transfer handling

**Key Files:**
- `include/hts/execution_engine.hpp`
- `src/cuda/execution_engine.cu`

### 8. Profiler

Built-in performance monitoring:

**Metrics Collected:**
- Task execution times
- Device utilization
- Memory allocation patterns
- Dependency wait times
- Parallelism metrics

**Export:**
- JSON format
- Chrome tracing format
- CSV format

**Key Files:**
- `include/hts/profiler.hpp`

## Design Principles

### Zero-Overhead Abstraction

HTS follows the C++ principle of "you don't pay for what you don't use":

- No virtual calls in hot paths (when not using polymorphic features)
- Compile-time device type selection when possible
- Inline functions for simple operations
- Template metaprogramming for type safety

### Lock-Free Where Possible

Critical paths use lock-free data structures:
- Atomic operations for status updates
- Lock-free queues for ready tasks
- Compare-and-swap for state transitions

### Error Resilience

- Comprehensive error codes (see `types.hpp`)
- Retry policies for transient failures
- Graceful degradation on errors
- Detailed error messages with context

## Threading Model

```mermaid
flowchart TB
    Main[Main Thread]
    Sch[Scheduler Thread]
    Prof[Profiler Thread]

    Main --> Sch
    Main --> Prof

    subgraph CPU["CPU Thread Pool (8 threads)"]
        W1[Worker 1]
        W2[Worker 2]
        Wn[Worker N]
    end

    subgraph GPU["GPU Streams (4 streams)"]
        S0[Stream 0]
        S1[Stream 1]
        Sn[Stream N]
    end

    Sch --> CPU
    Sch --> GPU
```

## Execution Flow

```mermaid
sequenceDiagram
    participant App as Application
    participant TG as TaskGraph
    participant Sch as Scheduler
    participant CPU as CPU Pool
    participant GPU as GPU Streams

    App->>TG: Build task graph
    App->>Sch: Init(graph)
    Sch->>TG: Validate DAG
    Sch-->>App: Ready

    loop Until complete
        Sch->>Sch: Select ready tasks
        alt CPU Task
            Sch->>CPU: Dispatch task
            CPU->>CPU: Execute
            CPU-->>Sch: Task complete
        else GPU Task
            Sch->>GPU: Dispatch task
            GPU->>GPU: Execute kernel
            GPU-->>Sch: Task complete
        end
        Sch->>TG: Update dependencies
    end

    Sch-->>App: All tasks complete
```

## Core Class Relationships

```mermaid
classDiagram
    class Scheduler {
        -TaskGraph* graph
        -SchedulingPolicy* policy
        +init(graph)
        +execute()
        +wait_for_completion()
    }

    class TaskGraph {
        -vector~Task~ tasks
        -DependencyGraph deps
        +add_task()
        +add_dependency()
        +get_ready_tasks()
    }

    class TaskBuilder {
        -TaskGraph* graph
        +set_name()
        +set_device()
        +set_priority()
        +build()
    }

    class SchedulingPolicy {
        <<interface>>
        +select_next(ready_queue) Task*
    }

    class GPUPriorityPolicy {
        +select_next() Task*
    }

    class CPUPriorityPolicy {
        +select_next() Task*
    }

    Scheduler --> TaskGraph : manages
    Scheduler --> SchedulingPolicy : uses
    TaskBuilder --> TaskGraph : builds
    GPUPriorityPolicy --|> SchedulingPolicy
    CPUPriorityPolicy --|> SchedulingPolicy
```

## Next Steps

- [Task Graph](/en/guide/task-graph) — Deep dive into DAG management
- [Scheduling](/en/guide/scheduling) — Scheduling policies in detail
- [Memory](/en/guide/memory) — Memory pool implementation
- [API Reference](/en/api/) — Complete API documentation
