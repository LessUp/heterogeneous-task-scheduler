# Product Requirements: Heterogeneous Task Scheduler

[![Spec](https://img.shields.io/badge/Spec-Requirements-orange)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

> Requirements specification for the Heterogeneous Task Scheduler (HTS) framework.

---

## Introduction

Heterogeneous Task Scheduler (HTS) is a C++ framework that allows users to define a Directed Acyclic Graph (DAG) of tasks, where the framework automatically assigns tasks to CPU or GPU for execution. Similar to low-level scheduling logic in TensorFlow/PyTorch or rendering pipelines in game engines.

---

## Glossary

| Term | Definition |
|------|------------|
| **Scheduler** | Core scheduler managing task graph execution and resource allocation |
| **Task** | A unit of computation executable on CPU or GPU |
| **TaskGraph** | A DAG composed of tasks and their dependencies |
| **MemoryPool** | GPU memory pool managing allocation and deallocation |
| **ExecutionEngine** | Execution engine running tasks on specified devices |
| **DependencyManager** | Dependency manager tracking inter-task dependencies |
| **Stream** | CUDA stream for asynchronous GPU task execution |

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

| Requirement | Design RFC | Test Spec |
|-------------|------------|-----------|
| REQ-1 | [RFC-001: Core Architecture](../rfc/001-core-architecture.md) | [Core Tests](../testing/core-tests.md) |
| REQ-2 | [RFC-001: Core Architecture](../rfc/001-core-architecture.md) | [Core Tests](../testing/core-tests.md) |
| REQ-3 | [RFC-002: Memory Pool](../rfc/002-memory-pool.md) | [Memory Tests](../testing/memory-tests.md) |
| REQ-4 | [RFC-003: Execution Engine](../rfc/003-execution-engine.md) | [Concurrency Tests](../testing/concurrency-tests.md) |
| REQ-5 | [RFC-001: Core Architecture](../rfc/001-core-architecture.md) | [Scheduling Tests](../testing/scheduling-tests.md) |
| REQ-6 | [RFC-004: Error Handling](../rfc/004-error-handling.md) | [Error Tests](../testing/error-tests.md) |
| REQ-7 | [RFC-005: Profiling](../rfc/005-profiling.md) | [Profiler Tests](../testing/profiler-tests.md) |
| REQ-8 | [RFC-001: Core Architecture](../rfc/001-core-architecture.md) | [Integration Tests](../testing/integration-tests.md) |

---

## Related Documents

- [Architecture Design](../rfc/001-core-architecture.md)
- [Implementation Tasks](tasks-core.md)
- [API Reference](../../docs/en/api-reference.md)
