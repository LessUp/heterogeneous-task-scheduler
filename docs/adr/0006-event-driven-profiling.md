# ADR-0006: Event-Driven Profiling

## Status
Accepted

## Context
运行时性能分析对于优化异构任务调度至关重要：
- 识别瓶颈任务
- 分析资源利用率
- 调优调度策略
- 验证优化效果

传统方案的问题：
- **侵入式**: 直接修改代码添加计时，影响代码可读性
- **固定测量点**: 无法灵活添加新观测点
- **高开销**: 全程开启 profiling 影响生产性能

## Decision
采用 **事件系统 + Profiler 集成** 设计：

### 事件系统
```python
class Event:
    type: str           # 事件类型
    timestamp: float    # 时间戳
    data: dict          # 事件数据

class EventEmitter:
    def emit(self, event_type: str, data: dict):
        """发射事件到所有订阅者"""
        for subscriber in self.subscribers[event_type]:
            subscriber.handle(event)

    def subscribe(self, event_type: str, handler: Callable):
        """订阅特定类型事件"""
        self.subscribers[event_type].append(handler)
```

### 预定义事件类型
```python
class EventType:
    TASK_SUBMITTED = "task.submitted"
    TASK_STARTED = "task.started"
    TASK_COMPLETED = "task.completed"
    MEMORY_ALLOCATED = "memory.allocated"
    MEMORY_FREED = "memory.freed"
    STREAM_SYNC = "stream.sync"
    SCHEDULER_DECISION = "scheduler.decision"
```

### Profiler 集成
```python
class Profiler:
    def __init__(self):
        self.events = []
        self.aggregates = defaultdict(list)

    def handle(self, event: Event):
        """处理事件，记录数据"""
        self.events.append(event)
        self._update_aggregates(event)

    def get_task_stats(self, task_id: str) -> TaskStats:
        """获取任务统计信息"""
        return TaskStats(
            submit_time=...,
            start_time=...,
            complete_time=...,
            wait_time=...,  # 等待时间
            run_time=...,   # 运行时间
        )
```

### 低侵入性
```python
# 调度器代码中只需
self.events.emit(EventType.TASK_STARTED, {"task_id": task.id})
# profiler 自动订阅并处理
```

## Consequences

### Positive
- **低侵入性**: 业务代码只有 emit 调用
- **灵活扩展**: 新观测点只需添加 emit
- **可插拔**: Profiler 可选，不影响生产
- **丰富数据**: 事件流支持任意分析

### Negative
- **事件开销**: emit 有一定开销（~微秒级）
- **存储需求**: 完整事件流需要内存
- **异步问题**: 多线程 emit 需要同步

### Neutral
- 需要权衡事件粒度和开销
- 可通过采样减少开销
- 事件格式需要版本兼容
