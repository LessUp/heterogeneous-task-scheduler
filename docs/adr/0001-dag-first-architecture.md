# ADR-0001: DAG-First Architecture

## Status
Accepted

## Context
HTS (Heterogeneous Task Scheduler) 需要支持复杂依赖关系的任务调度。任务的执行顺序不是简单的 FIFO 队列，而是需要：
- 表达任务之间的依赖关系（任务 B 依赖任务 A 的输出）
- 支持并行执行无依赖的任务
- 检测循环依赖，避免死锁
- 支持动态添加任务和修改依赖关系

传统的队列调度无法表达这些复杂的依赖关系，需要一种更强大的抽象。

## Decision
采用 **DAG-first 架构**，核心设计如下：

### 分层设计
1. **Graph Layer**: 使用有向无环图 (DAG) 表示任务依赖关系
   - 节点 = 任务
   - 边 = 依赖关系
   - 提供拓扑排序、环检测等图算法

2. **Schedule Layer**: 负责任务调度决策
   - 根据 DAG 计算就绪任务（依赖已满足）
   - 应用调度策略选择执行任务
   - 管理任务状态转换

3. **Execute Layer**: 负责任务实际执行
   - 管理 CPU/GPU 执行器
   - 处理异步执行和结果收集
   - 资源分配和回收

### 核心接口
```python
class TaskGraph:
    def add_task(task, dependencies) -> None
    def get_ready_tasks() -> List[Task]
    def mark_completed(task) -> None
    def detect_cycles() -> bool
```

## Consequences

### Positive
- **清晰的抽象边界**: Graph/Schedule/Execute 各司其职，单一职责
- **易于测试**: 每层可独立测试，图算法可单元测试
- **易于扩展**: 新调度策略只需实现接口，不影响图结构
- **自然支持并行**: 拓扑排序自动识别可并行任务
- **依赖可视化**: DAG 结构易于可视化和调试

### Negative
- **内存开销**: 需要维护完整的图结构
- **构建成本**: 拓扑排序在最坏情况下 O(V+E)
- **动态更新复杂**: 运行时修改依赖需要重新验证

### Neutral
- 需要用户理解 DAG 概念才能正确使用
- 调试时需要检查图结构而非简单队列
