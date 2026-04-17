# TaskBuilder API Reference

The `TaskBuilder` class provides a fluent API for creating and configuring tasks with a clean, chainable syntax.

## Class Overview

```cpp
namespace hts {

class TaskBuilder {
public:
    explicit TaskBuilder(TaskGraph& graph);
    
    // Task Creation
    TaskConfigBuilder create_task(const std::string& name);
    TaskConfigBuilder create_task(uint64_t task_id, const std::string& name);
    
    // Bulk Operations
    std::vector<std::shared_ptr<Task>> create_tasks(size_t count, const std::string& prefix);
    
    // Graph Reference
    TaskGraph& get_graph() const;
};

class TaskConfigBuilder {
public:
    // Configuration
    TaskConfigBuilder& device(DeviceType device);
    TaskConfigBuilder& cpu_func(std::function<void(TaskContext&)> func);
    TaskConfigBuilder& gpu_func(std::function<void(TaskContext&, cudaStream_t)> func);
    TaskConfigBuilder& priority(int priority);
    TaskConfigBuilder& memory(size_t bytes);
    TaskConfigBuilder& timeout(std::chrono::milliseconds ms);
    TaskConfigBuilder& retry_policy(const RetryPolicy& policy);
    TaskConfigBuilder& fallback(std::function<void(TaskContext&)> func);
    TaskConfigBuilder& tag(const std::string& tag);
    
    // Build
    std::shared_ptr<Task> build();
};

} // namespace hts
```

## TaskBuilder Constructor

### TaskBuilder()

Creates a TaskBuilder associated with a TaskGraph.

```cpp
TaskBuilder(TaskGraph& graph);
```

**Parameters:**
- `graph`: Reference to the TaskGraph to add tasks to

**Example:**
```cpp
TaskGraph graph;
TaskBuilder builder(graph);
```

## TaskBuilder Methods

### create_task()

Starts building a new task.

```cpp
TaskConfigBuilder create_task(const std::string& name);
```

**Parameters:**
- `name`: Human-readable name for the task

**Returns:**
- `TaskConfigBuilder` for fluent configuration

**Example:**
```cpp
auto task = builder
    .create_task("MyTask")
    .device(DeviceType::GPU)
    .gpu_func(gpu_kernel)
    .build();
```

### create_tasks()

Creates multiple tasks with a common prefix.

```cpp
std::vector<std::shared_ptr<Task>> create_tasks(
    size_t count, 
    const std::string& prefix
);
```

**Parameters:**
- `count`: Number of tasks to create
- `prefix`: Name prefix (tasks will be named "prefix_0", "prefix_1", etc.)

**Returns:**
- Vector of created tasks

**Example:**
```cpp
// Create 10 preprocessing tasks
auto tasks = builder.create_tasks(10, "Preprocess");
// Creates: Preprocess_0, Preprocess_1, ..., Preprocess_9
```

### get_graph()

Returns the associated TaskGraph.

```cpp
TaskGraph& get_graph() const;
```

## TaskConfigBuilder Methods

### device()

Sets the device type for the task.

```cpp
TaskConfigBuilder& device(DeviceType device);
```

**Parameters:**
- `device`: `DeviceType::CPU` or `DeviceType::GPU`

**Example:**
```cpp
builder.create_task("CPUTask").device(DeviceType::CPU);
builder.create_task("GPUTask").device(DeviceType::GPU);
```

### cpu_func()

Sets the CPU function to execute.

```cpp
TaskConfigBuilder& cpu_func(std::function<void(TaskContext&)> func);
```

**Parameters:**
- `func`: Function to execute on CPU

**Example:**
```cpp
builder.create_task("CPUWork")
    .device(DeviceType::CPU)
    .cpu_func([](TaskContext& ctx) {
        // CPU computation here
        std::cout << "Running on CPU" << std::endl;
    });
```

### gpu_func()

Sets the GPU function to execute.

```cpp
TaskConfigBuilder& gpu_func(std::function<void(TaskContext&, cudaStream_t)> func);
```

**Parameters:**
- `func`: Function to execute on GPU (receives CUDA stream)

