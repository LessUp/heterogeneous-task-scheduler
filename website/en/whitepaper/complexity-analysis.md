# Algorithm Complexity Analysis

This page provides detailed analysis of time and space complexity for HTS core algorithms.

## 1. DAG Operation Complexity

### 1.1 Task Graph Operations

| Operation | Time Complexity | Space Complexity | Notes |
|-----------|-----------------|------------------|-------|
| `add_task()` | O(1) | O(1) | Single hash table insert |
| `add_dependency()` | O(1) | O(1) | Adjacency list append |
| `has_cycle()` | O(V + E) | O(V) | DFS with three-color marking |
| `topological_order()` | O(V + E) | O(V) | Kahn's algorithm |
| `get_ready_tasks()` | O(1) amortized | O(1) | Atomic counter + lock-free queue |

Where:
- **V** = Number of tasks (vertices)
- **E** = Number of dependency edges

### 1.2 Cycle Detection Algorithm

HTS uses Depth-First Search (DFS) with three-color marking for cycle detection:

```cpp
enum class Color { WHITE, GRAY, BLACK };

bool has_cycle_dfs(TaskId v, std::vector<Color>& colors) {
    colors[v] = Color::GRAY;

    for (TaskId succ : successors[v]) {
        if (colors[succ] == Color::GRAY) return true;  // Back edge → cycle
        if (colors[succ] == Color::WHITE && has_cycle_dfs(succ, colors)) {
            return true;
        }
    }

    colors[v] = Color::BLACK;
    return false;
}
```

**Complexity Proof**:
- Each vertex visited once: O(V)
- Each edge traversed once: O(E)
- Total complexity: O(V + E)

### 1.3 Topological Sort Algorithm

HTS uses Kahn's algorithm for topological sorting:

```cpp
std::vector<TaskId> topological_order() {
    std::vector<uint32_t> in_degree(tasks.size());
    for (auto& task : tasks) {
        for (TaskId succ : task.successors) {
            in_degree[succ]++;
        }
    }

    std::queue<TaskId> ready;
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (in_degree[i] == 0) ready.push(i);
    }

    std::vector<TaskId> order;
    while (!ready.empty()) {
        TaskId v = ready.front(); ready.pop();
        order.push_back(v);
        for (TaskId succ : tasks[v].successors) {
            if (--in_degree[succ] == 0) {
                ready.push(succ);
            }
        }
    }

    return order;
}
```

**Complexity Proof**:
- Compute in-degrees: O(E)
- Initialize queue: O(V)
- Process each vertex and edge: O(V + E)
- Total complexity: O(V + E)

---

## 2. Memory Pool Complexity

### 2.1 Buddy System Allocator

| Operation | Time Complexity | Notes |
|-----------|-----------------|-------|
| `allocate(n)` | O(log n) | Buddy splitting, n is pool size |
| `deallocate(ptr)` | O(log n) | Buddy merging |
| `defragment()` | O(n) | Periodic background execution |

### 2.2 Allocation Algorithm

```
allocate(n bytes):
1. Round up to nearest power of 2: 2^k
2. Search for free block at level k
3. If found → return
4. If not found → split from larger level
   - Find 2^m block (m > k)
   - Recursively split: 2^m → 2^(m-1) → ... → 2^k
5. Return block address
```

**Splitting depth**: At most log₂(pool_size) times

### 2.3 Deallocation Algorithm

```
deallocate(ptr, size):
1. Mark block as free
2. Check if buddy block is free
3. If buddy free → merge
4. Recursively check merged block
```

**Merging depth**: At most log₂(pool_size) times

### 2.4 Fragmentation Analysis

**Internal Fragmentation**:
- Definition: Allocated size - Requested size
- Worst case: Request $2^k + 1$ bytes, allocate $2^{k+1}$ bytes
- Fragmentation rate: $\frac{2^{k+1} - (2^k + 1)}{2^{k+1}} \approx 50\% - \frac{1}{2^{k+1}}$

**External Fragmentation**:
- Definition: Unusable scattered free space
- Buddy system: **No external fragmentation** (any free block can be split to satisfy request)

---

## 3. Concurrency Performance Analysis

