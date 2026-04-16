# RFC-004: Error Handling

[![Spec](https://img.shields.io/badge/Spec-RFC-blue)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

> Technical design for error handling, propagation, and recovery mechanisms.

---

## Overview

HTS provides comprehensive error handling capabilities including error callbacks, retry policies, failure propagation through dependency chains, and graceful shutdown.

---

## Error Types

### TaskError Structure

```cpp
struct TaskError {
    Task::TaskId task_id;
    std::string message;
    std::string source;  // e.g., "cuda", "memory", "user_code"
    std::chrono::system_clock::time_point timestamp;
    std::exception_ptr original_exception;
};
```

### Error Categories

| Category | Handling |
|----------|----------|
| **Graph Construction** | Cycle: return false; Invalid ID: throw |
| **Memory** | Allocation failure: return nullptr or throw |
| **CUDA** | Capture error, wrap in TaskError, propagate |
| **Execution** | Catch exceptions, mark task as Failed |

---

## Retry Policies

| Policy | Use Case |
|--------|----------|
| **NoRetry** | No retry (default) |
| **Fixed** | Fixed delay between attempts |
| **ExponentialBackoff** | Exponentially increasing delay |
| **JitteredBackoff** | Randomized backoff (avoid thundering herd) |
| **Conditional** | Retry only specific error types |
| **CircuitBreaker** | Stop retrying after consecutive failures |

---

## Failure Propagation

When a task fails:
1. Error callback is invoked
2. All dependent tasks are marked as `Blocked`
3. Execution continues for independent tasks
4. Final stats report all failures

---

## Graceful Shutdown

- Complete in-flight tasks
- Cancel pending tasks
- Release all resources
- Return final execution stats

---

## Related Documents

- [Product Requirements](../product/001-heterogeneous-task-scheduler.md) (REQ-6)
- [Error Handling Guide](../../docs/en/error-handling.md)