**Example:**
```cpp
builder.create_task("GPUWork")
    .device(DeviceType::GPU)
    .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
        // GPU kernel launch
        my_kernel<<<blocks, threads, 0, stream>>>(data);
        cudaStreamSynchronize(stream);
    });
```

### priority()

Sets the task priority.

```cpp
TaskConfigBuilder& priority(int priority);
```

**Parameters:**
- `priority`: Priority value (higher = more important)

**Example:**
```cpp
builder.create_task("ImportantTask")
    .priority(100)
    
builder.create_task("LowPriorityTask")
    .priority(1)
```

**Note:** Priorities are relative. The scheduler uses them to make scheduling decisions.

### memory()

Sets the memory requirement in bytes.

```cpp
TaskConfigBuilder& memory(size_t bytes);
```

**Parameters:**
- `bytes`: Memory required in bytes

**Example:**
```cpp
builder.create_task("MemoryIntensive")
    .device(DeviceType::GPU)
    .memory(256 * 1024 * 1024)  // 256 MB
    .gpu_func(gpu_kernel);
```

### timeout()

Sets the execution timeout.

```cpp
TaskConfigBuilder& timeout(std::chrono::milliseconds ms);
```

**Parameters:**
- `ms`: Maximum execution time

**Example:**
```cpp
builder.create_task("LongRunning")
    .timeout(std::chrono::minutes(5))
    .cpu_func(long_computation);
```

**Note:** Tasks that exceed the timeout will be marked as failed.

### retry_policy()

Sets the retry policy for the task.

```cpp
TaskConfigBuilder& retry_policy(const RetryPolicy& policy);
```

**Parameters:**
- `policy`: Retry policy configuration

**RetryPolicy Structure:**
```cpp
struct RetryPolicy {
    size_t max_retries = 0;               // Maximum retry attempts
    size_t backoff_ms = 0;                // Initial backoff in milliseconds
    float backoff_multiplier = 1.0f;      // Multiplier for exponential backoff
};
```

**Example:**
```cpp
builder.create_task("UnreliableTask")
    .device(DeviceType::GPU)
    .gpu_func(risky_kernel)
    .retry_policy(RetryPolicy{
        .max_retries = 3,
        .backoff_ms = 100,
        .backoff_multiplier = 2.0f
    });
// Retries: 100ms, 200ms, 400ms
```

### fallback()

Sets a fallback function if the task fails.

```cpp
TaskConfigBuilder& fallback(std::function<void(TaskContext&)> func);
```

**Parameters:**
- `func`: Fallback function to execute on failure

**Example:**
```cpp
builder.create_task("GPUCompute")
    .device(DeviceType::GPU)
    .gpu_func(gpu_kernel)
    .fallback([](TaskContext& ctx) {
        std::cerr << "GPU failed, using CPU fallback" << std::endl;
        cpu_kernel(ctx);
    });
```

### tag()

Adds a metadata tag to the task.

```cpp
TaskConfigBuilder& tag(const std::string& tag);
```

**Parameters:**
- `tag`: Tag string (can be called multiple times)

**Example:**
```cpp
builder.create_task("DataLoading")
    .tag("io")
    .tag("cpu-bound")
    .tag("preprocessing")
    .cpu_func(load_data);

// Query by tag later
auto io_tasks = graph.find_tasks_by_tag("io");
```

### build()

Builds and returns the configured task.

```cpp
std::shared_ptr<Task> build();
```

**Returns:**
- Shared pointer to the created Task

**Note:** This must be called last to finalize the task creation.

## Complete Examples

### Basic Usage

```cpp
#include <hts/task_builder.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Simple CPU task
    auto cpu_task = builder
        .create_task("CPU_Preprocess")
        .device(DeviceType::CPU)
        .cpu_func([](TaskContext& ctx) {
            std::cout << "Preprocessing data" << std::endl;
        })
        .priority(5)
        .build();
    
    // GPU task with memory requirement
    auto gpu_task = builder
        .create_task("GPU_Compute")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            my_kernel<<<256, 128, 0, stream>>>(data);
            cudaStreamSynchronize(stream);
        })
        .memory(512 * 1024 * 1024)  // 512 MB
        .priority(10)
        .build();
    
    // Task with retry policy
    auto unreliable_task = builder
        .create_task("Unreliable_GPU")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            // Risky operation
            risky_kernel<<<...>>>(stream);
        })
        .retry_policy(RetryPolicy{
            .max_retries = 3,
            .backoff_ms = 200,
            .backoff_multiplier = 2.0f
        })
        .fallback([](TaskContext& ctx) {
            std::cout << "Using CPU fallback" << std::endl;
            cpu_fallback(ctx);
        })
        .build();
    
    // Add dependencies
    graph.add_dependency(cpu_task->id(), gpu_task->id());
    graph.add_dependency(gpu_task->id(), unreliable_task->id());
    
    return 0;
}
```

