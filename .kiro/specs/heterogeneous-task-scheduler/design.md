# Design Document: Heterogeneous Task Scheduler

## Overview

本设计文档描述了异构任务图调度器的架构和实现细节。该框架采用分层架构，将任务图管理、依赖跟踪、内存管理和执行引擎解耦，以实现高内聚低耦合的设计。

核心设计原则：
- **零拷贝内存管理**：通过内存池避免频繁的 GPU 内存分配
- **异步优先**：默认使用异步执行，最大化 CPU/GPU 并行度
- **类型安全**：利用 C++ 模板和 RAII 确保资源安全
- **可扩展性**：支持多 GPU 和自定义调度策略

## Architecture

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
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ Block Allocator │  │ Free List       │  │ Statistics      │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                    CUDA Runtime / CPU Threads                    │
└─────────────────────────────────────────────────────────────────┘
```

## Components and Interfaces

### Task

任务是调度的基本单元，封装了计算逻辑和元数据。

```cpp
enum class DeviceType {
    CPU,
    GPU,
    Any  // 调度器自动选择
};

enum class TaskState {
    Pending,    // 等待依赖
    Ready,      // 可执行
    Running,    // 执行中
    Completed,  // 已完成
    Failed      // 执行失败
};

class Task {
public:
    using TaskId = uint64_t;
    using CpuFunction = std::function<void(TaskContext&)>;
    using GpuFunction = std::function<void(TaskContext&, cudaStream_t)>;

    Task(TaskId id, DeviceType preferred_device);
    
    // 设置执行函数
    void set_cpu_function(CpuFunction func);
    void set_gpu_function(GpuFunction func);
    
    // 状态查询
    TaskId id() const;
    TaskState state() const;
    DeviceType preferred_device() const;
    DeviceType actual_device() const;
    
    // 执行时间统计
    std::chrono::nanoseconds execution_time() const;

private:
    TaskId id_;
    TaskState state_;
    DeviceType preferred_device_;
    DeviceType actual_device_;
    CpuFunction cpu_func_;
    GpuFunction gpu_func_;
    std::chrono::nanoseconds exec_time_;
};
```

### TaskContext

任务执行上下文，提供内存分配和数据访问接口。

```cpp
class TaskContext {
public:
    // 从内存池分配 GPU 内存
    void* allocate_gpu_memory(size_t bytes);
    void free_gpu_memory(void* ptr);
    
    // 获取上游任务的输出
    template<typename T>
    T* get_input(const std::string& name);
    
    // 设置输出供下游任务使用
    template<typename T>
    void set_output(const std::string& name, T* data, size_t size);
    
    // 错误报告
    void report_error(const std::string& message);

private:
    MemoryPool* memory_pool_;
    std::unordered_map<std::string, void*> inputs_;
    std::unordered_map<std::string, void*> outputs_;
};
```

### TaskGraph

任务图管理器，负责构建和验证 DAG。

```cpp
class TaskGraph {
public:
    using TaskPtr = std::shared_ptr<Task>;
    
    // 添加任务
    TaskPtr add_task(DeviceType device = DeviceType::Any);
    
    // 添加依赖关系
    // 返回 true 表示成功，false 表示会形成环
    bool add_dependency(Task::TaskId from, Task::TaskId to);
    
    // 验证图结构
    bool validate() const;
    
    // 获取拓扑排序
    std::vector<TaskPtr> topological_sort() const;
    
    // 获取入度为0的任务（初始可执行任务）
    std::vector<TaskPtr> get_root_tasks() const;
    
    // 获取任务的所有后继
    std::vector<TaskPtr> get_successors(Task::TaskId id) const;
    
    // 获取任务的所有前驱
    std::vector<TaskPtr> get_predecessors(Task::TaskId id) const;

private:
    std::unordered_map<Task::TaskId, TaskPtr> tasks_;
    std::unordered_map<Task::TaskId, std::vector<Task::TaskId>> adjacency_list_;
    std::unordered_map<Task::TaskId, std::vector<Task::TaskId>> reverse_adjacency_;
    
    // 环检测（DFS）
    bool has_cycle() const;
};
```

### DependencyManager

依赖管理器，跟踪任务完成状态和依赖满足情况。

```cpp
class DependencyManager {
public:
    explicit DependencyManager(const TaskGraph& graph);
    
    // 标记任务完成
    void mark_completed(Task::TaskId id);
    
    // 标记任务失败
    void mark_failed(Task::TaskId id);
    
    // 检查任务是否就绪
    bool is_ready(Task::TaskId id) const;
    
