# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.1.0] - 2024-12-31

### Added

#### Synchronization
- `TaskFuture<T>` - Type-safe result retrieval from async tasks
- `TaskBarrier` - Synchronization points between task phases
- `BarrierGroup` - Multiple barrier management

#### Retry Policies
- `RetryPolicy` - Abstract base class for retry strategies
- `NoRetryPolicy` - No retry (default behavior)
- `FixedRetryPolicy` - Fixed delay between retry attempts
- `ExponentialBackoffPolicy` - Exponential delay growth
- `JitteredBackoffPolicy` - Randomized exponential backoff
- `ConditionalRetryPolicy` - Error-type based retry decisions
- `RetryPolicyFactory` - Convenient factory methods for policy creation

#### Event System
- `EventSystem` - Publish/subscribe system for scheduler events
- `Event` - Event data structure with timestamps and task info
- `ScopedSubscription` - RAII-based subscription management
- Event types: `TaskCreated`, `TaskStarted`, `TaskCompleted`, `TaskFailed`, `TaskCancelled`, `TaskRetrying`, `GraphStarted`, `GraphCompleted`, `MemoryAllocated`, `MemoryFreed`, `StreamAcquired`, `StreamReleased`

#### Resource Management
- `ResourceLimiter` - Concurrent resource usage limits
- `Semaphore` - Counting semaphore implementation
- `SemaphoreGuard` - RAII semaphore acquisition
- `ResourceSlotGuard` - RAII resource slot management
- Configurable limits: CPU/GPU task concurrency, memory bounds, total tasks

### Examples
- `advanced_features.cpp` - Demonstrates events, barriers, and retry policies

### Tests
- `test_task_future.cpp` - TaskFuture unit tests
- `test_task_barrier.cpp` - TaskBarrier unit tests
- `test_event_system.cpp` - EventSystem unit tests
- `test_retry_policy.cpp` - RetryPolicy unit tests
- `test_resource_limiter.cpp` - ResourceLimiter unit tests

## [1.0.0] - 2024-12-31

### Added

#### Core Features
- **Task Graph (DAG)** - Directed acyclic graph with automatic cycle detection
- **Dependency Management** - Topological sorting and dependency tracking
- **GPU Memory Pool** - Buddy system allocator for efficient GPU memory management
- **Asynchronous Execution** - CUDA streams for non-blocking GPU operations
- **CPU Thread Pool** - Parallel task execution on CPU
- **Heterogeneous Scheduling** - Load-based device selection between CPU and GPU

#### API Components
- `Scheduler` - Main scheduler class with sync/async execution modes
- `TaskGraph` - DAG construction, validation, and traversal
- `Task` - Task definition with CPU/GPU function support
- `TaskContext` - Memory allocation and data passing context
- `TaskBuilder` - Fluent API for ergonomic task creation
- `TaskGroup` - Batch task management and operations

#### Scheduling Policies
- `DefaultSchedulingPolicy` - Load-based device selection
- `GpuFirstPolicy` - GPU-preferred scheduling
- `CpuFirstPolicy` - CPU-preferred scheduling
- `RoundRobinPolicy` - Alternating device selection
- `ShortestJobFirstPolicy` - Priority-based scheduling

#### Monitoring & Logging
- `Profiler` - Performance analysis with timing statistics
- `Logger` - Thread-safe logging with configurable levels
- Execution timeline generation (JSON format)
- Memory usage statistics and fragmentation metrics

#### Utilities
- `GraphSerializer` - Export task graphs to JSON and DOT formats
- `CudaUtils` - CUDA device management and information
- `DeviceMemory<T>` - RAII device memory wrapper
- `PinnedMemory<T>` - RAII pinned host memory wrapper
- `ScopedDevice` - RAII CUDA device selection

#### Error Handling
- Error callbacks for task failure notification
- Failure propagation to dependent tasks
- Graceful shutdown with in-flight task completion

### Examples
- `simple_dag.cpp` - Basic DAG execution demo
- `parallel_pipeline.cpp` - Parallel processing pipeline
- `error_handling.cpp` - Error propagation demonstration
- `fluent_api.cpp` - TaskBuilder usage patterns
- `task_groups.cpp` - TaskGroup management demo
- `profiling.cpp` - Performance profiler usage
- `scheduling_policies.cpp` - Policy comparison examples
- `graph_visualization.cpp` - Graph export to DOT/JSON
- `gpu_computation.cpp` - CUDA kernel execution example

### Documentation
- Comprehensive README with usage examples
- API documentation in header files
- Example programs for all major features

---

## Version History

| Version | Date | Highlights |
|---------|------|------------|
| [1.1.0] | 2024-12-31 | Events, barriers, retry policies, resource limits |
| [1.0.0] | 2024-12-31 | Initial release with core DAG scheduling |

[1.1.0]: https://github.com/LessUp/heterogeneous-task-scheduler/compare/v1.0.0...v1.1.0
[1.0.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.0.0