### Bulk Task Creation

```cpp
#include <hts/task_builder.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Create multiple similar tasks
    auto preprocess_tasks = builder.create_tasks(8, "Preprocess");
    
    // Configure each task
    for (size_t i = 0; i < preprocess_tasks.size(); ++i) {
        auto& task = preprocess_tasks[i];
        task->set_cpu_func([i](TaskContext& ctx) {
            std::cout << "Preprocessing chunk " << i << std::endl;
            process_chunk(i);
        });
        task->set_priority(5);
    }
    
    // Create GPU tasks that depend on all preprocessing
    auto gpu_task = builder
        .create_task("GPU_Process")
        .device(DeviceType::GPU)
        .gpu_func([](TaskContext& ctx, cudaStream_t stream) {
            gpu_compute(stream);
        })
        .build();
    
    // All preprocessing tasks must complete before GPU task
    for (const auto& task : preprocess_tasks) {
        graph.add_dependency(task->id(), gpu_task->id());
    }
    
    return 0;
}
```

### Complex Pipeline

```cpp
#include <hts/task_builder.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    TaskBuilder builder(graph);
    
    // Build a complex pipeline
    auto load = builder
        .create_task("LoadData")
        .device(DeviceType::CPU)
        .cpu_func(load_data)
        .tag("io")
        .build();
    
    auto preprocess = builder
        .create_task("Preprocess")
        .device(DeviceType::CPU)
        .cpu_func(preprocess_data)
        .priority(5)
        .tag("cpu-bound")
        .build();
    
    auto gpu_compute1 = builder
        .create_task("GPU_Stage1")
        .device(DeviceType::GPU)
        .gpu_func(gpu_stage1)
        .memory(256 * 1024 * 1024)
        .priority(10)
        .retry_policy(RetryPolicy{
            .max_retries = 2,
            .backoff_ms = 100
        })
        .build();
    
    auto gpu_compute2 = builder
        .create_task("GPU_Stage2")
        .device(DeviceType::GPU)
        .gpu_func(gpu_stage2)
        .memory(512 * 1024 * 1024)
        .priority(10)
        .build();
    
    auto save = builder
        .create_task("SaveResults")
        .device(DeviceType::CPU)
        .cpu_func(save_results)
        .tag("io")
        .fallback([](TaskContext& ctx) {
            std::cerr << "Save failed, data may be lost" << std::endl;
        })
        .build();
    
    // Set up pipeline dependencies
    graph.add_dependency(load->id(), preprocess->id());
    graph.add_dependency(preprocess->id(), gpu_compute1->id());
    graph.add_dependency(gpu_compute1->id(), gpu_compute2->id());
    graph.add_dependency(gpu_compute2->id(), save->id());
    
    /*
        Pipeline:
        LoadData → Preprocess → GPU_Stage1 → GPU_Stage2 → SaveResults
    */
    
    return 0;
}
```

## Best Practices

1. **Use TaskBuilder for Cleaner Code**: The fluent API is more readable than sequential method calls
2. **Set Priorities Meaningfully**: Higher priority for critical path tasks
3. **Add Retry Policies for GPU Tasks**: GPU operations may fail transiently
4. **Use Tags for Organization**: Makes querying and debugging easier
5. **Set Timeouts for Long Tasks**: Prevents runaway tasks from blocking execution
6. **Provide Fallbacks for Critical Tasks**: Ensures graceful degradation

## Related Pages

- [TaskGraph API](/api/task-graph) — TaskGraph reference
- [Scheduler API](/api/scheduler) — Scheduler reference
- [Task Graph Guide](/guide/task-graph) — Task guide