### 3.1 Lock Contention Analysis

| Component | Synchronization | Lock Granularity | Notes |
|-----------|-----------------|------------------|-------|
| Ready Queue | Lock-free queue | Lock-free | Michael-Scott queue |
| Task Status | Atomic variable | Lock-free | `std::atomic<TaskStatus>` |
| Dependency Count | Atomic operation | Lock-free | `fetch_sub` atomic decrement |
| Memory Pool | Segmented locks | Fine-grained | Independent locks per level |

### 3.2 Lock-Free Ready Queue

HTS uses lock-free queue for ready tasks:

```cpp
// Pseudocode
void push_ready(Task* task) {
    ready_queue.push(task);  // Lock-free push
}

Task* pop_ready() {
    return ready_queue.pop();  // Lock-free pop
}
```

**Advantages**:
- No lock contention
- No priority inversion
- Linear scalability

### 3.3 Atomic State Transitions

Task status uses atomic variables for lock-free updates:

```cpp
bool try_transition(Task* task, TaskStatus from, TaskStatus to) {
    return task->status.compare_exchange_strong(from, to);
}
```

**CAS Operation Complexity**: O(1)

---

## 4. Scalability Analysis

### 4.1 CPU Core Scaling

**Ideal Speedup**:
$$S(N) = \frac{T(1)}{T(N)} = N$$

Where N is the number of CPU cores.

**Actual Speedup** (Amdahl's Law):
$$S(N) = \frac{1}{(1 - P) + \frac{P}{N}}$$

Where P is the parallelizable fraction.

### 4.2 GPU Stream Scaling

**Throughput**:
$$Throughput(M) \approx M \times \frac{1}{T_{kernel}}$$

Where M is the number of concurrent CUDA streams.

**Limiting Factors**:
- Number of GPU SMs
- Memory bandwidth
- Kernel execution time

### 4.3 Scalability Experiments

| Tasks | 1 Core | 4 Cores | 8 Cores | Speedup (8 cores) |
|-------|--------|---------|---------|-------------------|
| 1,000 | 12 ms | 3.5 ms | 2.1 ms | 5.7× |
| 10,000 | 120 ms | 32 ms | 18 ms | 6.7× |
| 100,000 | 1.2 s | 310 ms | 170 ms | 7.1× |

---

## 5. Memory Usage Analysis

### 5.1 Data Structure Overhead

| Data Structure | Per-Task Overhead | Notes |
|----------------|-------------------|-------|
| `Task` struct | 64 bytes | ID, status, function pointers, etc. |
| Dependency edge | 16 bytes/edge | Adjacency list storage |
| Scheduling state | 32 bytes | Ready queue, completion count, etc. |

### 5.2 Total Memory Footprint

For a DAG with V tasks and E dependency edges:

$$Memory = V \times (64 + 16 \times \frac{E}{V}) + O(V)$$

Simplified:
$$Memory \approx 64V + 16E$$

---

## 6. Performance Benchmarks

### 6.1 DAG Operation Latency

| Operation | 1K Tasks | 10K Tasks | 100K Tasks |
|-----------|----------|-----------|------------|
| Build graph | 0.3 ms | 2.8 ms | 28 ms |
| Cycle detection | 0.1 ms | 1.0 ms | 12 ms |
| Topological sort | 0.2 ms | 2.0 ms | 22 ms |
| Get ready tasks | 0.01 ms | 0.05 ms | 0.3 ms |

### 6.2 Memory Allocation Latency

| Allocation Size | cudaMalloc | HTS Memory Pool | Speedup |
|-----------------|------------|-----------------|---------|
| 256 bytes | 12 μs | 0.3 μs | 40× |
| 4 KB | 15 μs | 0.4 μs | 37× |
| 1 MB | 25 μs | 0.6 μs | 42× |
| 16 MB | 45 μs | 1.2 μs | 38× |

---

## Further Reading

- [DAG Scheduling](/en/whitepaper/dag-scheduling) - Detailed algorithm description
- [Memory Management](/en/whitepaper/memory-management) - Buddy system design
- [Performance Analysis](/en/whitepaper/performance-analysis) - Optimization strategies
