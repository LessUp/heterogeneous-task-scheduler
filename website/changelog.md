# Changelog

All notable changes to HTS will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/), and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.2.0] - 2026-04-15

### Added

- **Load-Based Scheduling Policy**: New `LoadBasedPolicy` that dynamically selects tasks based on current CPU/GPU utilization
- **Task Tags**: Support for adding metadata tags to tasks for easier querying and organization
- **TaskBuilder API**: Fluent builder API for cleaner task creation and configuration
- **Execution Engine Configuration**: Configurable CPU thread count and GPU stream count
- **Profiling Enhancements**: Export execution timeline to Chrome tracing format
- **Task Timeout**: Configurable timeout for long-running tasks
- **Error Statistics**: Comprehensive error tracking and reporting

### Changed

- **Improved Memory Pool**: Enhanced buddy system allocator with automatic defragmentation
- **Better Error Messages**: More descriptive error messages with context information
- **Performance Optimizations**: Reduced scheduling overhead by 30%
- **Documentation**: Comprehensive website with guides, API reference, and examples

### Fixed

- **Cycle Detection**: Fixed false positive in cycle detection for complex graphs
- **Memory Leaks**: Fixed memory leaks in error paths
- **Thread Safety**: Improved thread safety in task status updates
- **Build System**: Fixed CUDA architecture detection for multi-GPU systems

---

## [1.1.0] - 2025-12-10

### Added

- **Retry Policies**: Automatic retry with configurable backoff strategies
- **Failure Propagation Control**: Customizable failure propagation behavior
- **Task Fallbacks**: Define fallback functions for task failures
- **Graph Validation**: Comprehensive validation before execution
- **Critical Path Analysis**: Identify the longest dependency chain
- **Orphan Task Detection**: Find tasks with no dependencies

### Changed

- **Scheduler Architecture**: Refactored scheduler for better modularity
- **Memory Pool Configuration**: More intuitive configuration API
- **CUDA Stream Management**: Improved stream reuse and priority handling
- **CPU Thread Pool**: Work-stealing support for better load balancing

### Fixed

- **Deadlock Prevention**: Fixed potential deadlock in dependency resolution
- **GPU Memory Alignment**: Fixed alignment issues for certain kernel launches
- **Build Errors**: Fixed compilation with GCC 13 and CUDA 12

---

## [1.0.0] - 2025-06-01

### Added

- **Core Scheduler**: Main scheduling engine with DAG-based task management
- **TaskGraph API**: Complete DAG management with dependency tracking
- **Task and TaskContext**: Task abstraction with execution context
- **Scheduling Policies**:
  - GPU Priority Policy
  - CPU Priority Policy
  - Round Robin Policy
- **Memory Pool**: Buddy system allocator for GPU memory
  - O(log n) allocation
  - Automatic defragmentation
  - Configurable pool size
- **Stream Manager**: CUDA stream management for concurrent GPU execution
- **Execution Engine**: CPU thread pool and GPU stream coordination
- **Profiler**: Built-in performance monitoring and statistics
  - Task execution times
  - Device utilization metrics
  - Memory allocation stats
  - JSON export
- **Error Handling**: Comprehensive error codes and exception safety
- **Logging**: Structured logging with configurable verbosity

### Documentation

- Getting started guide
- API reference for core components
- Architecture overview
- Basic examples

---

## [Unreleased]

### Planned

- **Event System**: Task lifecycle events for monitoring
- **Dynamic DAG Updates**: Modify task graph during execution
- **Remote Execution**: Distribute tasks across multiple GPUs/nodes
- **Python Bindings**: Python API for rapid prototyping
- **Visual DAG Editor**: GUI for creating and visualizing task graphs
- **Benchmarking Suite**: Comprehensive performance benchmarks

---

## Version Guide

- **1.2.x**: Current stable release
- **1.1.x**: Maintenance mode (bug fixes only)
- **1.0.x**: End of life

## Upgrade Guides

### Upgrading from 1.1.x to 1.2.0

**Breaking Changes:**
- `Scheduler::init()` now requires `ExecutionEngineConfig` as second parameter (optional, defaults provided)
- `Task::set_name()` removed (use TaskBuilder instead)

**Migration Steps:**

```cpp
// Old (1.1.x)
scheduler.init(&graph);

// New (1.2.0)
scheduler.init(&graph);  // Still works, uses defaults

// Or with configuration
ExecutionEngineConfig config;
config.cpu_thread_count = 8;
scheduler.init(&graph, config);
```

### Upgrading from 1.0.x to 1.1.0

**Breaking Changes:**
- None (fully backward compatible)

**New Features to Adopt:**

```cpp
// Add retry policies for better fault tolerance
task->set_retry_policy(RetryPolicy{
    .max_retries = 3,
    .backoff_ms = 100
});

// Add fallback functions
task->set_fallback([](TaskContext& ctx) {
    // Fallback implementation
});
```

---

## Contributing

We welcome contributions! Please see our [Contributing Guide](/contributing) for details.

### Release Process

1. Update `CHANGELOG.md` with all changes
2. Update version numbers in documentation
3. Create git tag: `git tag -a v1.2.0 -m "Release v1.2.0"`
4. Push tag: `git push origin v1.2.0`
5. Create GitHub release with changelog

---

**Links:**

- [GitHub Releases](https://github.com/LessUp/heterogeneous-task-scheduler/releases)
- [Contributing Guide](/contributing)
- [Migration Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
