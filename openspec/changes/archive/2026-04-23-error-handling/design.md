# Design: Error Handling

[![Spec](https://img.shields.io/badge/Spec-RFC-blue)]()
[![Version](https://img.shields.io/badge/Version-1.2.0-green)]()
[![Status](https://img.shields.io/badge/Status-Implemented-brightgreen)]()

## Overview

HTS provides comprehensive error handling capabilities including error callbacks, retry policies, failure propagation through dependency chains, and graceful shutdown.

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

## Retry Policies

| Policy | Use Case |
|--------|----------|
| **NoRetry** | No retry (default) |
| **Fixed** | Fixed delay between attempts |
| **ExponentialBackoff** | Exponentially increasing delay |
| **JitteredBackoff** | Randomized backoff (avoid thundering herd) |
| **Conditional** | Retry only specific error types |
| **CircuitBreaker** | Stop retrying after consecutive failures |

## Failure Propagation

When a task fails:
1. Error callback is invoked
2. All dependent tasks are marked as `Blocked`
3. Execution continues for independent tasks
4. Final stats report all failures

## Graceful Shutdown

- Complete in-flight tasks
- Cancel pending tasks
- Release all resources
- Return final execution stats

## File Changes

| File | Action | Description |
|------|--------|-------------|
| `src/core/error_handling.cpp` | add | Error handling utilities |
| `src/core/retry_policy.cpp` | add | Retry policy implementations |
| `include/hts/error.hpp` | add | Error structures |
| `include/hts/retry_policy.hpp` | add | Retry policy interface |

## Testing Strategy

- Unit tests for error callback invocation
- Unit tests for each retry policy
- Integration tests for failure propagation
- Tests for graceful shutdown
