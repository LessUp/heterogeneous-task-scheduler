# ADR-0005: CUDA Stream Management

## Status
Accepted

## Context
现代 GPU 支持多流并发执行，但：
- **单流局限**: 默认单流无法充分利用 GPU
- **计算与传输重叠**: 内存传输和计算可在不同流并行
- **多核并发**: 不同 SM 可同时执行不同 kernel

然而，多流管理带来挑战：
- 同步复杂性：跨流依赖需要显式同步
- 资源竞争：过多流可能导致反效果
- 编程难度：用户需理解 CUDA 流语义

## Decision
采用 **多流并发 + 事件同步** 策略：

### 流分配策略
```python
class StreamManager:
    def __init__(self, num_streams: int = 4):
        self.streams = [cuda.Stream() for _ in range(num_streams)]
        self.event_pool = []

    def get_stream(self, task: Task) -> cuda.Stream:
        """根据任务类型分配流"""
        if task.is_memory_bound():
            return self.h2d_stream  # 传输专用流
        elif task.is_compute_bound():
            return self.compute_streams[task.priority]
        else:
            return self.default_stream
```

### 事件同步机制
```python
class EventSynchronizer:
    def record_event(self, stream: cuda.Stream) -> cuda.Event:
        """记录流完成事件"""
        event = cuda.Event()
        event.record(stream)
        return event

    def wait_event(self, stream: cuda.Stream, event: cuda.Event):
        """使流等待事件完成"""
        stream.wait_event(event)
```

### 依赖处理
```
Task A (stream 0) ──event_A──┐
                             ├──> Task C (stream 2) waits event_A, event_B
Task B (stream 1) ──event_B──┘
```

### 默认配置
- 2 计算流 + 1 传输流 + 1 默认流
- 最大并发 kernel 数根据 GPU SM 数动态调整

## Consequences

### Positive
- **更高 GPU 利用率**: 传输与计算重叠
- **延迟隐藏**: 长操作不阻塞短操作
- **灵活控制**: 用户可指定任务到特定流

### Negative
- **同步复杂性**: 需要正确管理事件依赖
- **调试困难**: 多流问题难以复现
- **资源开销**: 每个流有一定管理开销

### Neutral
- 需要文档说明流选择策略
- 用户可选择单流模式简化使用
- 性能调优需要考虑具体硬件