    // 获取所有就绪任务
    std::vector<Task::TaskId> get_ready_tasks() const;
    
    // 获取未满足的依赖数量
    size_t pending_dependency_count(Task::TaskId id) const;

private:
    const TaskGraph& graph_;
    std::unordered_map<Task::TaskId, size_t> pending_deps_;
    std::unordered_set<Task::TaskId> completed_;
    std::unordered_set<Task::TaskId> failed_;
    mutable std::mutex mutex_;
};
```

### MemoryPool

GPU 内存池，采用伙伴系统（Buddy System）算法管理内存块。

```cpp
struct MemoryBlock {
    void* ptr;
    size_t size;
    bool in_use;
};

struct MemoryStats {
    size_t total_bytes;
    size_t used_bytes;
    size_t peak_bytes;
    size_t allocation_count;
    size_t free_count;
    double fragmentation_ratio;
};

class MemoryPool {
public:
    explicit MemoryPool(size_t initial_size_bytes, bool allow_growth = true);
    ~MemoryPool();
    
    // 分配内存
    void* allocate(size_t bytes);
    
    // 释放内存
    void free(void* ptr);
    
    // 获取统计信息
    MemoryStats get_stats() const;
    
    // 重置池（释放所有分配）
    void reset();

private:
    size_t pool_size_;
    bool allow_growth_;
    void* base_ptr_;
    
    // 按大小分类的空闲块列表
    std::map<size_t, std::list<MemoryBlock>> free_lists_;
    std::unordered_map<void*, MemoryBlock> allocated_blocks_;
    
    MemoryStats stats_;
    mutable std::mutex mutex_;
    
    // 内部方法
    size_t round_up_to_power_of_two(size_t size);
    void split_block(MemoryBlock& block, size_t target_size);
    void coalesce_blocks();
};
```

### StreamManager

CUDA 流管理器，管理多个流以实现并发执行。

```cpp
class StreamManager {
public:
    explicit StreamManager(size_t num_streams = 4);
    ~StreamManager();
    
    // 获取一个可用的流
    cudaStream_t acquire_stream();
    
    // 释放流
    void release_stream(cudaStream_t stream);
    
    // 同步所有流
    void synchronize_all();
    
    // 创建事件用于流间同步
    cudaEvent_t create_event();
    void destroy_event(cudaEvent_t event);

private:
    std::vector<cudaStream_t> streams_;
    std::queue<cudaStream_t> available_streams_;
    std::mutex mutex_;
    std::condition_variable cv_;
};
```

### ExecutionEngine

执行引擎，负责在指定设备上运行任务。

```cpp
class ExecutionEngine {
public:
    ExecutionEngine(MemoryPool& memory_pool, size_t cpu_threads = 4, size_t gpu_streams = 4);
    ~ExecutionEngine();
    
    // 执行单个任务
    std::future<void> execute_task(std::shared_ptr<Task> task, DeviceType device);
    
    // 等待所有任务完成
    void wait_all();
    
    // 获取设备负载
    double get_cpu_load() const;
    double get_gpu_load() const;

private:
    MemoryPool& memory_pool_;
    StreamManager stream_manager_;
    
    // CPU 线程池
    std::vector<std::thread> cpu_workers_;
    std::queue<std::function<void()>> cpu_task_queue_;
    std::mutex cpu_mutex_;
    std::condition_variable cpu_cv_;
    std::atomic<bool> stop_flag_;
    std::atomic<size_t> active_cpu_tasks_;
    
    // GPU 任务跟踪
    std::atomic<size_t> active_gpu_tasks_;
    
    void cpu_worker_loop();
    void execute_on_cpu(std::shared_ptr<Task> task, std::promise<void> promise);
    void execute_on_gpu(std::shared_ptr<Task> task, std::promise<void> promise);
};
```

### Scheduler

核心调度器，协调所有组件。

```cpp
struct SchedulerConfig {
    size_t memory_pool_size = 1024 * 1024 * 256;  // 256 MB
    size_t cpu_thread_count = 4;
    size_t gpu_stream_count = 4;
    bool allow_memory_growth = true;
};

class Scheduler {
public:
    explicit Scheduler(const SchedulerConfig& config = {});
    ~Scheduler();
    
    // 构建任务图
    TaskGraph& graph();
    
    // 同步执行
    void execute();
    
    // 异步执行
    std::future<void> execute_async();
    
    // 设置错误回调
    using ErrorCallback = std::function<void(Task::TaskId, const std::string&)>;
    void set_error_callback(ErrorCallback callback);
    
