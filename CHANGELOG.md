# Changelog

All notable changes to this project are documented in this file.

[![Keep a Changelog](https://img.shields.io/badge/Keep%20a%20Changelog-1.1.0-informational)](https://keepachangelog.com/en/1.1.0/)
[![Semantic Versioning](https://img.shields.io/badge/Semantic%20Versioning-2.0.0-blue)](https://semver.org/spec/v2.0.0.html)

> 💡 **Tip**: Individual change logs are available in [`changelog/`](changelog/) directory.

---

## [Unreleased]

### Added

- Comprehensive documentation overhaul with bilingual (English/Chinese) support
- Status badges in README (License, C++17, CUDA)
- Architecture diagram in README
- Core components documentation section

### Fixed

- `profiler.hpp`: Undeclared variable `cumulative_memory` in `generate_summary()` method
- Missing `#include <string>` in 6 header files
- Missing `#include <vector>` in 2 header files
- Missing `#include <mutex>`, `#include <atomic>` in `scheduler.hpp`
- Missing `#include <memory>` in `retry_policy.hpp`

### Changed

- Restructured README with improved code examples
- Reformatted CHANGELOG following Keep a Changelog 1.1.0
- Modernized CONTRIBUTING.md with bilingual support
- Simplified CODE_OF_CONDUCT.md
- Enhanced SECURITY.md with code examples

---

## [1.1.0] - 2024-12-31

[![GitHub Release](https://img.shields.io/github/release/LessUp/heterogeneous-task-scheduler.svg?label=Release)](https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.1.0)

### ✨ New Features

#### Synchronization Primitives

| Class | Description |
|-------|-------------|
| `TaskFuture<T>` | Type-safe asynchronous result retrieval |
| `TaskBarrier` | Barrier synchronization between task phases |
| `BarrierGroup` | Container for managing multiple barriers |

#### Retry Policies

| Class | Use Case |
|-------|----------|
| `RetryPolicy` | Abstract base for custom retry strategies |
| `NoRetryPolicy` | No retry (default) |
| `FixedRetryPolicy` | Fixed delay between attempts |
| `ExponentialBackoffPolicy` | Exponentially increasing delay |
| `JitteredBackoffPolicy` | Randomized backoff (avoid thundering herd) |
| `ConditionalRetryPolicy` | Retry only specific error types |
| `RetryPolicyFactory` | Factory methods for creating policies |

#### Event System

| Class | Description |
|-------|-------------|
| `EventSystem` | Pub/sub for scheduler lifecycle events |
| `Event` | Event container with timestamp and context |
| `ScopedSubscription` | RAII subscription management |

**Supported Events**: `TaskCreated`, `TaskStarted`, `TaskCompleted`, `TaskFailed`, `TaskCancelled`, `TaskRetrying`, `GraphStarted`, `GraphCompleted`, `MemoryAllocated`, `MemoryFreed`, `StreamAcquired`, `StreamReleased`

#### Resource Management

| Class | Description |
|-------|-------------|
| `ResourceLimiter` | Configurable resource constraints |
| `Semaphore` | Counting semaphore |
| `SemaphoreGuard` | RAII semaphore wrapper |
| `ResourceSlotGuard` | RAII device slot management |

**Limit Types**: CPU task concurrency, GPU task concurrency, total memory, GPU memory, total tasks

### 📦 Examples

- `advanced_features.cpp` — Events, barriers, retry policies demonstration

### ✅ Tests

| Test File | Coverage |
|-----------|----------|
| `test_task_future.cpp` | `TaskFuture<T>` unit tests |
| `test_task_barrier.cpp` | `TaskBarrier` unit tests |
| `test_event_system.cpp` | `EventSystem` unit tests |
| `test_retry_policy.cpp` | All retry policy tests |
| `test_resource_limiter.cpp` | `ResourceLimiter` unit tests |

---

## [1.0.0] - 2024-12-31

[![GitHub Release](https://img.shields.io/github/release/LessUp/heterogeneous-task-scheduler.svg?label=Release)](https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.0.0)

🎉 **Initial Release** — Core DAG scheduling framework for heterogeneous CPU/GPU task execution.

### ✨ Features

#### Core Engine

| Component | Description |
|-----------|-------------|
| **Task Graph (DAG)** | Directed acyclic graph with automatic cycle detection |
| **Dependency Manager** | Topological sort, dependency tracking, failure propagation |
| **GPU Memory Pool** | Buddy system allocator (reduces cudaMalloc overhead) |
| **CPU Thread Pool** | Parallel task execution |
| **CUDA Stream Manager** | Concurrent GPU kernel execution |
| **Device Scheduler** | Load-based CPU/GPU selection |

#### Public API

| Class | Purpose |
|-------|---------|
| `Scheduler` | Main entry point for graph execution |
| `TaskGraph` | Build and validate task dependencies |
| `Task` | Define CPU/GPU task functions |
| `TaskContext` | Memory allocation, I/O, error reporting |
| `TaskBuilder` | Fluent task construction API |
| `TaskGroup` | Batch operations on task sets |

#### Scheduling Policies

| Policy | Behavior |
|--------|----------|
| `DefaultSchedulingPolicy` | Load-based device selection |
| `GpuFirstPolicy` | Prefer GPU when available |
| `CpuFirstPolicy` | Prefer CPU execution |
| `RoundRobinPolicy` | Alternate between CPU/GPU |
| `ShortestJobFirstPolicy` | Priority-based ordering |

#### Observability

| Component | Output |
|-----------|--------|
| `Profiler` | Timing stats, parallelism metrics, memory usage |
| `Logger` | Thread-safe logging with levels |
| Timeline JSON | Chrome tracing compatible timeline |

#### Utilities

| Class | Description |
|-------|-------------|
| `GraphSerializer` | Export to JSON / Graphviz DOT |
| `CudaUtils` | Device info, memory stats |
| `DeviceMemory<T>` | RAII device memory |
| `PinnedMemory<T>` | RAII pinned host memory |
| `ScopedDevice` | RAII device selection |

#### Error Handling

- Configurable error callbacks per-task
- Automatic failure propagation to dependents
- Graceful shutdown waiting for in-flight tasks

### 📦 Examples

| File | Demonstrates |
|------|--------------|
| `simple_dag.cpp` | Basic DAG construction and execution |
| `parallel_pipeline.cpp` | Parallel processing pattern |
| `error_handling.cpp` | Error callback and propagation |
| `fluent_api.cpp` | TaskBuilder patterns |
| `task_groups.cpp` | TaskGroup batch operations |
| `profiling.cpp` | Performance profiling |
| `scheduling_policies.cpp` | Policy comparison |
| `graph_visualization.cpp` | DOT/JSON export |
| `gpu_computation.cpp` | CUDA kernel integration |

### 📚 Documentation

- API documentation in header files (Doxygen-style)
- Comprehensive README with examples
- Working example programs

---

## 🏗️ Infrastructure

### 2026-03-13 — CI: CPU-Safe Format Check

> 📄 [Full Details](changelog/2026-03-13_workflow-cpu-safe-ci.md)

**Problem**: CUDA container builds failed on GitHub Hosted Runners (no GPU).

**Solution**: Replaced with `clang-format` static analysis.

| Change | Details |
|--------|---------|
| Removed | `nvidia/cuda` container build |
| Added | `jidicula/clang-format-action@v4` |
| Restored | `push`, `pull_request`, `workflow_dispatch` triggers |

### 2026-03-10 — CI: Workflow Standardization

> 📄 [Full Details](changelog/2026-03-10_workflow-deep-standardization.md)

| Change | Details |
|--------|---------|
| Permissions | Unified `contents: read` |
| Concurrency | Cancel duplicate runs |
| Pages | Added `configure-pages@v5` |
| Filtering | Path-based trigger reduction |

### 2026-03-09 — CI: Initial Workflow

> 📄 [Full Details](changelog/2026-03-09_workflow-optimization.md)

| Change | Details |
|--------|---------|
| Added | `.github/workflows/ci.yml` |
| Triggers | `push`, `pull_request`, `workflow_dispatch` |
| Jobs | Build validation, format check |

---

## 📊 Version History

| Version | Date | Type | Summary |
|---------|------|------|---------|
| [Unreleased] | — | — | Docs, fixes |
| [1.1.0] | 2024-12-31 | Minor | Synchronization, events, retry policies |
| [1.0.0] | 2024-12-31 | Major | Initial release |

---

[Unreleased]: https://github.com/LessUp/heterogeneous-task-scheduler/compare/v1.1.0...HEAD
[1.1.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.1.0
[1.0.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.0.0
