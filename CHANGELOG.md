# Changelog

All notable changes to this project will be documented in this file.

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
