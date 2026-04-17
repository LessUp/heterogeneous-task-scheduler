# TaskGraph API Reference

The `TaskGraph` class manages the DAG (Directed Acyclic Graph) of tasks and their dependencies.

## Class Overview

```cpp
namespace hts {

class TaskGraph {
public:
    TaskGraph();
    ~TaskGraph();
    
    // Task Creation
    std::shared_ptr<Task> add_task(DeviceType device, const std::string& name = "");
    std::shared_ptr<Task> add_task(const TaskConfig& config);
    
    // Task Removal
    bool remove_task(uint64_t task_id);
    
    // Dependencies
    Error add_dependency(uint64_t predecessor_id, uint64_t successor_id);
    Error remove_dependency(uint64_t predecessor_id, uint64_t successor_id);
    
    // Queries
    bool has_dependency(uint64_t successor_id, uint64_t predecessor_id) const;
    std::vector<uint64_t> get_predecessors(uint64_t task_id) const;
    std::vector<uint64_t> get_successors(uint64_t task_id) const;
    std::shared_ptr<Task> get_task(uint64_t task_id) const;
    
    // Graph Properties
    size_t num_tasks() const;
    size_t num_dependencies() const;
    std::vector<uint64_t> topological_sort() const;
    std::vector<uint64_t> get_critical_path() const;
    std::vector<uint64_t> find_orphan_tasks() const;
    
    // Validation
    Error validate() const;
    bool has_cycles() const;
    std::vector<uint64_t> detect_cycles() const;
    
    // Execution Info
    std::vector<uint64_t> get_ready_tasks() const;
    bool is_task_ready(uint64_t task_id) const;
    bool is_task_completed(uint64_t task_id) const;
    bool is_task_failed(uint64_t task_id) const;
    
    // Reset
    void reset();
    void reset_task_status();
};

} // namespace hts
```

## Constructor

### TaskGraph()

Creates an empty task graph.

```cpp
TaskGraph graph;
```

## Task Creation Methods

### add_task()

Adds a new task to the graph.

```cpp
std::shared_ptr<Task> add_task(DeviceType device, const std::string& name = "");
std::shared_ptr<Task> add_task(const TaskConfig& config);
```

**Parameters:**
- `device`: The device type (CPU or GPU)
- `name`: Optional human-readable name for the task
- `config`: Task configuration structure

**Returns:**
- Shared pointer to the created Task

**TaskConfig Structure:**
```cpp
struct TaskConfig {
    std::string name;                    // Task name
    DeviceType device = DeviceType::CPU; // Device type
    int priority = 0;                    // Priority (higher = more important)
    size_t memory_requirement = 0;       // Memory in bytes
    std::chrono::milliseconds timeout{0}; // Execution timeout
};
```

**Example:**
```cpp
// Simple task creation
auto cpu_task = graph.add_task(DeviceType::CPU, "PreprocessData");
auto gpu_task = graph.add_task(DeviceType::GPU, "GPUCompute");

// Task with configuration
TaskConfig config;
config.name = "ComplexTask";
config.device = DeviceType::GPU;
config.priority = 10;
config.memory_requirement = 256 * 1024 * 1024; // 256 MB
config.timeout = std::chrono::seconds(30);

auto task = graph.add_task(config);
```

## Task Removal Methods

### remove_task()

Removes a task from the graph.

```cpp
bool remove_task(uint64_t task_id);
```

**Parameters:**
- `task_id`: ID of the task to remove

**Returns:**
- `true` if task was removed, `false` if not found

**Example:**
```cpp
auto task = graph.add_task(DeviceType::CPU, "TemporaryTask");
if (graph.remove_task(task->id())) {
    std::cout << "Task removed" << std::endl;
}
```

**Note:** Removing a task also removes all its dependencies.

## Dependency Methods

### add_dependency()

Adds a dependency between two tasks.

```cpp
Error add_dependency(uint64_t predecessor_id, uint64_t successor_id);
```

