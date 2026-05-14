# Scheduler API Reference

The `Scheduler` class is the main entry point for executing task graphs.

## Class Overview

```cpp
namespace hts {

class Scheduler {
public:
    Scheduler();
    ~Scheduler();
    
    // Configuration
    void set_policy(std::unique_ptr<SchedulingPolicy> policy);
    void configure(const SchedulerConfig& config);
    void configure_memory_pool(const MemoryPoolConfig& config);
    
    // Initialization
    Error init(TaskGraph* graph);
    Error init(TaskGraph* graph, const ExecutionEngineConfig& engine_config);
    
    // Execution
    Error execute();
    Error wait_for_completion();
    Error wait_for_completion(std::chrono::milliseconds timeout);
    
    // Status
    bool is_running() const;
    bool is_completed() const;
    
    // Statistics
    const SchedulerStats& get_stats() const;
    const ErrorStats& get_error_stats() const;
    
    // Memory Pool
    MemoryPool& get_memory_pool();
    
    // Reset
    void reset();
};

} // namespace hts
```

## Constructor

### Scheduler()

Creates a new Scheduler instance.

```cpp
Scheduler scheduler;
```

## Configuration Methods

### set_policy()

Sets the scheduling policy to use during execution.

```cpp
void set_policy(std::unique_ptr<SchedulingPolicy> policy);
```

**Parameters:**
- `policy`: The scheduling policy (e.g., `GPUPriorityPolicy`, `RoundRobinPolicy`)

**Example:**
```cpp
scheduler.set_policy(std::make_unique<GPUPriorityPolicy>());
scheduler.set_policy(std::make_unique<LoadBasedPolicy>());
```

**Note:** Must be called before `init()`.

### configure()

Configures scheduler behavior.

```cpp
void configure(const SchedulerConfig& config);
```

**SchedulerConfig Structure:**
```cpp
struct SchedulerConfig {
    bool stop_on_first_error = false;    // Stop execution on first error
    size_t max_failed_tasks = 10;        // Maximum failed tasks before stopping
    size_t cpu_thread_count = 0;         // 0 = auto-detect (CPU cores)
    size_t gpu_stream_count = 4;         // Number of GPU streams
    bool enable_profiling = false;        // Enable performance profiling
};
```

**Example:**
```cpp
SchedulerConfig config;
config.cpu_thread_count = 8;
config.gpu_stream_count = 8;
config.enable_profiling = true;

scheduler.configure(config);
```

### configure_memory_pool()

Configures the GPU memory pool.

```cpp
void configure_memory_pool(const MemoryPoolConfig& config);
```

**MemoryPoolConfig Structure:**
```cpp
struct MemoryPoolConfig {
    size_t pool_size_mb = 4096;           // Pool size in MB
    size_t min_block_size_kb = 4;         // Minimum block size in KB
    size_t max_block_size_mb = 1024;      // Maximum block size in MB
    bool enable_defragmentation = true;    // Enable auto defragmentation
    float defrag_threshold = 0.3f;         // Defrag when 30% fragmented
};
```

**Example:**
```cpp
MemoryPoolConfig pool_config;
pool_config.pool_size_mb = 8192;  // 8 GB
pool_config.enable_defragmentation = true;

scheduler.configure_memory_pool(pool_config);
```

## Initialization Methods

### init()

Initializes the scheduler with a task graph.

```cpp
Error init(TaskGraph* graph);
Error init(TaskGraph* graph, const ExecutionEngineConfig& engine_config);
```

**Parameters:**
- `graph`: Pointer to the TaskGraph to execute
- `engine_config`: Optional execution engine configuration

**Returns:**
- `Error` object indicating success or failure

**Example:**
```cpp
TaskGraph graph;
// ... add tasks and dependencies ...

Error err = scheduler.init(&graph);
if (!err.ok()) {
    std::cerr << "Initialization failed: " << err.message() << std::endl;
    return 1;
}
```

**Errors:**
- `InvalidGraph`: Task graph is invalid or has cycles
- `DeviceNotAvailable`: Required device (GPU) not available
- `OutOfMemory`: Cannot allocate memory for initialization

## Execution Methods

### execute()

Starts executing the task graph.

```cpp
Error execute();
```

**Returns:**
- `Error` object indicating success or failure

**Example:**
```cpp
scheduler.init(&graph);
scheduler.execute();
```

**Note:** This is non-blocking. Use `wait_for_completion()` to wait for completion.

### wait_for_completion()

Blocks until all tasks complete or timeout.

```cpp
Error wait_for_completion();
Error wait_for_completion(std::chrono::milliseconds timeout);
```

**Parameters:**
- `timeout`: Maximum time to wait (optional)

**Returns:**
- `Error` object indicating success or failure

