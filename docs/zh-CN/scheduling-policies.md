# 调度策略

> 理解和配置任务调度策略

---

## 目录

- [概览](#概览)
- [内置策略](#内置策略)
- [策略选择指南](#策略选择指南)
- [自定义策略](#自定义策略)
- [性能调优](#性能调优)
- [示例](#示例)

---

## 概览

HTS 使用可插拔的调度策略系统来决定哪个设备（CPU 或 GPU）执行每个任务。策略接收任务特性和系统状态，然后做出执行决策。

### 策略接口

```cpp
class SchedulingPolicy {
public:
    virtual ~SchedulingPolicy() = default;
    
    virtual DeviceType select_device(
        const Task& task,
        const SystemStatus& status
    ) = 0;
    
    virtual std::string name() const = 0;
};
```

### 系统状态

策略接收当前系统状态：

```cpp
struct SystemStatus {
    // CPU 状态
    size_t cpu_queue_depth;           // 待处理的 CPU 任务
    double cpu_utilization;           // CPU 利用率 (0-1)
    
    // GPU 状态
    size_t gpu_queue_depth;           // 待处理的 GPU 任务
    double gpu_utilization;           // GPU 利用率 (0-1)
    size_t free_gpu_memory;           // 可用 GPU 内存
    
    // 任务历史
    double avg_cpu_task_time;         // 平均 CPU 任务耗时
    double avg_gpu_task_time;         // 平均 GPU 任务耗时
};
```

---

## 内置策略

### 1. DefaultSchedulingPolicy

**描述**：基于当前系统负载和队列深度做决策。

**算法**：
1. 如果任务指定了 CPU 或 GPU，尊重该偏好
2. 比较队列深度（更短的队列获胜）
3. 考虑历史任务性能

**适用**：混合 CPU/GPU 任务的通用场景

```cpp
scheduler.set_policy(std::make_unique<DefaultSchedulingPolicy>());
```

### 2. GpuFirstPolicy

**描述**：优先使用 GPU 执行。

**算法**：
1. 如果任务有 GPU 函数，使用 GPU
2. 仅当 GPU 不可用时回退到 CPU

**适用**：GPU 密集型工作负载、计算密集型任务

```cpp
scheduler.set_policy(std::make_unique<GpuFirstPolicy>());
```

**特性**：
- 最大化 GPU 利用率
- 可能导致 CPU 利用率不足
- 适合深度学习推理、图像处理

### 3. CpuFirstPolicy

**描述**：优先使用 CPU 执行。

**算法**：
1. 如果任务有 CPU 函数，使用 CPU
2. 仅对没有 CPU 实现的任务使用 GPU

**适用**：CPU 密集型工作负载、延迟敏感型任务

```cpp
scheduler.set_policy(std::make_unique<CpuFirstPolicy>());
```

**特性**：
- 节省 GPU 内存
- 避免 CPU→GPU 数据传输开销
- 适合控制逻辑、预处理

### 4. RoundRobinPolicy

**描述**：在 CPU 和 GPU 之间交替实现负载均衡。

**算法**：
1. 追踪上次使用的设备
2. 为下个任务选择相反设备
3. 尊重任务设备约束

**适用**：需要均衡利用的场景

```cpp
scheduler.set_policy(std::make_unique<RoundRobinPolicy>());
```

**特性**：
- 工作均匀分布
- 防止任一设备空闲
- 可能导致不必要的数据传输

### 5. ShortestJobFirstPolicy

**描述**：基于预计完成时间优先执行任务。

**算法**：
1. 使用历史数据估算执行时间
2. 选择预计完成时间更短的设备

**适用**：延迟敏感、异构任务时长

```cpp
scheduler.set_policy(std::make_unique<ShortestJobFirstPolicy>());
```

**特性**：
- 最小化完工时间
- 需要历史数据（预热期）
- 适合实时处理

---

## 策略选择指南

### 决策树

```
工作负载类型？
│
├─> 主要是 GPU 计算（ML 推理、图像处理）
│   └─> GpuFirstPolicy
│
├─> 主要是 CPU 计算（解析、控制流）
│   └─> CpuFirstPolicy
│
├─> 混合 CPU/GPU 流水线
│   └─> 需要均衡利用？
│       ├─> 是 → RoundRobinPolicy
│       └─> 否 → DefaultSchedulingPolicy
│
└─> 延迟敏感、任务大小变化
    └─> ShortestJobFirstPolicy
```

### 工作负载特定推荐

| 工作负载类型 | 推荐策略 | 理由 |
|-------------|---------|------|
| 深度学习推理 | GpuFirstPolicy | 最大化 GPU 吞吐量 |
| 视频处理 | DefaultSchedulingPolicy | 平衡解码（CPU）和滤镜（GPU）|
| 数据流水线 | CpuFirstPolicy | 最小化传输，CPU 预处理 |
| 实时系统 | ShortestJobFirstPolicy | 满足延迟要求 |
| 混合计算 | RoundRobinPolicy | 资源均衡利用 |

---

## 自定义策略

### 基础自定义策略

```cpp
#include <hts/scheduling_policy.hpp>

class MyCustomPolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task, 
                             const SystemStatus& status) override {
        // 高优先级任务总是用 GPU
        if (task.priority() == TaskPriority::Critical) {
            return DeviceType::GPU;
        }
        
        // GPU 过载时使用 CPU
        if (status.gpu_queue_depth > 10) {
            return DeviceType::CPU;
        }
        
        // 默认用 GPU
        return DeviceType::GPU;
    }
    
    std::string name() const override {
        return "MyCustomPolicy";
    }
};

// 使用
scheduler.set_policy(std::make_unique<MyCustomPolicy>());
```

### 数据感知策略

```cpp
class DataAwarePolicy : public SchedulingPolicy {
public:
    DeviceType select_device(const Task& task,
                             const SystemStatus& status) override {
        // 检查数据是否已在 GPU 上
        if (task.has_gpu_input()) {
            return DeviceType::GPU;  // 留在 GPU
        }
        
        // 小数据量，CPU 可能更快
        if (task.estimated_data_size() < 1024) {
            return DeviceType::CPU;
        }
        
        // 大数据量：GPU 通常更好
        return DeviceType::GPU;
    }
    
    std::string name() const override {
        return "DataAwarePolicy";
    }
};
```

---

## 性能调优

### 测量策略有效性

```cpp
scheduler.set_profiling(true);
scheduler.execute();

auto summary = scheduler.profiler().generate_summary();

std::cout << "总时间: " << summary.total_time.count() / 1e6 << " ms\n";
std::cout << "并行度: " << summary.parallelism << "x\n";
std::cout << "CPU 任务: " << summary.cpu_task_count << "\n";
std::cout << "GPU 任务: " << summary.gpu_task_count << "\n";
```

---

## 最佳实践

1. **从默认开始**：先用 `DefaultSchedulingPolicy` 并测量
2. **先分析再优化**：不要猜测，用分析器数据指导决策
3. **考虑数据局部性**：最小化 CPU↔GPU 数据传输
4. **尊重任务提示**：如果任务指定了设备，仅在必要时覆盖
5. **监控队列深度**：防止任一设备被饿死
6. **考虑预热**：GPU 任务可能有初始开销
