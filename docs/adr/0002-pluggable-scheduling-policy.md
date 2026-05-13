# ADR-0002: Pluggable Scheduling Policy

## Status
Accepted

## Context
不同的使用场景需要不同的调度策略：
- **批处理场景**: 最小化总完成时间，优先执行耗时长的任务
- **交互场景**: 最小化响应时间，优先执行短任务
- **资源敏感场景**: 根据资源可用性动态调整
- **优先级场景**: 用户指定的优先级覆盖默认策略

硬编码单一调度策略无法满足这些需求，同时违反开闭原则。

## Decision
采用 **策略模式** 实现可插拔调度策略：

### 核心接口
```python
class SchedulingPolicy(Protocol):
    """调度策略协议"""

    def select(self, ready_tasks: List[Task], context: SchedulingContext) -> Task:
        """从就绪任务中选择下一个执行的任务"""
        ...

    def prioritize(self, tasks: List[Task]) -> List[Task]:
        """对任务列表进行优先级排序"""
        ...
```

### 内置策略
1. **FIFOPolicy**: 先进先出，简单公平
2. **SJFPolicy**: 短作业优先，适合交互场景
3. **LJFPolicy**: 长作业优先，适合批处理场景
4. **PriorityPolicy**: 用户指定优先级
5. **ResourceAwarePolicy**: 考虑资源可用性

### 扩展机制
用户可通过继承 `SchedulingPolicy` 实现自定义策略：
```python
class CustomPolicy(SchedulingPolicy):
    def select(self, ready_tasks, context):
        # 自定义调度逻辑
        return selected_task
```

## Consequences

### Positive
- **开闭原则**: 新增策略无需修改现有代码
- **用户可扩展**: 用户可实现自定义策略
- **易于测试**: 每个策略独立测试
- **运行时切换**: 可根据场景动态更换策略
- **组合策略**: 可组合多个策略（如 PriorityPolicy + SJFPolicy）

### Negative
- **抽象开销**: 简单场景可能过度设计
- **选择困难**: 用户需要理解各策略特点
- **性能差异**: 不同策略性能特征不同，需要文档说明

### Neutral
- 默认策略选择需要明确（当前为 FIFO）
- 策略间的组合规则需要文档化
