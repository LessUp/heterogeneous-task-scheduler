# 内存管理

> HTS 中高效的 GPU 内存分配和管理

---

## 目录

- [概览](#概览)
- [伙伴系统分配器](#伙伴系统分配器)
- [内存池配置](#内存池配置)
- [分配策略](#分配策略)
- [最佳实践](#最佳实践)
- [调试内存问题](#调试内存问题)
- [高级主题](#高级主题)

---

## 概览

HTS 使用专门的内存池进行 GPU 内存分配，以避免频繁的 `cudaMalloc`/`cudaFree` 调用开销。该池使用**伙伴系统分配器**，提供：

- **O(log n)** 分配和释放
- **O(1)** 相邻空闲块合并
- 最小化外部碎片

### 为什么使用内存池？

| 指标 | cudaMalloc | HTS 内存池 |
|------|------------|-----------|
| 分配时间 | ~10-100 μs | ~1 μs |
| 释放时间 | ~10-50 μs | ~1 μs |
| 同步 | 需要 | 可选 |
| 碎片 | N/A | 有界 |

---

## 伙伴系统分配器

### 工作原理

1. **初始状态**：一个大的空闲块
2. **分配**：分割块直到达到合适大小
3. **释放**：与伙伴块合并（如也为空闲）
4. **块大小**：2 的幂（64B, 128B, 256B, ...）

---

## 内存池配置

### 基础配置

```cpp
#include <hts/scheduler.hpp>

SchedulerConfig config;

// 总内存池大小（默认：256 MB）
config.memory_pool_size = 512 * 1024 * 1024;  // 512 MB

// 允许池在需要时增长
config.allow_memory_growth = true;

Scheduler scheduler(config);
```

### 大小指南

| 应用类型 | 推荐池大小 | 说明 |
|---------|-----------|------|
| 小型模型 | 256-512 MB | 移动端/边缘设备 |
| 中型模型 | 1-2 GB | 桌面工作站 |
| 大型模型 | 4-8 GB | 服务器 GPU |
| 动态工作负载 | 从小开始，启用增长 | allow_memory_growth = true |

---

## 分配策略

### 1. 自动分配（推荐）

TaskContext 自动管理内存：

```cpp
auto task = scheduler.graph().add_task(DeviceType::GPU);

task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    // 通过上下文分配
    void* buffer = ctx.allocate_gpu_memory(1024 * 1024);  // 1MB
    
    // 使用 buffer...
    
    // 任务完成时自动释放
});
```

**优势**：
- 自动生命周期管理
- 异常时自动清理
- 无内存泄漏

### 2. 手动分配（高级）

直接访问内存池：

```cpp
auto& pool = scheduler.memory_pool();

// 分配
void* mem = pool.allocate(size);

// 使用...

// 必须手动释放
pool.deallocate(mem, size);
```

### 3. RAII 包装器

```cpp
#include <hts/cuda_utils.hpp>

// 设备内存（GPU）
hts::DeviceMemory<float> d_data(pool.allocate<float>(1024), 1024);
// 超出作用域时自动释放

// 固定内存（CPU，页锁定）
hts::PinnedMemory<float> h_data(1024);
// 快速的 CPU↔GPU 传输
```

---

## 最佳实践

### 1. 大小对齐

请求 64 字节对齐的大小以获得最佳性能：

```cpp
// 好：对齐到缓存行
size_t size = ((requested_size + 63) / 64) * 64;

// 更好：使用辅助函数
size_t aligned_size = align_up(requested_size, 64);
```

### 2. 重用缓冲区

避免重复分配：

```cpp
// 不好：在循环中分配
for (int i = 0; i < 1000; ++i) {
    void* temp = ctx.allocate_gpu_memory(1024);
    // 使用 temp...
}

// 好：分配一次，重复使用
void* buffer = ctx.allocate_gpu_memory(1024);
for (int i = 0; i < 1000; ++i) {
    // 重用 buffer...
}
```

### 3. 池大小规划

估算最大并发内存使用量：

```cpp
// 示例：图像处理流水线
// - 输入缓冲区：4K RGB = 3840*2160*3 = ~24 MB
// - 工作缓冲区：相同 = 24 MB
// - 输出缓冲区：相同 = 24 MB
// 每帧总计：~72 MB
// 4 帧同时处理：288 MB

SchedulerConfig config;
config.memory_pool_size = 300 * 1024 * 1024;  // 300 MB
```

### 4. 数据传输入优化

```cpp
// 使用固定内存加快传输
hts::PinnedMemory<float> pinned(1024);

// 异步拷贝（非阻塞）
cudaMemcpyAsync(d_data.get(), pinned.get(), size, 
                cudaMemcpyHostToDevice, stream);
```

---

## 调试内存问题

### 启用内存追踪

```cpp
// 在调试构建中
#define HTS_DEBUG_MEMORY 1

// 池将追踪分配
auto stats = pool.get_detailed_stats();
for (const auto& alloc : stats.allocations) {
    std::cout << "块: " << alloc.ptr 
              << " 大小: " << alloc.size
              << " 任务: " << alloc.task_id << "\n";
}
```

### 内存泄漏检测

```cpp
{
    Scheduler scheduler;
    
    auto before = scheduler.memory_pool().get_stats();
    
    // ... 运行工作负载 ...
    
    auto after = scheduler.memory_pool().get_stats();
    
    if (after.used_bytes > before.used_bytes) {
        std::cerr << "潜在泄漏: " 
                  << (after.used_bytes - before.used_bytes) 
                  << " 字节\n";
    }
}
```

### 内存不足处理

```cpp
task->set_gpu_function([](TaskContext& ctx, cudaStream_t stream) {
    try {
        void* large_buffer = ctx.allocate_gpu_memory(1024 * 1024 * 100);  // 100MB
        // ... 使用 buffer ...
    } catch (const MemoryPool::OutOfMemory& e) {
        // 优雅处理 OOM
        ctx.set_error("GPU 内存不足");
        return;
    }
});
```

---

## 高级主题

### 内存压力处理

```cpp
class MemoryPressurePolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        // 如果 GPU 内存不足，优先使用 CPU
        if (status.free_gpu_memory < 100 * 1024 * 1024) {  // < 100MB
            return DeviceType::CPU;
        }
        return DeviceType::GPU;
    }
};

scheduler.set_policy(std::make_unique<MemoryPressurePolicy>());
```

---

## 性能对比

### 分配基准测试

```cpp
constexpr size_t NUM_ALLOCS = 100000;
constexpr size_t ALLOC_SIZE = 1024;

// cudaMalloc 基线
auto start = std::chrono::high_resolution_clock::now();
for (size_t i = 0; i < NUM_ALLOCS; ++i) {
    void* ptr;
    cudaMalloc(&ptr, ALLOC_SIZE);
    cudaFree(ptr);
}
auto cuda_time = std::chrono::high_resolution_clock::now() - start;

// HTS 内存池
MemoryPool pool(256 * 1024 * 1024);
start = std::chrono::high_resolution_clock::now();
for (size_t i = 0; i < NUM_ALLOCS; ++i) {
    void* ptr = pool.allocate(ALLOC_SIZE);
    pool.deallocate(ptr, ALLOC_SIZE);
}
auto pool_time = std::chrono::high_resolution_clock::now() - start;

std::cout << "cudaMalloc: " 
          << std::chrono::duration_cast<std::chrono::microseconds>(cuda_time).count()
          << " μs\n";
std::cout << "MemoryPool: " 
          << std::chrono::duration_cast<std::chrono::microseconds>(pool_time).count()
          << " μs\n";
```

典型结果：
- `cudaMalloc`/`cudaFree`: ~50-100 μs 每次操作
- HTS 内存池: ~1-2 μs 每次操作
- **加速：50-100 倍**