**Example:**
```cpp
scheduler.execute();

// Wait indefinitely
Error err = scheduler.wait_for_completion();

// Or with timeout
Error err = scheduler.wait_for_completion(std::chrono::seconds(30));
if (err.code() == ErrorCode::TaskTimeout) {
    std::cerr << "Execution timed out" << std::endl;
}
```

## Status Methods

### is_running()

Checks if the scheduler is currently running.

```cpp
bool is_running() const;
```

**Returns:**
- `true` if executing tasks, `false` otherwise

**Example:**
```cpp
scheduler.execute();
while (scheduler.is_running()) {
    std::cout << "Progress: " << scheduler.get_stats().completion_percentage() << "%" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

### is_completed()

Checks if all tasks have completed.

```cpp
bool is_completed() const;
```

**Returns:**
- `true` if all tasks completed successfully, `false` otherwise

## Statistics Methods

### get_stats()

Returns scheduler execution statistics.

```cpp
const SchedulerStats& get_stats() const;
```

**SchedulerStats Structure:**
```cpp
struct SchedulerStats {
    size_t tasks_scheduled = 0;           // Total tasks scheduled
    size_t cpu_tasks = 0;                 // CPU tasks executed
    size_t gpu_tasks = 0;                 // GPU tasks executed
    size_t failed_tasks = 0;              // Tasks that failed
    double avg_wait_time_ms = 0.0;        // Average task wait time
    double avg_execution_time_ms = 0.0;   // Average task execution time
    double parallelism_factor = 1.0;      // Average parallelism
    double completion_percentage = 0.0;   // Progress percentage
    double total_time_ms = 0.0;           // Total execution time
};
```

**Example:**
```cpp
scheduler.wait_for_completion();
const auto& stats = scheduler.get_stats();

std::cout << "Tasks executed: " << stats.tasks_scheduled << std::endl;
std::cout << "CPU tasks: " << stats.cpu_tasks << std::endl;
std::cout << "GPU tasks: " << stats.gpu_tasks << std::endl;
std::cout << "Total time: " << stats.total_time_ms << " ms" << std::endl;
std::cout << "Parallelism: " << stats.parallelism_factor << "x" << std::endl;
```

### get_error_stats()

Returns error statistics.

```cpp
const ErrorStats& get_error_stats() const;
```

**ErrorStats Structure:**
```cpp
struct ErrorStats {
    size_t total_failures = 0;                     // Total task failures
    size_t successful_retries = 0;                 // Successful retries
    size_t permanent_failures = 0;                 // Permanent failures
    double error_rate = 0.0;                       // Failure rate
    std::map<ErrorCode, size_t> error_breakdown;   // Errors by type
};
```

## Memory Pool Methods

### get_memory_pool()

Returns the memory pool instance.

```cpp
MemoryPool& get_memory_pool();
```

**Example:**
```cpp
auto& pool = scheduler.get_memory_pool();
auto pool_stats = pool.get_stats();

std::cout << "Pool usage: " << pool_stats.allocated_bytes << " bytes" << std::endl;
```

## Reset Method

### reset()

Resets the scheduler for reuse.

```cpp
void reset();
```

**Example:**
```cpp
// First execution
scheduler.init(&graph1);
scheduler.execute();
scheduler.wait_for_completion();

// Reset for second execution
scheduler.reset();
scheduler.init(&graph2);
scheduler.execute();
scheduler.wait_for_completion();
```

## Complete Example

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

using namespace hts;

int main() {
    // Create scheduler
    Scheduler scheduler;
    
    // Configure
    SchedulerConfig config;
    config.cpu_thread_count = 8;
    config.gpu_stream_count = 4;
    config.enable_profiling = true;
    scheduler.configure(config);
    
    // Set policy
    scheduler.set_policy(std::make_unique<GPUPriorityPolicy>());
    
    // Initialize with graph
    TaskGraph graph;
    // ... add tasks ...
    
    Error err = scheduler.init(&graph);
    if (!err.ok()) {
        std::cerr << "Failed: " << err.message() << std::endl;
        return 1;
    }
    
    // Execute
    scheduler.execute();
    
    // Monitor progress
    while (scheduler.is_running()) {
        const auto& stats = scheduler.get_stats();
        std::cout << "Progress: " << stats.completion_percentage << "%" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    // Wait for completion
    scheduler.wait_for_completion();
    
    // Print statistics
    const auto& stats = scheduler.get_stats();
    std::cout << "Completed in " << stats.total_time_ms << " ms" << std::endl;
    std::cout << "Parallelism: " << stats.parallelism_factor << "x" << std::endl;
    
    return 0;
}
```

## Related Pages

- [TaskGraph API](/en/api/task-graph) — TaskGraph reference
- [TaskBuilder API](/en/api/task-builder) — TaskBuilder reference
- [Scheduling Policies](/en/guide/scheduling) — Policy documentation
