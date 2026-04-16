# Requirements Document: Heterogeneous Task Scheduler

[![Spec](https://img.shields.io/badge/Spec-Requirements-orange)]()
[![Version](https://img.shields.io/badge/Version-1.1.0-green)]()

> 异构任务图调度器需求文档

## Introduction

Heterogeneous Task Scheduler (HTS) 是一个 C++ 框架，允许用户定义由多个任务组成的有向无环图 (DAG)，框架自动将任务分配给 CPU 或 GPU 执行。类似于 TensorFlow/PyTorch 底层调度逻辑或游戏引擎渲染图。

---

## Glossary

| Term | Definition |
|------|------------|
| **Scheduler** | 核心调度器，管理任务图执行和资源分配 |
| **Task** | 可在 CPU 或 GPU 上执行的计算单元 |
| **TaskGraph** | 任务和依赖关系组成的有向无环图 |
| **MemoryPool** | GPU 显存池，管理内存分配和回收 |
| **ExecutionEngine** | 执行引擎，在指定设备上运行任务 |
| **DependencyManager** | 依赖管理器，跟踪任务间依赖关系 |
| **Stream** | CUDA 流，用于异步执行 GPU 任务 |

---

## Requirements

### REQ-1: Task Definition and Graph Construction

**User Story:** As a developer, I want to define tasks and dependencies as a DAG.

| ID | Acceptance Criteria |
|----|---------------------|
| 1.1 | TaskGraph SHALL allow creating tasks with unique IDs and execution targets (CPU/GPU) |
| 1.2 | WHEN adding dependency A→B, DependencyManager SHALL record that B depends on A |
| 1.3 | WHEN adding dependency that creates cycle, TaskGraph SHALL reject and return false |
| 1.4 | Task SHALL support user-defined CPU and GPU computation functions |
| 1.5 | TaskGraph validation SHALL confirm all dependencies form valid DAG |

### REQ-2: Dependency Management and Execution Order

**User Story:** As a developer, I want automatic dependency management for correct execution order.

| ID | Acceptance Criteria |
|----|---------------------|
| 2.1 | WHEN Task A completes, DependencyManager SHALL notify all dependents |
| 2.2 | WHEN all dependencies satisfied, Scheduler SHALL add Task to ready queue |
| 2.3 | Scheduler SHALL NEVER execute Task before all predecessors complete |
| 2.4 | WHEN multiple tasks ready, Scheduler SHALL support concurrent execution |
| 2.5 | IF Task fails, Scheduler SHALL mark all dependents as blocked |

### REQ-3: Memory Pool Management

**User Story:** As a developer, I want efficient GPU memory management.

| ID | Acceptance Criteria |
|----|---------------------|
| 3.1 | MemoryPool SHALL pre-allocate configurable GPU memory at initialization |
| 3.2 | Allocation SHALL return block from pool without cudaMalloc |
| 3.3 | Free SHALL return block to pool without cudaFree |
| 3.4 | IF insufficient memory, pool SHALL expand or block (never crash) |
| 3.5 | MemoryPool SHALL track usage statistics |

### REQ-4: Asynchronous Concurrent Execution

**User Story:** As a developer, I want concurrent CPU/GPU execution.

| ID | Acceptance Criteria |
|----|---------------------|
| 4.1 | ExecutionEngine SHALL use CUDA Streams for async GPU execution |
| 4.2 | WHILE GPU task executing, CPU tasks SHALL run concurrently |
| 4.3 | Independent GPU tasks SHALL execute on separate streams |
| 4.4 | Scheduler SHALL overlap CPU preparation with GPU execution |
| 4.5 | CPU/GPU sync SHALL use CUDA events efficiently |

### REQ-5: Device Assignment and Load Balancing

**User Story:** As a developer, I want intelligent device assignment.

| ID | Acceptance Criteria |
|----|---------------------|
| 5.1 | Task SHALL allow specifying preferred device (CPU, GPU, or Any) |
| 5.2 | WHEN "Any", Scheduler SHALL assign to least loaded device |
| 5.3 | Scheduler SHALL track device utilization for load balancing |
| 5.4 | WHEN GPU fully utilized, tasks SHALL queue without blocking CPU |

### REQ-6: Error Handling and Recovery

**User Story:** As a developer, I want robust error handling.

| ID | Acceptance Criteria |
|----|---------------------|
| 6.1 | IF CUDA operation fails, error SHALL be captured and propagated |
| 6.2 | WHEN Task fails, detailed error info SHALL be provided |
| 6.3 | User-defined error callbacks SHALL be supported |
| 6.4 | IF allocation fails, error SHALL be returned (no crash) |
| 6.5 | Graceful shutdown SHALL complete in-flight tasks |

### REQ-7: Performance Monitoring

**User Story:** As a developer, I want performance monitoring.

| ID | Acceptance Criteria |
|----|---------------------|
| 7.1 | Scheduler SHALL record execution time for each Task |
| 7.2 | Scheduler SHALL track total graph execution time |
| 7.3 | Device utilization statistics SHALL be provided |
| 7.4 | Timeline generation for visualization SHALL be supported |
| 7.5 | Memory fragmentation metrics SHALL be reported |

### REQ-8: API Usability

**User Story:** As a developer, I want a clean C++ API.

| ID | Acceptance Criteria |
|----|---------------------|
| 8.1 | Fluent builder API SHALL be provided for graph construction |
| 8.2 | Lambda functions SHALL be supported for task logic |
| 8.3 | Both sync and async execution modes SHALL be supported |
| 8.4 | Sync execute() SHALL block until all tasks complete |
| 8.5 | Async execute_async() SHALL return future |

---

## Requirement Traceability

| Requirement | Properties | Tests |
|-------------|------------|-------|
| REQ-1 | P1, P2, P3 | test_task.cpp, test_task_graph.cpp |
| REQ-2 | P4, P5 | test_dependency_manager.cpp, test_integration.cpp |
| REQ-3 | P6, P7 | test_memory_pool.cu |
| REQ-4 | P8 | test_stream_manager.cu, test_execution_engine.cu |
| REQ-5 | P9 | test_scheduling_policy.cpp |
| REQ-6 | P10, P11 | test_error_handling.cpp, test_integration.cpp |
| REQ-7 | P12, P13 | test_profiler.cpp |
| REQ-8 | P14 | test_integration.cpp |

---

## Related Documents

- [Design](design.md)
- [Tasks](tasks.md)
