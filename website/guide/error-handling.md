# Error Handling

HTS provides comprehensive error handling mechanisms to handle failures gracefully in production environments.

## Error Types

HTS defines several error categories:

```cpp
enum class ErrorCode {
    Success = 0,
    
    // Resource errors
    OutOfMemory,
    DeviceNotAvailable,
    InvalidDevice,
    
    // Task errors
    TaskNotFound,
    TaskCreationFailed,
    TaskExecutionFailed,
    TaskTimeout,
    
    // Graph errors
    InvalidGraph,
    CycleDetected,
    DependencyNotFound,
    
    // CUDA errors
    CudaError,
    CudaMallocFailed,
    CudaKernelError,
    
    // System errors
    ThreadCreationFailed,
    StreamCreationFailed,
    InternalError
};
```

## Error Handling Mechanisms

### 1. Return Codes

HTS functions return `Error` objects:

```cpp
#include <hts/types.hpp>

Error err = scheduler.init(&graph);
if (!err.ok()) {
    std::cerr << "Failed to initialize: " << err.message() << std::endl;
    std::cerr << "Error code: " << static_cast<int>(err.code()) << std::endl;
    return 1;
}
```

### 2. Exceptions

HTS throws exceptions for critical errors:

```cpp
try {
    graph.add_dependency(taskA->id(), taskB->id());
    graph.add_dependency(taskB->id(), taskA->id()); // Creates cycle!
} catch (const CycleDetectedError& e) {
    std::cerr << "Cycle detected: " << e.what() << std::endl;
    std::cerr << "Cycle: " << e.get_cycle_description() << std::endl;
}
```

### 3. Task Status

Tasks track their execution status:

```cpp
auto task = graph.add_task(DeviceType::GPU, "RiskyTask");
task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // Risky GPU operation
    cudaError_t err = my_kernel<<<...>>>();
    if (err != cudaSuccess) {
        throw std::runtime_error("Kernel failed: " + std::string(cudaGetErrorString(err)));
    }
});

// After execution
if (task->get_status() == TaskStatus::Failed) {
    std::cerr << "Task failed: " << task->get_error_message() << std::endl;
}
```

## Retry Policies

HTS supports automatic retry for transient failures:

### Basic Retry

```cpp
#include <hts/retry_policy.hpp>

RetryPolicy simple_retry;
simple_retry.max_retries = 3;
simple_retry.backoff_ms = 100;
simple_retry.backoff_multiplier = 2.0f;

task->set_retry_policy(simple_retry);
```

### Exponential Backoff

```cpp
// Retry with exponential backoff
// Attempt 1: wait 100ms
// Attempt 2: wait 200ms
// Attempt 3: wait 400ms
// Attempt 4: wait 800ms
task->set_retry_policy(RetryPolicy{
    .max_retries = 4,
    .backoff_ms = 100,
    .backoff_multiplier = 2.0f
});
```

### Custom Retry Logic

```cpp
class MyRetryPolicy : public RetryPolicy {
public:
    bool should_retry(Task* task, int attempt_count) override {
        // Custom retry logic
        if (task->get_error_code() == ErrorCode::OutOfMemory) {
            // Don't retry OOM errors
            return false;
        }
        
        // Retry up to 5 times
        return attempt_count < 5;
    }
    
    int calculate_delay_ms(Task* task, int attempt_count) override {
        // Custom delay logic
        return std::min(1000, 100 * attempt_count);
    }
};
```

## Failure Propagation

When a task fails, HTS can propagate the failure to dependent tasks:

### Default Behavior

By default, if a task fails, all its successors are marked as failed:

```cpp
graph.add_dependency(taskA->id(), taskB->id());
graph.add_dependency(taskB->id(), taskC->id());

// If taskB fails:
// - taskA: Completed (already finished)
// - taskB: Failed
// - taskC: Failed (dependency failed)
```

### Custom Failure Handling

You can customize failure propagation:

```cpp
task->set_failure_handler([](Task* failed_task) {
    std::cerr << "Task " << failed_task->get_name() << " failed" << std::endl;
    
    // Log the failure
    log_error(failed_task->get_error_message());
    
    // Don't propagate failure (allow successors to run anyway)
    failed_task->get_graph().mark_successors_as_ready(failed_task->id());
});
```

### Failure Callbacks

Register callbacks for failure notification:

```cpp
task->on_failure([](Task* task) {
    std::cerr << "Task failed: " << task->get_name() << std::endl;
    std::cerr << "Error: " << task->get_error_message() << std::endl;
    std::cerr << "Retries: " << task->get_retry_count() << std::endl;
    
    // Notify monitoring system
    alert_team("Task " + task->get_name() + " failed");
});
```

## Graceful Degradation

HTS can continue execution even when some tasks fail:

### Continue on Error

```cpp
SchedulerConfig config;
config.stop_on_first_error = false; // Continue even if tasks fail
config.max_failed_tasks = 10;       // Stop if too many failures

scheduler.configure(config);
```

