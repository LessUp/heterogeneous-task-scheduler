# ADR-0004: Buddy Memory Pool

## Status
Accepted

## Context
GPU 内存管理面临以下挑战：
- **分配开销**: `cudaMalloc` 每次调用有显著开销
- **碎片化**: 频繁分配释放导致内存碎片
- **大小不确定**: 任务内存需求在编译时不可预知
- **并发访问**: 多任务并发需要线程安全分配

直接使用系统分配器 (`cudaMalloc`/`cudaFree`) 导致：
- 性能瓶颈（分配延迟累积）
- 内存利用率低（碎片化）
- OOM 错误（碎片化导致无法分配大块）

## Decision
实现 **Buddy System 内存池** 进行 GPU 内存管理：

### Buddy System 原理
- 将内存划分为 2 的幂次大小的块
- 分配时找到最小满足需求的块，必要时分裂
- 释放时合并相邻伙伴块，减少碎片

### 数据结构
```python
class BuddyPool:
    def __init__(self, total_size: int, min_block: int = 256):
        self.total_size = total_size  # 总内存大小
        self.min_block = min_block    # 最小块大小
        self.free_lists = {...}       # 按大小组织的空闲块列表

    def allocate(self, size: int) -> Block:
        # 1. 计算需要的块大小 (>=size 的最小 2 的幂)
        # 2. 从对应 free_list 取块，或分裂更大块
        # 3. 返回块地址

    def deallocate(self, block: Block) -> None:
        # 1. 将块放回 free_list
        # 2. 尝试与伙伴块合并
```

### 复杂度
- 分配: O(log n)，n = 块数量
- 释放: O(log n)
- 内部碎片: 最多 50%（块大小 - 请求大小）

## Consequences

### Positive
- **O(log n) 复杂度**: 快速分配释放
- **低碎片化**: 自动合并消除外部碎片
- **可预测性**: 分配时间有界，适合实时场景
- **统计友好**: 易于实现内存统计和监控

### Negative
- **内部碎片**: 最多 50% 内部碎片浪费
- **固定总大小**: 需要预先分配大块内存
- **不适应小对象**: 非常小的分配浪费空间

### Neutral
- min_block 大小需要调优（当前默认 256 bytes）
- 需要权衡碎片率和内存利用率
- 可与其他分配器组合使用（小对象用 slab）
