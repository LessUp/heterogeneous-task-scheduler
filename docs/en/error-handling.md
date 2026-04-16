# Error Handling

> Robust error management and recovery strategies in HTS

---

## Table of Contents

- [Overview](#overview)
- [Error Types](#error-types)
- [Error Callbacks](#error-callbacks)
- [Retry Policies](#retry-policies)
- [Error Propagation](#error-propagation)
- [Best Practices](#best-practices)
- [Advanced Patterns](#advanced-patterns)

---

## Overview

HTS provides a comprehensive error handling system that supports:

- **Error callbacks** for immediate notification
- **Retry policies** for transient failures
- **Error propagation** to dependent tasks
- **Graceful degradation** and recovery

### Error Handling Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Error Flow                                │
├─────────────────────────────────────────────────────────────────┤
│                                                                  │
│   Task Execution                                                 │
│        │                                                         │
│        ▼                                                         │
│   ┌─────────┐                                                    │
│   │ Success │──→ Mark complete → Notify dependents               │
│   └─────────┘                                                    │
│        │                                                         │
│        ▼                                                         │
│   ┌─────────┐     ┌──────────┐                                   │
│   │  Error  │──→  │ Callback │ → Log / Alert / Record            │
│   └─────────┘     └──────────┘                                   │
│        │                                                         │
│        ▼                                                         │
│   ┌─────────────┐   No   ┌─────────────┐                        │
│   │ Retry Left? │──→───→ │ Mark Failed │                        │
│   └─────────────┘        └──────┬──────┘                        │
│        │ Yes                    │                                │
│        ▼                        ▼                                │
│   ┌─────────────┐         ┌─────────────┐                        │
│   │   Retry     │         │ Propagate   │ → Fail dependents     │
│   └─────────────┘         └─────────────┘                        │
│                                                                  │
└─────────────────────────────────────────────────────────────────┘
```

---

## Error Types

### Task-Level Errors

```cpp
enum class TaskErrorType {
    None,           // No error
    Exception,      // C++ exception thrown
    CUDAError,      // CUDA runtime error
    LogicError,     // Logic/precondition violation
    RuntimeError,   // General runtime error
    OutOfMemory,    // Memory allocation failed
    Timeout,        // Timeout exceeded
    Cancelled       // Task was cancelled
};
```

### Scheduler-Level Errors

```cpp
enum class SchedulerErrorType {
    CycleDetected,      // Dependency graph has cycle
    InvalidTask,        // Task refers to invalid ID
    MemoryExhausted,    // Memory pool exhausted
    DeviceUnavailable,  // GPU not available but required
    ExecutionFailed     // General execution failure
};
```

---

## Error Callbacks

### Global Error Handler

```cpp
Scheduler scheduler;

// Set global error callback
scheduler.set_error_callback(
    [](TaskId task_id, const std::string& message, TaskErrorType type) {
        std::cerr << "[ERROR] Task " << task_id 
                  << " failed with " << error_type_name(type)
                  << ": " << message << "\n";
        
        // Additional actions:
        // - Log to file
        // - Send alert
        // - Update metrics
        // - Trigger recovery
    }
);
```

### Per-Task Error Handler

```cpp
auto task = scheduler.graph().add_task(DeviceType::CPU);

task->set_error_callback(
    [](const Task& task, const std::string& message) {
        std::cerr << "Critical task " << task.name() 
                  << " failed: " << message << "\n";
        
        // Custom handling for this specific task
        notify_admin(task.name(), message);
    }
);
```

### Error Context

```cpp
task->set_cpu_function([](TaskContext& ctx) {
    try {
        risky_operation();
    } catch (const NetworkException& e) {
        // Set error with context
        ctx.set_error(e.what());
        ctx.set_error_code(ErrorCode::NetworkFailure);
        ctx.set_retryable(true);  // Can retry
        return;
    } catch (const DataException& e) {
        ctx.set_error(e.what());
        ctx.set_error_code(ErrorCode::InvalidData);
        ctx.set_retryable(false); // Don't retry data errors
        return;
    }
});
```

---

## Retry Policies

### Built-in Retry Policies

```cpp
#include <hts/retry_policy.hpp>

// 1. No Retry (default)
task->set_retry_policy(RetryPolicyFactory::no_retry());

// 2. Fixed Delay
// Retry 3 times with 100ms delay
task->set_retry_policy(
    RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100})
);

// 3. Exponential Backoff
// Retry 5 times: 100ms, 200ms, 400ms, 800ms, 1600ms
task->set_retry_policy(
    RetryPolicyFactory::exponential(5, std::chrono::milliseconds{100})
);

// 4. Jittered Backoff
// Adds randomness (±25%) to prevent thundering herd
task->set_retry_policy(
    RetryPolicyFactory::jittered(5, std::chrono::milliseconds{100})
);
```

### Conditional Retry

```cpp
// Only retry transient errors
auto retry_policy = ConditionalRetryPolicy::transient_errors(
    RetryPolicyFactory::exponential(5)
);

task->set_retry_policy(std::move(retry_policy));
```

### Custom Retry Policy

```cpp
class CircuitBreakerPolicy : public RetryPolicy {
    size_t max_retries_;
    std::chrono::milliseconds timeout_;
    std::atomic<size_t> failure_count_{0};
    std::chrono::steady_clock::time_point circuit_opened_;
    
public:
    CircuitBreakerPolicy(size_t max_retries, 
                         std::chrono::milliseconds timeout)
        : max_retries_(max_retries), timeout_(timeout) {}
    
    bool should_retry(size_t attempt_count, 
                      const std::exception& error) override {
        // Check if circuit is open
        if (failure_count_ > max_retries_) {
            auto now = std::chrono::steady_clock::now();
            if (now - circuit_opened_ < timeout_) {
                return false; // Circuit open, fail fast
            }
            failure_count_ = 0; // Reset circuit
        }
        
        return attempt_count < max_retries_;
    }
    
    std::chrono::milliseconds delay(size_t attempt_count) override {
        return std::chrono::milliseconds{100 * (1 << attempt_count)};
    }
    
    void on_failure(const std::exception& error) override {
        failure_count_++;
        if (failure_count_ > max_retries_) {
            circuit_opened_ = std::chrono::steady_clock::now();
        }
    }
    
    void on_success() override {
        failure_count_ = 0;
    }
};

// Usage
task->set_retry_policy(std::make_unique<CircuitBreakerPolicy>(3, std::chrono::seconds{30}));
```

---

## Error Propagation

### Default Behavior

When a task fails, all dependent tasks are automatically cancelled:

```
Task A (fails)
   │
   ├──→ Task B (cancelled)
   │
   └──→ Task C (cancelled)
            │
            └──→ Task D (cancelled)
```

### Custom Propagation

```cpp
// Continue execution despite failure
task->set_error_propagation(ErrorPropagation::Continue);

// Custom propagation logic
task->set_error_handler([](const Task& failed_task, TaskGraph& graph) {
    // Only propagate to high-priority dependents
    for (auto& dep_id : failed_task.dependents()) {
        auto dep = graph.get_task(dep_id);
        if (dep->priority() == TaskPriority::Critical) {
            dep->cancel();
        }
    }
});
```

### Graceful Degradation

```cpp
auto primary_task = scheduler.graph().add_task(DeviceType::GPU);
primary_task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // Try high-accuracy model
    run_precise_model();
});

auto fallback_task = scheduler.graph().add_task(DeviceType::CPU);
fallback_task->set_cpu_function([](TaskContext& ctx) {
    // Fallback to faster, less accurate model
    run_fast_model();
});

// Link with error handling
primary_task->on_error([fallback_task](const Task& task) {
    // Trigger fallback on failure
    fallback_task->set_skip_on_success(true);  // Only run if primary fails
});
```

---

## Best Practices

### 1. Distinguish Retryable vs Non-Retryable

```cpp
task->set_cpu_function([](TaskContext& ctx) {
    try {
        process_data();
    } catch (const NetworkTimeout& e) {
        // Transient - should retry
        ctx.set_error(e.what(), true);  // retryable = true
    } catch (const InvalidData& e) {
        // Permanent - don't retry
        ctx.set_error(e.what(), false); // retryable = false
    }
});
```

### 2. Set Appropriate Retry Limits

```cpp
// Network requests: reasonable retries
auto network_task = graph.add_task(DeviceType::CPU);
network_task->set_retry_policy(RetryPolicyFactory::exponential(3));

// GPU computation: fewer retries (expensive)
auto gpu_task = graph.add_task(DeviceType::GPU);
gpu_task->set_retry_policy(RetryPolicyFactory::fixed(1));

// Critical path: no retries, fail fast
auto critical_task = graph.add_task(DeviceType::CPU);
critical_task->set_retry_policy(RetryPolicyFactory::no_retry());
```

### 3. Use Exponential Backoff

```cpp
// Prevents overwhelming failing services
auto policy = RetryPolicyFactory::exponential(
    5,                                    // max 5 attempts
    std::chrono::milliseconds{100},      // initial delay
    std::chrono::seconds{30}             // max delay cap
);
```

### 4. Implement Circuit Breaker

```cpp
// Prevent cascading failures
class CircuitBreaker {
    enum State { Closed, Open, HalfOpen };
    State state_ = Closed;
    size_t failure_count_ = 0;
    
public:
    bool allow_request() {
        if (state_ == Open) {
            if (time_since_opened() > timeout_) {
                state_ = HalfOpen;
                return true;
            }
            return false;
        }
        return true;
    }
    
    void record_success() {
        failure_count_ = 0;
        state_ = Closed;
    }
    
    void record_failure() {
        if (++failure_count_ >= threshold_) {
            state_ = Open;
        }
    }
};
```

### 5. Log Contextual Information

```cpp
scheduler.set_error_callback([](TaskId id, const std::string& msg, 
                                 TaskErrorType type) {
    auto& task = scheduler.graph().get_task(id);
    
    std::cerr << "ERROR:\n"
              << "  Task: " << task.name() << " (" << id << ")\n"
              << "  Type: " << error_type_name(type) << "\n"
              << "  Message: " << msg << "\n"
              << "  Attempt: " << task.retry_count() << "/" 
              << task.max_retries() << "\n"
              << "  Timestamp: " << current_timestamp() << "\n";
});
```

---

## Advanced Patterns

### Pattern 1: Dead Letter Queue

```cpp
class DeadLetterQueue {
    std::queue<std::pair<TaskId, std::string>> failed_tasks_;
    
public:
    void enqueue(TaskId id, const std::string& reason) {
        failed_tasks_.push({id, reason});
        persist_to_disk(id, reason);
    }
    
    void process_retries() {
        while (!failed_tasks_.empty()) {
            auto [id, reason] = failed_tasks_.front();
            failed_tasks_.pop();
            
            if (should_retry(id)) {
                retry_task(id);
            }
        }
    }
};

// Usage
DeadLetterQueue dlq;
scheduler.set_error_callback([&dlq](TaskId id, const std::string& msg, auto) {
    dlq.enqueue(id, msg);
});
```

### Pattern 2: Fallback Chain

```cpp
auto t1 = graph.add_task(DeviceType::GPU);
t1->set_gpu_function([](auto& ctx, auto stream) { /* primary */ });

auto t2 = graph.add_task(DeviceType::CPU);
t2->set_cpu_function([](auto& ctx) { /* fallback 1 */ });

auto t3 = graph.add_task(DeviceType::CPU);
t3->set_cpu_function([](auto& ctx) { /* fallback 2 */ });

// Chain fallbacks
t1->on_failure([t2]() { t2->execute(); });
t2->on_failure([t3]() { t3->execute(); });
```

### Pattern 3: Partial Success

```cpp
task->set_cpu_function([](TaskContext& ctx) {
    std::vector<Result> results;
    std::vector<std::string> errors;
    
    for (auto& item : batch) {
        try {
            results.push_back(process(item));
        } catch (const std::exception& e) {
            errors.push_back(e.what());
            // Continue with remaining items
        }
    }
    
    ctx.set_output("results", results);
    ctx.set_output("errors", errors);
    
    // Only fail if all items failed
    if (results.empty()) {
        ctx.set_error("All items failed");
    }
});
```

### Pattern 4: Timeout Handling

```cpp
task->set_timeout(std::chrono::seconds{30});

task->set_cpu_function([](TaskContext& ctx) {
    // Check for cancellation
    while (!ctx.is_cancelled()) {
        do_work();
        
        // Periodically check
        if (++iterations % 100 == 0) {
            if (ctx.is_cancelled()) {
                cleanup();
                return;
            }
        }
    }
});
```

---

## Error Code Reference

| Error Code | Description | Retryable |
|------------|-------------|-----------|
| `Success` | No error | - |
| `Unknown` | Unknown error | Maybe |
| `OutOfMemory` | GPU/CPU memory exhausted | Yes |
| `CUDAError` | CUDA runtime error | Depends |
| `NetworkError` | Network failure | Yes |
| `TimeoutError` | Operation timed out | Yes |
| `InvalidData` | Data validation failed | No |
| `NotFound` | Resource not found | No |
| `PermissionDenied` | Access denied | No |
| `Cancelled` | Task was cancelled | No |

---

## Further Reading

- [API Reference](api-reference.md) - Error handling APIs
- [Retry Policies](scheduling-policies.md) - Built-in retry mechanisms
- [Examples](examples.md) - Error handling examples