**Parameters:**
- `predecessor_id`: ID of the task that must complete first
- `successor_id`: ID of the task that depends on the predecessor

**Returns:**
- `Error` object indicating success or failure

**Example:**
```cpp
auto taskA = graph.add_task(DeviceType::CPU, "TaskA");
auto taskB = graph.add_task(DeviceType::GPU, "TaskB");

Error err = graph.add_dependency(taskA->id(), taskB->id());
if (!err.ok()) {
    std::cerr << "Failed to add dependency: " << err.message() << std::endl;
}
```

**Errors:**
- `CycleDetected`: Adding this dependency would create a cycle
- `TaskNotFound`: One or both task IDs are invalid

### remove_dependency()

Removes a dependency between two tasks.

```cpp
Error remove_dependency(uint64_t predecessor_id, uint64_t successor_id);
```

**Parameters:**
- `predecessor_id`: ID of the predecessor task
- `successor_id`: ID of the successor task

**Returns:**
- `Error` object indicating success or failure

### has_dependency()

Checks if a dependency exists between two tasks.

```cpp
bool has_dependency(uint64_t successor_id, uint64_t predecessor_id) const;
```

**Parameters:**
- `successor_id`: ID of the successor task
- `predecessor_id`: ID of the predecessor task

**Returns:**
- `true` if dependency exists, `false` otherwise

**Example:**
```cpp
if (graph.has_dependency(taskB->id(), taskA->id())) {
    std::cout << "TaskB depends on TaskA" << std::endl;
}
```

## Query Methods

### get_predecessors()

Gets all predecessor tasks of a given task.

```cpp
std::vector<uint64_t> get_predecessors(uint64_t task_id) const;
```

**Parameters:**
- `task_id`: ID of the task

**Returns:**
- Vector of predecessor task IDs

**Example:**
```cpp
auto predecessors = graph.get_predecessors(taskB->id());
std::cout << "TaskB has " << predecessors.size() << " predecessors" << std::endl;
for (uint64_t pred_id : predecessors) {
    auto pred_task = graph.get_task(pred_id);
    std::cout << "  - " << pred_task->get_name() << std::endl;
}
```

### get_successors()

Gets all successor tasks of a given task.

```cpp
std::vector<uint64_t> get_successors(uint64_t task_id) const;
```

**Parameters:**
- `task_id`: ID of the task

**Returns:**
- Vector of successor task IDs

### get_task()

Gets a task by its ID.

```cpp
std::shared_ptr<Task> get_task(uint64_t task_id) const;
```

**Parameters:**
- `task_id`: ID of the task

**Returns:**
- Shared pointer to the Task, or nullptr if not found

## Graph Property Methods

### num_tasks()

Returns the number of tasks in the graph.

```cpp
size_t num_tasks() const;
```

**Example:**
```cpp
std::cout << "Total tasks: " << graph.num_tasks() << std::endl;
```

### num_dependencies()

Returns the number of dependencies in the graph.

```cpp
size_t num_dependencies() const;
```

### topological_sort()

Returns a topological ordering of tasks.

```cpp
std::vector<uint64_t> topological_sort() const;
```

**Returns:**
- Vector of task IDs in execution order

**Example:**
```cpp
auto order = graph.topological_sort();
std::cout << "Execution order:" << std::endl;
for (uint64_t task_id : order) {
    auto task = graph.get_task(task_id);
    std::cout << "  " << task->get_name() << std::endl;
}
```

### get_critical_path()

Returns the critical path (longest dependency chain).

```cpp
std::vector<uint64_t> get_critical_path() const;
```

**Returns:**
- Vector of task IDs on the critical path

**Example:**
```cpp
auto critical_path = graph.get_critical_path();
std::cout << "Critical path length: " << critical_path.size() << " tasks" << std::endl;
```

### find_orphan_tasks()

Finds tasks with no predecessors or successors.

