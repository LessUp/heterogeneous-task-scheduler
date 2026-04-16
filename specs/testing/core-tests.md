# Test Specifications: Core Components

[![Spec](https://img.shields.io/badge/Spec-Testing-purple)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

> Test specifications for core HTS components.

---

## Test Framework

| Framework | Purpose |
|-----------|---------|
| **Google Test** | Unit and integration testing |
| **RapidCheck** | Property-based testing |

---

## Unit Tests

### Task Tests

| Test File | Tests |
|-----------|-------|
| `test_task.cpp` | Task creation, state transitions, device preference, function execution |

### TaskGraph Tests

| Test File | Tests |
|-----------|-------|
| `test_task_graph.cpp` | DAG construction, cycle detection, topological sort, dependency tracking |

### TaskBuilder Tests

| Test File | Tests |
|-----------|-------|
| `test_task_builder.cpp` | Fluent API, task configuration, builder patterns |

### TaskGroup Tests

| Test File | Tests |
|-----------|-------|
| `test_task_group.cpp` | Batch operations, group dependencies, group execution |

### DependencyManager Tests

| Test File | Tests |
|-----------|-------|
| `test_dependency_manager.cpp` | Ready queue, completion notification, failure propagation |

---

## Memory Tests

| Test File | Tests |
|-----------|-------|
| `test_memory_pool.cu` | Buddy allocator, block splitting, coalescing, stats tracking |

---

## Execution Tests

| Test File | Tests |
|-----------|-------|
| `test_stream_manager.cu` | CUDA stream acquisition/release, event management |
| `test_scheduling_policy.cpp` | Policy selection, load balancing, device assignment |

---

## Concurrency Tests

| Test File | Tests |
|-----------|-------|
| `test_execution_engine.cu` | CPU/GPU concurrent execution, async operations |
| `test_integration.cpp` | End-to-end execution, CPU-GPU pipeline |

---

## Error Tests

| Test File | Tests |
|-----------|-------|
| `test_error_handling.cpp` | Error callbacks, failure propagation, graceful shutdown |
| `test_retry_policy.cpp` | Retry policies (fixed, exponential, jittered, circuit breaker) |

---

## Synchronization Tests

| Test File | Tests |
|-----------|-------|
| `test_task_future.cpp` | Async result retrieval, type safety |
| `test_task_barrier.cpp` | Barrier synchronization, phase coordination |

---

## Infrastructure Tests

| Test File | Tests |
|-----------|-------|
| `test_profiler.cpp` | Execution stats, timeline generation, metrics accuracy |
| `test_event_system.cpp` | Pub/sub, scoped subscriptions, event filtering |
| `test_resource_limiter.cpp` | Resource constraints, semaphore, slot guards |
| `test_graph_serializer.cpp` | DOT/JSON export, graph import |
| `test_logger.cpp` | Logging levels, formatting, output destinations |
| `test_cuda_utils.cpp` | CUDA error handling, device queries |

---

## Test Coverage Summary

| Component | Test Count | Status |
|-----------|------------|--------|
| Core | 10 test files | ✅ |
| Memory | 1 test file | ✅ |
| Execution | 3 test files | ✅ |
| Error Handling | 2 test files | ✅ |
| Synchronization | 2 test files | ✅ |
| Infrastructure | 6 test files | ✅ |
| **Total** | **18 test files** | ✅ |

---

## Related Documents

- [Product Requirements](../product/001-heterogeneous-task-scheduler.md)
- [Core Architecture](../rfc/001-core-architecture.md)
