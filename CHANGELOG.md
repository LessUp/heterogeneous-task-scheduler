# Changelog

All notable changes to this project are documented in this file.

[![Keep a Changelog](https://img.shields.io/badge/Keep%20a%20Changelog-1.1.0-blue)](https://keepachangelog.com/en/1.1.0/)
[![Semantic Versioning](https://img.shields.io/badge/Semantic%20Versioning-2.0.0-blue)](https://semver.org/spec/v2.0.0.html)

> 📄 **Detailed changelogs** are available in the [`changelog/`](changelog/) directory.

---

## [Unreleased]

---

## [1.2.0] - 2026-04-16

### 🌟 Highlights

**Bilingual Documentation Release** — Complete documentation overhaul with full English and Chinese support, plus professional changelog structure.

### 📚 Documentation

#### English Documentation (10 documents)

| Document | Description |
|----------|-------------|
| [docs/en/installation.md](docs/en/installation.md) | Complete installation and configuration guide |
| [docs/en/quickstart.md](docs/en/quickstart.md) | 5-minute quick start tutorial |
| [docs/en/architecture.md](docs/en/architecture.md) | System architecture deep dive |
| [docs/en/api-reference.md](docs/en/api-reference.md) | Complete API reference |
| [docs/en/scheduling-policies.md](docs/en/scheduling-policies.md) | Scheduling policy guide |
| [docs/en/memory-management.md](docs/en/memory-management.md) | Memory pool and optimization |
| [docs/en/error-handling.md](docs/en/error-handling.md) | Error handling best practices |
| [docs/en/profiling.md](docs/en/profiling.md) | Performance profiling guide |
| [docs/en/examples.md](docs/en/examples.md) | Detailed example walkthroughs |
| [docs/README.md](docs/README.md) | Documentation landing page |

#### 中文文档 (10 篇)

| 文档 | 描述 |
|------|------|
| [docs/zh-CN/installation.md](docs/zh-CN/installation.md) | 完整安装和配置指南 |
| [docs/zh-CN/quickstart.md](docs/zh-CN/quickstart.md) | 5 分钟快速入门教程 |
| [docs/zh-CN/architecture.md](docs/zh-CN/architecture.md) | 系统架构深入解析 |
| [docs/zh-CN/api-reference.md](docs/zh-CN/api-reference.md) | 完整 API 参考文档 |
| [docs/zh-CN/scheduling-policies.md](docs/zh-CN/scheduling-policies.md) | 调度策略指南 |
| [docs/zh-CN/memory-management.md](docs/zh-CN/memory-management.md) | 内存池和优化指南 |
| [docs/zh-CN/error-handling.md](docs/zh-CN/error-handling.md) | 错误处理最佳实践 |
| [docs/zh-CN/profiling.md](docs/zh-CN/profiling.md) | 性能分析指南 |
| [docs/zh-CN/examples.md](docs/zh-CN/examples.md) | 详细示例教程 |
| [docs/README.md](docs/README.md) | 文档首页（双语）|

### 🔧 Project Structure

- Reorganized `docs/` with bilingual structure (`en/` and `zh-CN/`)
- Professionalized `changelog/` with templates and consistent formatting
- Enhanced `README.md` with table of contents and performance metrics
- Updated `README.zh-CN.md` with full Chinese translation

### 🆕 Added

- Bilingual documentation support (English/Chinese)
- 20 new documentation files across all topics
- Professional changelog templates for future entries
- Enhanced README with performance benchmarks
- Documentation index with cross-language links

---

## [1.1.0] - 2024-12-31

[![GitHub Release](https://img.shields.io/badge/Release-v1.1.0-blue)](https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.1.0)

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

[![GitHub Release](https://img.shields.io/badge/Release-v1.0.0-blue)](https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.0.0)

🎉 **Initial Release** — Core DAG scheduling framework for heterogeneous CPU/GPU task execution.

### ✨ Core Features

| Component | Description |
|-----------|-------------|
| **Task Graph (DAG)** | Directed acyclic graph with automatic cycle detection |
| **Dependency Manager** | Topological sort, dependency tracking, failure propagation |
| **GPU Memory Pool** | Buddy system allocator (reduces cudaMalloc overhead) |
| **CPU Thread Pool** | Parallel task execution |
| **CUDA Stream Manager** | Concurrent GPU kernel execution |
| **Device Scheduler** | Load-based CPU/GPU selection |

### 📦 Examples Included

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
| `advanced_features.cpp` | Events, barriers, retry policies |

---

## 🏗️ Infrastructure

### 2026-03-13 — CI: CPU-Safe Format Check

> 📄 [Full Details](changelog/2026-03-13_workflow-cpu-safe-ci.md)

**Problem**: CUDA container builds failed on GitHub Hosted Runners (no GPU).

**Solution**: Migrated to CPU-safe static analysis (clang-format, markdownlint, yamllint).

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
| [Unreleased] | — | — | — |
| [1.2.0] | 2026-04-16 | Minor | Bilingual documentation overhaul |
| [1.1.0] | 2024-12-31 | Minor | Synchronization, events, retry policies |
| [1.0.0] | 2024-12-31 | Major | Initial release |

---

[Unreleased]: https://github.com/LessUp/heterogeneous-task-scheduler/compare/v1.2.0...HEAD
[1.2.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.2.0
[1.1.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.1.0
[1.0.0]: https://github.com/LessUp/heterogeneous-task-scheduler/releases/tag/v1.0.0
