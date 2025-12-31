# Requirements Document

## Introduction

异构任务图调度器（Heterogeneous Task Scheduler）是一个 C++ 框架，允许用户定义由多个任务组成的有向无环图（DAG），框架自动将任务分配给 CPU 或 GPU 执行。该框架关注 CPU 和 GPU 的协同管理，提供依赖管理、内存池和异步并发能力，类似于 TensorFlow/PyTorch 的底层调度逻辑或游戏引擎中的渲染图。

## Glossary

- **Scheduler**: 核心调度器组件，负责管理任务图的执行和资源分配
- **Task**: 可在 CPU 或 GPU 上执行的计算单元
- **Task_Graph**: 由任务和依赖关系组成的有向无环图
- **Memory_Pool**: 显存池组件，管理 GPU 内存的分配和回收
- **Execution_Engine**: 执行引擎，负责在指定设备上运行任务
- **Dependency_Manager**: 依赖管理器，跟踪任务间的依赖关系
- **Device**: 执行设备，可以是 CPU 或 GPU
- **Stream**: CUDA 流，用于异步执行 GPU 任务

## Requirements

### Requirement 1: Task Definition and Graph Construction

**User Story:** As a developer, I want to define tasks and their dependencies as a DAG, so that I can express complex computation workflows.

#### Acceptance Criteria

1. THE Task_Graph SHALL allow users to create tasks with unique identifiers and execution targets (CPU or GPU)
2. WHEN a user adds a dependency between Task A and Task B, THE Dependency_Manager SHALL record that Task B depends on Task A
3. WHEN a user attempts to add a dependency that would create a cycle, THE Task_Graph SHALL reject the dependency and return an error
4. THE Task SHALL support user-defined computation functions for both CPU and GPU execution
5. WHEN a Task_Graph is finalized, THE Scheduler SHALL validate that all dependencies form a valid DAG

### Requirement 2: Dependency Management and Execution Order

**User Story:** As a developer, I want the scheduler to automatically manage task dependencies, so that tasks execute only when their prerequisites are complete.

#### Acceptance Criteria

1. WHEN Task A completes execution, THE Dependency_Manager SHALL notify all tasks that depend on Task A
2. WHEN all dependencies of a Task are satisfied, THE Scheduler SHALL add the Task to the ready queue
3. THE Scheduler SHALL never execute a Task before all its dependencies have completed
4. WHEN multiple tasks become ready simultaneously, THE Scheduler SHALL support concurrent execution on available devices
5. IF a Task fails during execution, THEN THE Scheduler SHALL mark all dependent tasks as blocked and report the failure

### Requirement 3: Memory Pool Management

**User Story:** As a developer, I want a memory pool to manage GPU memory efficiently, so that I avoid the overhead of frequent cudaMalloc and cudaFree calls.

#### Acceptance Criteria

1. THE Memory_Pool SHALL pre-allocate a configurable amount of GPU memory at initialization
2. WHEN a Task requests memory, THE Memory_Pool SHALL return a memory block from the pool without calling cudaMalloc
3. WHEN a Task releases memory, THE Memory_Pool SHALL return the block to the pool without calling cudaFree
4. IF the Memory_Pool has insufficient free memory, THEN THE Memory_Pool SHALL either expand the pool or block until memory becomes available
5. THE Memory_Pool SHALL track memory usage statistics including total allocated, currently used, and peak usage
6. WHEN the Memory_Pool is destroyed, THE Memory_Pool SHALL release all GPU memory back to the system

### Requirement 4: Asynchronous Concurrent Execution

**User Story:** As a developer, I want CPU and GPU tasks to execute concurrently, so that I maximize hardware utilization.

#### Acceptance Criteria

1. THE Execution_Engine SHALL use CUDA Streams to enable asynchronous GPU task execution
2. WHILE a GPU Task is executing, THE Scheduler SHALL allow CPU tasks to execute concurrently
3. WHEN multiple independent GPU tasks are ready, THE Execution_Engine SHALL execute them on separate CUDA Streams
4. THE Scheduler SHALL overlap CPU task preparation with GPU task execution
5. WHEN synchronization is required between CPU and GPU, THE Execution_Engine SHALL use CUDA events for efficient waiting

### Requirement 5: Device Assignment and Load Balancing

**User Story:** As a developer, I want the scheduler to intelligently assign tasks to devices, so that workload is balanced across CPU and GPU.

#### Acceptance Criteria

1. THE Task SHALL allow users to specify preferred execution device (CPU, GPU, or Any)
2. WHEN a Task is marked as "Any", THE Scheduler SHALL assign it to the least loaded device
3. THE Scheduler SHALL track device utilization and queue depths for load balancing decisions
4. WHEN a GPU is fully utilized, THE Scheduler SHALL queue GPU tasks rather than blocking CPU execution
5. THE Scheduler SHALL support multiple GPU devices when available

### Requirement 6: Error Handling and Recovery

**User Story:** As a developer, I want robust error handling, so that I can diagnose and recover from failures.

#### Acceptance Criteria

1. IF a CUDA operation fails, THEN THE Execution_Engine SHALL capture the error code and propagate it to the Scheduler
2. WHEN a Task fails, THE Scheduler SHALL provide detailed error information including task ID, device, and error message
3. THE Scheduler SHALL support user-defined error callbacks for custom error handling
4. IF a memory allocation fails, THEN THE Memory_Pool SHALL return an error rather than crashing
5. THE Scheduler SHALL support graceful shutdown, completing in-flight tasks before termination

### Requirement 7: Performance Monitoring

**User Story:** As a developer, I want to monitor scheduler performance, so that I can optimize my task graphs.

#### Acceptance Criteria

1. THE Scheduler SHALL record execution time for each Task
2. THE Scheduler SHALL track total graph execution time from start to completion
3. THE Scheduler SHALL provide statistics on device utilization percentages
4. WHEN requested, THE Scheduler SHALL generate a timeline of task executions for visualization
5. THE Memory_Pool SHALL report fragmentation metrics

### Requirement 8: API Usability

**User Story:** As a developer, I want a clean and intuitive C++ API, so that I can easily integrate the scheduler into my projects.

#### Acceptance Criteria

1. THE Scheduler SHALL provide a fluent builder API for constructing task graphs
2. THE Task SHALL support lambda functions for defining computation logic
3. THE Scheduler SHALL support both synchronous and asynchronous graph execution modes
4. WHEN executing synchronously, THE Scheduler SHALL block until all tasks complete
5. WHEN executing asynchronously, THE Scheduler SHALL return a future that resolves when execution completes
