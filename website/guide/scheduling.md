# Scheduling

HTS provides flexible scheduling policies to optimize CPU and GPU utilization for different workload patterns.

## Policy Architecture

All scheduling policies inherit from the base `SchedulingPolicy` class:

```cpp
class SchedulingPolicy {
public:
    virtual ~SchedulingPolicy() = default;
    
    // Select the next task to execute from ready queue
    virtual Task* select_next(std::vector<Task*>& ready_queue) = 0;
    
    // Called when a task completes
    virtual void on_task_completed(Task* task) {}
    
    // Called when a task fails
    virtual void on_task_failed(Task* task) {}
};
```

## Built-in Policies

### 1. GPU Priority Policy

Prioritizes GPU tasks over CPU tasks.

**Use Case:** GPU-heavy workloads where GPU resources should be kept busy.

```cpp
#include <hts/scheduling_policy.hpp>

auto policy = std::make_unique<GPUPriorityPolicy>();
scheduler.set_policy(std::move(policy));
```

**Behavior:**
- Scans ready queue for GPU tasks first
- If multiple GPU tasks ready, selects by priority
- Falls back to CPU tasks only when no GPU tasks available

**When to Use:**
- Deep learning inference
- GPU compute kernels
- Data parallelism on GPU

### 2. CPU Priority Policy

Prioritizes CPU tasks over GPU tasks.

**Use Case:** Workloads with significant CPU preprocessing.

```cpp
auto policy = std::make_unique<CPUPriorityPolicy>();
scheduler.set_policy(std::move(policy));
```

**Behavior:**
- Prefers CPU tasks to keep CPU busy
- GPU tasks run when CPU tasks complete or GPU becomes idle
- Good for CPU → GPU pipelines

**When to Use:**
- Data preprocessing before GPU compute
- CPU-bound workloads with occasional GPU acceleration
- I/O heavy workloads

### 3. Round Robin Policy

Alternates between CPU and GPU tasks.

**Use Case:** Balanced workloads with mixed CPU/GPU tasks.

```cpp
auto policy = std::make_unique<RoundRobinPolicy>();
scheduler.set_policy(std::move(policy));
```

**Behavior:**
- Maintains separate ready queues for CPU and GPU
- Alternates between queues each scheduling decision
- Ensures fair resource utilization

**When to Use:**
- Mixed workloads
- Preventing GPU or CPU starvation
- General-purpose scheduling

### 4. Load Based Policy

Dynamically selects based on current device load.

**Use Case:** Dynamic workloads with varying resource requirements.

```cpp
auto policy = std::make_unique<LoadBasedPolicy>();
scheduler.set_policy(std::move(policy));
```

**Behavior:**
- Monitors CPU thread pool utilization
- Monitors GPU stream utilization
- Selects task type based on which device is less busy
- Adapts to workload changes automatically

**When to Use:**
- Unpredictable workloads
- Maximizing overall utilization
- Production systems with varying tasks

## Creating Custom Policies

You can implement your own scheduling policy:

```cpp
#include <hts/scheduling_policy.hpp>

class MyCustomPolicy : public SchedulingPolicy {
public:
    Task* select_next(std::vector<Task*>& ready_queue) override {
        // Custom selection logic
        Task* best_task = nullptr;
        int best_score = -1;
        
        for (auto* task : ready_queue) {
            int score = calculate_score(task);
            if (score > best_score) {
                best_score = score;
                best_task = task;
            }
        }
        
        return best_task;
    }
    
private:
    int calculate_score(Task* task) {
        // Score based on priority, device type, memory usage, etc.
        int score = task->get_priority();
        
        // Prefer smaller tasks for better parallelism
        score += 100 / (task->get_estimated_duration_ms() + 1);
        
        return score;
    }
};

// Use custom policy
scheduler.set_policy(std::make_unique<MyCustomPolicy>());
```

## Policy Configuration

### CPU Thread Pool Size

Configure the number of CPU worker threads:

```cpp
#include <hts/execution_engine.hpp>

ExecutionEngineConfig config;
config.cpu_thread_count = 8;  // Default: number of CPU cores
config.gpu_stream_count = 4;  // Default: 4 streams

scheduler.init(&graph, config);
```

### GPU Stream Configuration

Configure GPU stream count and priorities:

```cpp
StreamManagerConfig stream_config;
stream_config.max_streams = 8;
stream_config.default_priority = cudaStreamNonBlocking;

scheduler.configure_streams(stream_config);
```

## Scheduling Metrics

HTS tracks scheduling efficiency metrics:

```cpp
const auto& stats = scheduler.get_stats();

std::cout << "Scheduling Statistics:" << std::endl;
std::cout << "  Total tasks scheduled: " << stats.tasks_scheduled << std::endl;
std::cout << "  CPU tasks: " << stats.cpu_tasks << std::endl;
std::cout << "  GPU tasks: " << stats.gpu_tasks << std::endl;
std::cout << "  Average wait time: " << stats.avg_wait_time_ms << " ms" << std::endl;
std::cout << "  Average execution time: " << stats.avg_execution_time_ms << " ms" << std::endl;
std::cout << "  Parallelism factor: " << stats.parallelism_factor << "x" << std::endl;
```

## Choosing the Right Policy

| Workload Type | Recommended Policy | Reason |
|---------------|-------------------|--------|
| GPU-heavy computation | GPUPriorityPolicy | Keeps GPU busy |
| CPU preprocessing → GPU | CPUPriorityPolicy | Feeds GPU efficiently |
| Mixed CPU/GPU | RoundRobinPolicy | Balanced utilization |
| Varying workloads | LoadBasedPolicy | Adapts automatically |
| Custom requirements | Your own policy | Tailored to your needs |

## Performance Tips

1. **Profile First**: Use the built-in profiler to understand your workload
2. **Right-Size Thread Pool**: Too many threads cause contention, too few underutilize
3. **Monitor GPU Utilization**: Aim for >80% GPU utilization
4. **Batch Small Tasks**: Group small tasks to reduce scheduling overhead
5. **Use Priorities**: Set meaningful priorities to guide the scheduler

## Example: Policy Comparison

```cpp
// Run same graph with different policies

std::vector<std::unique_ptr<SchedulingPolicy>> policies;
policies.push_back(std::make_unique<GPUPriorityPolicy>());
policies.push_back(std::make_unique<CPUPriorityPolicy>());
policies.push_back(std::make_unique<RoundRobinPolicy>());
policies.push_back(std::make_unique<LoadBasedPolicy>());

for (auto& policy : policies) {
    TaskGraph graph = create_test_graph();
    Scheduler scheduler;
    scheduler.set_policy(policy.get());
    scheduler.init(&graph);
    
    auto start = std::chrono::high_resolution_clock::now();
    scheduler.execute();
    scheduler.wait_for_completion();
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << policy->get_name() << ": " << duration.count() << " ms" << std::endl;
}
```

## Next Steps

- [Memory](/guide/memory) — GPU memory pool management
- [Error Handling](/guide/error-handling) — Handling task failures
- [Architecture](/guide/architecture) — Overall system architecture
- [API Reference](/api/scheduler) — Scheduler API documentation
