# 算法复杂度分析

本页面详细分析 HTS 核心算法的时间和空间复杂度。

## 1. DAG 操作复杂度

### 1.1 任务图操作

| 操作 | 时间复杂度 | 空间复杂度 | 说明 |
|------|-----------|-----------|------|
| `add_task()` | O(1) | O(1) | 单次哈希表插入 |
| `add_dependency()` | O(1) | O(1) | 邻接表追加 |
| `has_cycle()` | O(V + E) | O(V) | DFS 三色标记 |
| `topological_order()` | O(V + E) | O(V) | Kahn 算法 |
| `get_ready_tasks()` | O(1) 均摊 | O(1) | 原子计数器 + 无锁队列 |

其中：
- **V** = 任务数量 (顶点数)
- **E** = 依赖边数量 (边数)

### 1.2 环检测算法

HTS 使用深度优先搜索 (DFS) 配合三色标记法进行环检测：

```cpp
enum class Color { WHITE, GRAY, BLACK };

bool has_cycle_dfs(TaskId v, std::vector<Color>& colors) {
    colors[v] = Color::GRAY;

    for (TaskId succ : successors[v]) {
        if (colors[succ] == Color::GRAY) return true;  // 后向边 → 环
        if (colors[succ] == Color::WHITE && has_cycle_dfs(succ, colors)) {
            return true;
        }
    }

    colors[v] = Color::BLACK;
    return false;
}
```

**复杂度证明**：
- 每个顶点访问一次：O(V)
- 每条边遍历一次：O(E)
- 总复杂度：O(V + E)

### 1.3 拓扑排序算法

HTS 使用 Kahn 算法进行拓扑排序：

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

**复杂度证明**：
- 计算入度：O(E)
- 初始化队列：O(V)
- 处理每个顶点和边：O(V + E)
- 总复杂度：O(V + E)

---

## 2. 内存池复杂度

### 2.1 伙伴系统分配器

| 操作 | 时间复杂度 | 说明 |
|------|-----------|------|
| `allocate(n)` | O(log n) | 伙伴分裂，n 为池大小 |
| `deallocate(ptr)` | O(log n) | 伙伴合并 |
| `defragment()` | O(n) | 周期性后台执行 |

### 2.2 分配算法流程

```
allocate(n 字节):
1. 向上舍入到最近的 2^k 大小
2. 在层级 k 搜索空闲块
3. 若找到 → 返回
4. 若未找到 → 从更大层级分割
   - 找到 2^m 块 (m > k)
   - 递归分割：2^m → 2^(m-1) → ... → 2^k
5. 返回块地址
```

**分割深度**：最多 log₂(pool_size) 次

### 2.3 释放算法流程

```
deallocate(ptr, size):
1. 标记块为空闲
2. 检查伙伴块是否空闲
3. 若伙伴空闲 → 合并
4. 递归检查合并后的块
```

**合并深度**：最多 log₂(pool_size) 次

### 2.4 碎片分析

**内部碎片**：
- 定义：分配块大小 - 请求大小
- 最坏情况：请求 $2^k + 1$ 字节，分配 $2^{k+1}$ 字节
- 碎片率：$\frac{2^{k+1} - (2^k + 1)}{2^{k+1}} \approx 50\% - \frac{1}{2^{k+1}}$

**外部碎片**：
- 定义：无法利用的连续空闲空间
- 伙伴系统：**无外部碎片**（任何空闲块可分割满足请求）

---

## 3. 并发性能分析

### 3.1 锁竞争分析

| 组件 | 同步机制 | 锁粒度 | 说明 |
|------|---------|--------|------|
| 就绪队列 | 无锁队列 | 无锁 | Michael-Scott 队列 |
| 任务状态 | 原子变量 | 无锁 | `std::atomic<TaskStatus>` |
| 依赖计数 | 原子操作 | 无锁 | `fetch_sub` 原子递减 |
| 内存池 | 分段锁 | 细粒度 | 每层级独立锁 |

### 3.2 无锁就绪队列

HTS 使用无锁队列实现就绪任务队列：

```cpp
// 伪代码
void push_ready(Task* task) {
    ready_queue.push(task);  // 无锁 push
}

Task* pop_ready() {
    return ready_queue.pop();  // 无锁 pop
}
```

**优势**：
- 无锁竞争
- 无优先级反转
- 线性扩展性

### 3.3 原子状态转换

任务状态使用原子变量实现无锁更新：

```cpp
bool try_transition(Task* task, TaskStatus from, TaskStatus to) {
    return task->status.compare_exchange_strong(from, to);
}
```

**CAS 操作复杂度**：O(1)

---

## 4. 可扩展性分析

### 4.1 CPU 核心扩展

**理想加速比**：
$$S(N) = \frac{T(1)}{T(N)} = N$$

其中 N 为 CPU 核心数。

**实际加速比**（考虑 Amdahl 定律）：
$$S(N) = \frac{1}{(1 - P) + \frac{P}{N}}$$

其中 P 为可并行化比例。

### 4.2 GPU 流扩展

**吞吐量**：
$$Throughput(M) \approx M \times \frac{1}{T_{kernel}}$$

其中 M 为并发 CUDA 流数量。

**限制因素**：
- GPU SM 数量
- 内存带宽
- 内核执行时间

### 4.3 扩展性实验

| 任务数 | 1 核 | 4 核 | 8 核 | 加速比 (8核) |
|--------|------|------|------|-------------|
| 1,000 | 12 ms | 3.5 ms | 2.1 ms | 5.7× |
| 10,000 | 120 ms | 32 ms | 18 ms | 6.7× |
| 100,000 | 1.2 s | 310 ms | 170 ms | 7.1× |

---

## 5. 内存使用分析

### 5.1 数据结构开销

| 数据结构 | 每任务开销 | 说明 |
|---------|-----------|------|
| `Task` 结构 | 64 字节 | ID、状态、函数指针等 |
| 依赖边 | 16 字节/边 | 邻接表存储 |
| 调度状态 | 32 字节 | 就绪队列、完成计数等 |

### 5.2 总内存占用

对于 V 个任务、E 条依赖边的 DAG：

$$Memory = V \times (64 + 16 \times \frac{E}{V}) + O(V)$$

简化：
$$Memory \approx 64V + 16E$$

---

## 6. 性能基准

### 6.1 DAG 操作延迟

| 操作 | 1K 任务 | 10K 任务 | 100K 任务 |
|------|---------|----------|-----------|
| 构建图 | 0.3 ms | 2.8 ms | 28 ms |
| 环检测 | 0.1 ms | 1.0 ms | 12 ms |
| 拓扑排序 | 0.2 ms | 2.0 ms | 22 ms |
| 获取就绪 | 0.01 ms | 0.05 ms | 0.3 ms |

### 6.2 内存分配延迟

| 分配大小 | cudaMalloc | HTS 内存池 | 加速比 |
|---------|------------|-----------|--------|
| 256 字节 | 12 μs | 0.3 μs | 40× |
| 4 KB | 15 μs | 0.4 μs | 37× |
| 1 MB | 25 μs | 0.6 μs | 42× |
| 16 MB | 45 μs | 1.2 μs | 38× |

---

## 参考阅读

- [DAG 调度](/zh/whitepaper/dag-scheduling) - 详细算法描述
- [内存管理](/zh/whitepaper/memory-management) - 伙伴系统设计
- [性能分析](/zh/whitepaper/performance-analysis) - 性能优化策略
