# Specs: Core Architecture

## Product Requirements

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

## Test Specifications

### Test Framework

| Framework | Purpose |
|-----------|---------|
| **Google Test** | Unit and integration testing |
| **RapidCheck** | Property-based testing |

### Unit Tests

| Test File | Tests |
|-----------|-------|
| `test_task.cpp` | Task creation, state transitions, device preference, function execution |
| `test_task_graph.cpp` | DAG construction, cycle detection, topological sort, dependency tracking |
| `test_task_builder.cpp` | Fluent API, task configuration, builder patterns |
| `test_task_group.cpp` | Batch operations, group dependencies, group execution |
| `test_dependency_manager.cpp` | Ready queue, completion notification, failure propagation |

### Memory Tests

| Test File | Tests |
|-----------|-------|
| `test_memory_pool.cu` | Buddy allocator, block splitting, coalescing, stats tracking |

### Execution Tests

| Test File | Tests |
|-----------|-------|
| `test_stream_manager.cu` | CUDA stream acquisition/release, event management |
| `test_scheduling_policy.cpp` | Policy selection, load balancing, device assignment |

### Concurrency Tests

| Test File | Tests |
|-----------|-------|
| `test_execution_engine.cu` | CPU/GPU concurrent execution, async operations |
| `test_integration.cpp` | End-to-end execution, CPU-GPU pipeline |

### Error Tests

| Test File | Tests |
|-----------|-------|
| `test_error_handling.cpp` | Error callbacks, failure propagation, graceful shutdown |
| `test_retry_policy.cpp` | Retry policies (fixed, exponential, jittered, circuit breaker) |

### Synchronization Tests

| Test File | Tests |
|-----------|-------|
| `test_task_future.cpp` | Async result retrieval, type safety |
| `test_task_barrier.cpp` | Barrier synchronization, phase coordination |

### Infrastructure Tests

| Test File | Tests |
|-----------|-------|
| `test_profiler.cpp` | Execution stats, timeline generation, metrics accuracy |
| `test_event_system.cpp` | Pub/sub, scoped subscriptions, event filtering |
| `test_resource_limiter.cpp` | Resource constraints, semaphore, slot guards |
| `test_graph_serializer.cpp` | DOT/JSON export, graph import |
| `test_logger.cpp` | Logging levels, formatting, output destinations |
| `test_cuda_utils.cpp` | CUDA error handling, device queries |