    // 获取执行统计
    struct ExecutionStats {
        std::chrono::nanoseconds total_time;
        std::unordered_map<Task::TaskId, std::chrono::nanoseconds> task_times;
        double cpu_utilization;
        double gpu_utilization;
        MemoryStats memory_stats;
    };
    ExecutionStats get_stats() const;
    
    // 生成执行时间线（用于可视化）
    std::string generate_timeline_json() const;

private:
    SchedulerConfig config_;
    TaskGraph graph_;
    std::unique_ptr<MemoryPool> memory_pool_;
    std::unique_ptr<ExecutionEngine> engine_;
    std::unique_ptr<DependencyManager> dep_manager_;
    
    ErrorCallback error_callback_;
    ExecutionStats stats_;
    
    void schedule_ready_tasks();
    DeviceType select_device(const Task& task);
};
```

## Data Models

### Task Execution Timeline

```cpp
struct TimelineEvent {
    Task::TaskId task_id;
    DeviceType device;
    std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
    std::chrono::time_point<std::chrono::high_resolution_clock> end_time;
    TaskState final_state;
};

struct ExecutionTimeline {
    std::vector<TimelineEvent> events;
    std::chrono::time_point<std::chrono::high_resolution_clock> graph_start;
    std::chrono::time_point<std::chrono::high_resolution_clock> graph_end;
};
```

### Error Information

```cpp
struct TaskError {
    Task::TaskId task_id;
    DeviceType device;
    std::string message;
    int cuda_error_code;  // 0 if not a CUDA error
    std::chrono::time_point<std::chrono::system_clock> timestamp;
};
```



## Correctness Properties

*A property is a characteristic or behavior that should hold true across all valid executions of a system—essentially, a formal statement about what the system should do. Properties serve as the bridge between human-readable specifications and machine-verifiable correctness guarantees.*

### Property 1: Task Creation Invariants

*For any* task created with a device preference and computation functions, the task SHALL have a unique identifier, store the specified device preference, and correctly retain both CPU and GPU function references.

**Validates: Requirements 1.1, 1.4, 5.1, 8.2**

### Property 2: Cycle Detection

*For any* task graph, if adding an edge would create a cycle, the add_dependency operation SHALL return false and leave the graph unchanged. Conversely, for any valid DAG, validation SHALL pass.

**Validates: Requirements 1.3, 1.5**

### Property 3: Dependency Recording Round Trip

*For any* two tasks A and B in a graph, after successfully adding a dependency from A to B, querying get_predecessors(B) SHALL include A, and querying get_successors(A) SHALL include B.

**Validates: Requirements 1.2**

### Property 4: Dependency Execution Order

*For any* execution trace of a task graph, no task SHALL begin execution before all of its predecessor tasks have reached the Completed state. Equivalently, for any task T, if T.state == Running or T.state == Completed, then for all P in predecessors(T), P.state == Completed.

**Validates: Requirements 2.1, 2.2, 2.3**

### Property 5: Failure Propagation

*For any* task T that transitions to Failed state, all tasks in the transitive closure of successors(T) SHALL be marked as blocked and SHALL NOT execute.

**Validates: Requirements 2.5**

### Property 6: Memory Pool Round Trip

*For any* sequence of allocations and deallocations on a memory pool, the following invariants hold:
1. After initialization with size N, total_bytes >= N
2. After allocating M bytes, used_bytes increases by at least M
3. After freeing a block, that memory becomes available for reallocation
4. peak_bytes >= used_bytes at all times

**Validates: Requirements 3.1, 3.2, 3.3, 3.5**

### Property 7: Memory Pool Growth

*For any* memory pool configured with allow_growth=true, when an allocation request exceeds available memory, the pool SHALL either expand to accommodate the request or block until memory is available (never crash or return null without error).

**Validates: Requirements 3.4**

### Property 8: Concurrent Execution

*For any* task graph with independent tasks (no dependency path between them), if one task is assigned to GPU and another to CPU, their execution time windows MAY overlap. Furthermore, independent GPU tasks SHALL be assigned to different CUDA streams.

**Validates: Requirements 4.2, 4.3, 4.4, 5.4**

### Property 9: Load Balancing

*For any* task with DeviceType::Any preference, the scheduler SHALL assign it to the device with lower current utilization. After assignment, the utilization metrics SHALL reflect the new task.

**Validates: Requirements 5.2, 5.3**

### Property 10: Error Propagation

*For any* error occurring during task execution (including CUDA errors), the error information SHALL contain: task_id, device type, error message, and (for CUDA errors) the error code. User-defined error callbacks SHALL be invoked with this information.

**Validates: Requirements 6.1, 6.2, 6.3, 6.4**

### Property 11: Graceful Shutdown

*For any* scheduler shutdown request, all tasks that were in Running state at the time of the request SHALL complete (either successfully or with failure) before the scheduler destructor returns.

**Validates: Requirements 6.5**

### Property 12: Statistics Accuracy

*For any* completed task graph execution:
1. Each task's recorded execution_time SHALL be > 0
2. total_time >= max(sum of execution times along any path from root to leaf)
3. cpu_utilization and gpu_utilization SHALL be in range [0.0, 1.0]
4. memory fragmentation_ratio SHALL be in range [0.0, 1.0]

**Validates: Requirements 7.1, 7.2, 7.3, 7.5**

### Property 13: Timeline Generation

*For any* completed task graph execution, the generated timeline JSON SHALL contain an entry for every task that was executed, with valid start_time < end_time for each entry.

**Validates: Requirements 7.4**

### Property 14: Execution Mode Correctness

*For any* task graph:
1. After synchronous execute() returns, all tasks SHALL be in Completed or Failed state
2. The future returned by execute_async() SHALL resolve only after all tasks reach Completed or Failed state
3. Both modes SHALL produce identical final task states for the same graph

**Validates: Requirements 8.3, 8.4, 8.5**

## Error Handling

### Error Categories

1. **Graph Construction Errors**
   - Cycle detection: Return false from add_dependency, graph remains unchanged
   - Invalid task ID: Throw std::invalid_argument
   - Duplicate task ID: Throw std::runtime_error

2. **Memory Errors**
   - Allocation failure: Return nullptr and set error flag, or throw std::bad_alloc if configured
   - Double free: Log warning, ignore operation
   - Invalid pointer: Throw std::invalid_argument

3. **CUDA Errors**
   - Capture cudaError_t from all CUDA calls
   - Wrap in TaskError with full context
   - Propagate to error callback and mark task as Failed

4. **Execution Errors**
   - Task function throws: Catch, wrap in TaskError, mark as Failed
   - Timeout: Mark as Failed with timeout message
   - Resource exhaustion: Queue task for retry or fail based on policy

### Error Recovery Strategies

```cpp
enum class ErrorPolicy {
    FailFast,      // Stop execution on first error
    ContinueOthers, // Continue independent tasks, block dependents
    RetryOnce      // Retry failed task once before failing
};
```

## Testing Strategy

### Unit Tests

Unit tests verify specific examples and edge cases:

1. **Task Creation**: Verify task IDs are unique, device preferences are stored
2. **Graph Construction**: Test adding tasks, dependencies, cycle detection edge cases
3. **Memory Pool**: Test allocation/free sequences, boundary conditions
4. **Stream Manager**: Test stream acquisition/release, synchronization

### Property-Based Tests

Property-based tests verify universal properties across randomly generated inputs. We will use [Rapidcheck](https://github.com/emil-e/rapidcheck) as the C++ property-based testing library.

Configuration:
- Minimum 100 iterations per property test
- Each test tagged with: **Feature: heterogeneous-task-scheduler, Property N: [property name]**

**Test Generators Required:**

1. **Arbitrary Task Graph Generator**: Generate random DAGs with configurable size and density
2. **Arbitrary Allocation Sequence Generator**: Generate sequences of allocate/free operations
3. **Arbitrary Task Configuration Generator**: Generate tasks with random device preferences and functions

**Property Test Implementation Pattern:**

```cpp
RC_GTEST_PROP(TaskGraph, CycleDetection, ()) {
    // Feature: heterogeneous-task-scheduler, Property 2: Cycle Detection
    auto graph = *rc::gen::arbitrary<TaskGraph>();
    auto [from, to] = *rc::gen::suchThat<std::pair<TaskId, TaskId>>(
        [&](auto p) { return would_create_cycle(graph, p.first, p.second); }
    );
    
    auto original = graph;
    bool result = graph.add_dependency(from, to);
    
    RC_ASSERT(!result);
    RC_ASSERT(graph == original);  // Graph unchanged
}
```

### Integration Tests

1. **End-to-End Execution**: Execute complete task graphs, verify all tasks complete
2. **CPU-GPU Concurrency**: Verify overlapping execution with timing measurements
3. **Error Propagation**: Inject failures, verify dependent tasks are blocked
4. **Memory Pool Under Load**: Concurrent allocations from multiple tasks

### Performance Tests

1. **Scheduling Overhead**: Measure time to schedule N tasks
2. **Memory Pool Throughput**: Allocations per second
3. **Concurrency Scaling**: Throughput vs. number of streams/threads
