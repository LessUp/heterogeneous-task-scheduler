# Changelog

All notable changes to this project will be documented in this file.

## [1.1.0] - 2024-12-31

### Added

#### Synchronization
- `TaskFuture<T>` - Type-safe result retrieval from tasks
- `TaskBarrier` - Synchronization points between task phases
- `BarrierGroup` - Multiple barrier management

#### Retry Policies
- `RetryPolicy` - Base class for retry strategies
- `NoRetryPolicy` - No retry (default)
- `FixedRetryPolicy` - Fixed delay between retries
- `ExponentialBackoffPolicy` - Exponential delay growth
- `JitteredBackoffPolicy` - Randomized exponential backoff
- `ConditionalRetryPolicy` - Error-type based retry decisions
- `RetryPolicyFactory` - Convenient policy creation

#### Event System
- `EventSystem` - Pub/sub for scheduler events
- `Event` - Event data structure with timestamps
- `ScopedSubscription` - RAII subscription management
- Events: TaskCreated, TaskStarted, TaskCompleted, TaskFailed, etc.

#### Resource Management
- `ResourceLimiter` - Concurrent resource usage limits
- `Semaphore` - Counting semaphore implementation
- `SemaphoreGuard` - RAII semaphore acquisition
- `ResourceSlotGuard` - RAII resource slot management
- CPU/GPU task limits, memory limits, total task limits

### Examples
- `advanced_features` - Demonstrates events, barriers, retry policies

### Tests
- `test_task_future.cpp` - TaskFuture tests
- `test_task_barrier.cpp` - TaskBarrier tests
- `test_event_system.cpp` - EventSystem tests
- `test_retry_policy.cpp` - RetryPolicy tests
- `test_resource_limiter.cpp` - ResourceLimiter tests

## [1.0.0] - 2024-12-31

### Added

#### Core Features
- Task graph (DAG) with automatic cycle detection
- Dependency management with topological sorting
- GPU memory pool using buddy system allocator
- Asynchronous execution with CUDA streams
- CPU thread pool for parallel task execution
- Load-based device selection for heterogeneous scheduling

#### API
- `Scheduler` - Main scheduler class with sync/async execution
- `TaskGraph` - DAG construction and validation
- `Task` - Task definition with CPU/GPU functions
- `TaskContext` - Memory allocation and data passing
- `TaskBuilder` - Fluent API for task creation
- `TaskGroup` - Batch task management

#### Scheduling
- `DefaultSchedulingPolicy` - Load-based scheduling
- `GpuFirstPolicy` - GPU-preferred scheduling
- `CpuFirstPolicy` - CPU-preferred scheduling
- `RoundRobinPolicy` - Alternating device selection
- `ShortestJobFirstPolicy` - Priority-based scheduling

#### Monitoring
- `Profiler` - Performance analysis and reporting
- `Logger` - Thread-safe logging with levels
- Execution timeline generation (JSON)
- Memory usage statistics

#### Utilities
- `GraphSerializer` - JSON and DOT export
- `CudaUtils` - CUDA device management
- `DeviceMemory` - RAII device memory wrapper
- `PinnedMemory` - RAII pinned memory wrapper
- `ScopedDevice` - RAII device selection

#### Error Handling
- Error callbacks for task failures
- Failure propagation to dependent tasks
- Graceful shutdown with in-flight task completion

### Examples
- `simple_dag` - Basic DAG execution
- `parallel_pipeline` - Parallel processing pipeline
- `error_handling` - Error propagation demo
- `fluent_api` - TaskBuilder usage
- `task_groups` - TaskGroup management
- `profiling` - Performance profiler demo
- `scheduling_policies` - Policy comparison
- `graph_visualization` - Graph export
- `gpu_computation` - CUDA kernel execution

### Documentation
- Comprehensive README with usage examples
- API documentation in headers
- Example programs for all features
