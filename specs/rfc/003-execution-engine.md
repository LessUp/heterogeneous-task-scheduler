# RFC-003: Execution Engine

[![Spec](https://img.shields.io/badge/Spec-RFC-blue)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

> Technical design for the async execution engine with CPU thread pool and CUDA stream management.

---

## Overview

The Execution Engine provides concurrent execution capabilities for CPU and GPU tasks, using a thread pool for CPU parallelism and CUDA streams for GPU asynchronous execution.

---

## Architecture

### CPU Thread Pool

- Configurable thread count (default: 4)
- Task queue with work stealing
- Load tracking via `get_cpu_load()`

### CUDA Stream Manager

- Configurable stream count (default: 4)
- Stream acquisition and release
- Event-based synchronization
- Timeline event recording

### Resource Limiter

Configurable resource constraints:

| Limit Type | Description |
|------------|-------------|
| CPU concurrency | Max concurrent CPU tasks |
| GPU concurrency | Max concurrent GPU tasks |
| Total memory | Memory usage cap |
| GPU memory | GPU memory usage cap |
| Total tasks | Task queue size limit |

---

## Execution Flow

```
1. Scheduler selects ready tasks from DependencyManager
2. For each ready task:
   a. Determine target device (CPU/GPU) via scheduling policy
   b. Acquire execution resource (thread slot or CUDA stream)
   c. Execute task with proper context
   d. On completion, notify DependencyManager
3. Repeat until all tasks completed or failed
```

---

## Scheduling Policies

| Policy | Description |
|--------|-------------|
| **Default** | Respect task's preferred device |
| **GpuFirst** | Prefer GPU when device is "Any" |
| **CpuFirst** | Prefer CPU when device is "Any" |
| **RoundRobin** | Alternate between CPU and GPU |
| **ShortestJobFirst** | Prioritize shorter tasks |

---

## Related Documents

- [Product Requirements](../product/001-heterogeneous-task-scheduler.md) (REQ-4, REQ-5)
- [Scheduling Policies Guide](../../docs/en/scheduling-policies.md)