```cpp
std::vector<uint64_t> find_orphan_tasks() const;
```

**Returns:**
- Vector of orphan task IDs

## Validation Methods

### validate()

Validates the entire graph.

```cpp
Error validate() const;
```

**Returns:**
- `Error` object indicating success or failure

**Checks:**
- No cycles exist
- All tasks have valid configurations
- All dependencies reference valid tasks
- No orphan tasks (optional warning)

**Example:**
```cpp
Error err = graph.validate();
if (!err.ok()) {
    std::cerr << "Graph validation failed: " << err.message() << std::endl;
}
```

### has_cycles()

Checks if the graph contains cycles.

```cpp
bool has_cycles() const;
```

**Returns:**
- `true` if cycles exist, `false` otherwise

### detect_cycles()

Detects and returns all cycles in the graph.

```cpp
std::vector<uint64_t> detect_cycles() const;
```

**Returns:**
- Vector of task IDs forming a cycle (empty if no cycles)

## Execution Info Methods

### get_ready_tasks()

Gets all tasks that are ready for execution.

```cpp
std::vector<uint64_t> get_ready_tasks() const;
```

**Returns:**
- Vector of task IDs that are ready (all predecessors completed)

### is_task_ready()

Checks if a task is ready for execution.

```cpp
bool is_task_ready(uint64_t task_id) const;
```

**Returns:**
- `true` if all predecessors completed, `false` otherwise

### is_task_completed()

Checks if a task has completed successfully.

```cpp
bool is_task_completed(uint64_t task_id) const;
```

### is_task_failed()

Checks if a task has failed.

```cpp
bool is_task_failed(uint64_t task_id) const;
```

## Reset Methods

### reset()

Resets the entire graph (removes all tasks and dependencies).

```cpp
void reset();
```

### reset_task_status()

Resets only task statuses without removing tasks or dependencies.

```cpp
void reset_task_status();
```

**Example:**
```cpp
// First execution
scheduler.init(&graph);
scheduler.execute();
scheduler.wait_for_completion();

// Reset statuses for re-execution
graph.reset_task_status();

// Re-execute
scheduler.init(&graph);
scheduler.execute();
scheduler.wait_for_completion();
```

## Complete Example

```cpp
#include <hts/task_graph.hpp>

using namespace hts;

int main() {
    TaskGraph graph;
    
    // Create tasks
    auto load_data = graph.add_task(DeviceType::CPU, "LoadData");
    auto preprocess = graph.add_task(DeviceType::CPU, "Preprocess");
    auto gpu_compute = graph.add_task(DeviceType::GPU, "GPUCompute");
    auto save_results = graph.add_task(DeviceType::CPU, "SaveResults");
    
    // Configure tasks
    load_data->set_cpu_function(load_data_func);
    preprocess->set_cpu_function(preprocess_func);
    gpu_compute->set_gpu_function(gpu_kernel);
    save_results->set_cpu_function(save_func);
    
    // Add dependencies
    graph.add_dependency(load_data->id(), preprocess->id());
    graph.add_dependency(preprocess->id(), gpu_compute->id());
    graph.add_dependency(gpu_compute->id(), save_results->id());
    
    // Validate
    Error err = graph.validate();
    if (!err.ok()) {
        std::cerr << "Invalid graph: " << err.message() << std::endl;
        return 1;
    }
    
    // Inspect
    std::cout << "Tasks: " << graph.num_tasks() << std::endl;
    std::cout << "Dependencies: " << graph.num_dependencies() << std::endl;
    
    auto critical_path = graph.get_critical_path();
    std::cout << "Critical path: " << critical_path.size() << " tasks" << std::endl;
    
    return 0;
}
```

## Related Pages

- [Scheduler API](/api/scheduler) — Scheduler reference
- [TaskBuilder API](/api/task-builder) — TaskBuilder reference
- [Task Graph Guide](/guide/task-graph) — Task guide