### Fallback Tasks

Define fallback tasks that run when primary tasks fail:

```cpp
auto gpu_task = graph.add_task(DeviceType::GPU, "GPU_Compute");
gpu_task->set_gpu_function(gpu_compute_kernel);

// Fallback to CPU if GPU fails
gpu_task->set_fallback([](TaskContext& ctx) {
    std::cerr << "GPU task failed, falling back to CPU" << std::endl;
    cpu_compute_function(ctx);
});
```

## Error Reporting

### Collecting Errors

After execution, collect all errors:

```cpp
scheduler.execute();
scheduler.wait_for_completion();

const auto& failed_tasks = scheduler.get_failed_tasks();
for (const auto* task : failed_tasks) {
    std::cerr << "Failed: " << task->get_name() << std::endl;
    std::cerr << "  Error: " << task->get_error_message() << std::endl;
    std::cerr << "  Retries: " << task->get_retry_count() << std::endl;
}
```

### Error Statistics

```cpp
const auto& stats = scheduler.get_error_stats();

std::cout << "Error Statistics:" << std::endl;
std::cout << "  Total failures: " << stats.total_failures << std::endl;
std::cout << "  Successful retries: " << stats.successful_retries << std::endl;
std::cout << "  Permanent failures: " << stats.permanent_failures << std::endl;
std::cout << "  Error rate: " << stats.error_rate * 100 << "%" << std::endl;

// Breakdown by error code
for (const auto& [code, count] : stats.error_breakdown) {
    std::cout << "  " << error_code_to_string(code) << ": " << count << std::endl;
}
```

## Best Practices

### 1. Set Appropriate Retry Policies

```cpp
// GPU tasks often benefit from retry
gpu_task->set_retry_policy(RetryPolicy{
    .max_retries = 3,
    .backoff_ms = 200,
    .backoff_multiplier = 2.0f
});

// CPU tasks may not need retries for deterministic errors
cpu_task->set_retry_policy(RetryPolicy{
    .max_retries = 1,
    .backoff_ms = 0
});
```

### 2. Use Fallbacks for Critical Tasks

```cpp
// Important tasks should have fallbacks
critical_task->set_fallback([](TaskContext& ctx) {
    // Simplified but functional fallback
    perform_basic_processing(ctx);
});
```

### 3. Monitor Error Rates

```cpp
// Periodically check error rates
auto check_errors = [&]() {
    const auto& stats = scheduler.get_error_stats();
    if (stats.error_rate > 0.1) { // 10% error rate
        alert_team("High error rate detected: " + 
                   std::to_string(stats.error_rate * 100) + "%");
    }
};
```

### 4. Log Detailed Error Information

```cpp
task->on_failure([](Task* task) {
    // Log comprehensive error information
    logger::error("Task failure", {
        {"task_id", task->get_id()},
        {"task_name", task->get_name()},
        {"error", task->get_error_message()},
        {"retries", task->get_retry_count()},
        {"execution_time", task->get_execution_time_ms()}
    });
});
```

### 5. Handle OOM Gracefully

```cpp
// OOM errors often require special handling
task->set_failure_handler([](Task* task) {
    if (task->get_error_code() == ErrorCode::OutOfMemory) {
        // Try to free memory and retry
        auto& pool = task->get_memory_pool();
        pool.defragment();
        
        // Mark for retry
        task->mark_for_retry();
    }
});
```

## Example: Production Error Handling

```cpp
// Production-ready task execution with comprehensive error handling

auto robust_task = graph.add_task(DeviceType::GPU, "ProductionTask");

robust_task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    try {
        // Perform GPU computation
        cudaError_t err = my_kernel<<<blocks, threads, 0, stream>>>(data);
        
        if (err != cudaSuccess) {
            throw std::runtime_error(
                "CUDA error: " + std::string(cudaGetErrorString(err))
            );
        }
        
        cudaStreamSynchronize(stream);
        
    } catch (const std::exception& e) {
        // Log and re-throw for HTS to handle
        logger::error("GPU computation failed", {{"error", e.what()}});
        throw;
    }
});

robust_task->set_retry_policy(RetryPolicy{
    .max_retries = 3,
    .backoff_ms = 500,
    .backoff_multiplier = 2.0f
});

robust_task->set_fallback([](TaskContext& ctx) {
    logger::warn("Falling back to CPU for task");
    cpu_fallback_function(ctx);
});

robust_task->on_failure([](Task* task) {
    metrics::increment("task.failures", {
        {"task_name", task->get_name()},
        {"error_code", std::to_string(task->get_error_code())}
    });
});
```

## Next Steps

- [Architecture](/guide/architecture) — System architecture overview
- [Memory](/guide/memory) — Memory pool management
- [API Reference](/api/) — Complete API documentation
- [Examples](/examples/) — Production-ready examples
