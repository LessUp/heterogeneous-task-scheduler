# 性能分析

> 监控和优化 HTS 应用性能

---

## 目录

- [概览](#概览)
- [启用性能分析](#启用性能分析)
- [分析摘要](#分析摘要)
- [时间线可视化](#时间线可视化)
- [性能指标](#性能指标)
- [优化指南](#优化指南)
- [高级分析](#高级分析)

---

## 概览

HTS 提供全面的性能分析功能，帮助您理解和优化应用：

- 所有任务的**执行计时**
- 衡量 CPU/GPU 利用率的**并行度指标**
- **内存使用**追踪
- 可视化的 **Chrome 追踪**导出
- 应用特定的 **自定义事件**指标

---

## 启用性能分析

### 基础使用

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>

int main() {
    Scheduler scheduler;
    
    // 启用性能分析
    scheduler.set_profiling(true);
    
    // 构建并执行您的任务图
    // ...
    
    scheduler.execute();
    
    // 访问分析器
    Profiler& profiler = scheduler.profiler();
    
    return 0;
}
```

### 配置

```cpp
SchedulerConfig config;
config.enable_profiling = true;

// 可选：详细内存追踪（少量开销）
config.detailed_memory_profiling = true;

Scheduler scheduler(config);
```

---

## 分析摘要

### 生成摘要

```cpp
scheduler.execute();

auto summary = scheduler.profiler().generate_summary();

std::cout << "==== 执行摘要 ====\n"
          << "总时间: " << summary.total_time.count() / 1e6 << " ms\n"
          << "CPU 时间: " << summary.cpu_time.count() / 1e6 << " ms\n"
          << "GPU 时间: " << summary.gpu_time.count() / 1e6 << " ms\n"
          << "并行度: " << summary.parallelism << "x\n"
          << "执行任务: " << summary.task_count << "\n"
          << "CPU 任务: " << summary.cpu_task_count << "\n"
          << "GPU 任务: " << summary.gpu_task_count << "\n";
```

### 摘要结构

```cpp
struct ProfileSummary {
    // 计时
    std::chrono::nanoseconds total_time;      // 实际耗时
    std::chrono::nanoseconds cpu_time;        // 所有 CPU 任务时间总和
    std::chrono::nanoseconds gpu_time;        // 所有 GPU 任务时间总和
    
    // 效率
    double parallelism;                       // (cpu_time + gpu_time) / total_time
    
    // 计数
    size_t task_count;
    size_t cpu_task_count;
    size_t gpu_task_count;
    size_t completed_count;
    size_t failed_count;
    size_t retried_count;
    
    // 内存
    size_t peak_memory_bytes;
    size_t total_allocations;
    
    // 每任务统计
    std::vector<TaskProfile> task_profiles;
};
```

### 结果解读

| 指标 | 良好 | 较差 | 行动 |
|------|------|------|------|
| 并行度 | >2.0 | <1.5 | 增加并行性 |
| CPU/GPU 比率 | 均衡 | 任一空闲 | 调整调度策略 |
| 失败任务 | 0% | >5% | 改进错误处理 |
| GPU 利用率 | >80% | <50% | 将更多工作移至 GPU |

### 详细报告

```cpp
std::cout << scheduler.profiler().generate_report();
```

示例输出：
```
=== HTS 分析报告 ===
生成时间: 2026-04-16 10:30:15

概览
----
总时间: 125.4 ms
并行度: 3.2x
任务数: 50 (CPU: 30, GPU: 20)

最慢的 5 个任务
-------------------
#1 "HeavyCompute" (GPU) : 45.2 ms
#2 "DataLoad" (CPU)     : 23.1 ms
#3 "Preprocess" (CPU)   : 12.5 ms
#4 "Transform" (GPU)    : 8.7 ms
#5 "Finalize" (CPU)     : 5.3 ms

设备利用率
----------
CPU: ████████████████████░░░░░ 78%
GPU: █████████████████░░░░░░░░ 65%

内存
----
峰值使用: 245.8 MB
分配次数: 120
碎片率: 2.3%
```

---

## 时间线可视化

### Chrome 追踪格式

导出分析数据用于 Chrome 的 `chrome://tracing`：

```cpp
scheduler.execute();

// 导出时间线
scheduler.profiler().export_timeline("profile.json");
```

### 查看

1. 打开 Chrome 浏览器
2. 导航到 `chrome://tracing`
3. 点击 "加载" 并选择 `profile.json`

### 可视化分析

```
CPU 线程 1:  [Task1=======]        [Task3===]
CPU 线程 2:           [Task2=======]
GPU 流 0:    [GPU1==============][GPU2====]

时间 →
```

**关注：**
- **间隙**：表示同步或空闲时间
- **重叠**：展示并行执行
- **长任务**：优化候选

---

## 性能指标

### 任务级指标

```cpp
struct TaskProfile {
    TaskId id;
    std::string name;
    DeviceType device;
    
    // 计时
    std::chrono::nanoseconds queued_time;
    std::chrono::nanoseconds start_time;
    std::chrono::nanoseconds end_time;
    std::chrono::nanoseconds duration;
    
    // 资源使用
    size_t memory_allocated;
    size_t memory_freed;
    
    // 状态
    TaskState final_state;
    size_t retry_count;
};
```

### 访问任务分析

```cpp
auto summary = profiler.generate_summary();

// 查找最慢的任务
std::vector<TaskProfile> sorted = summary.task_profiles;
std::sort(sorted.begin(), sorted.end(),
    [](const auto& a, const auto& b) {
        return a.duration > b.duration;
    });

for (const auto& tp : sorted | std::views::take(5)) {
    std::cout << tp.name << ": " 
              << tp.duration.count() / 1e6 << " ms\n";
}
```

### 自定义事件

```cpp
// 在任务代码中记录自定义事件
task->set_cpu_function([](TaskContext& ctx) {
    ctx.profiler().begin_event("DataLoading");
    load_data();
    ctx.profiler().end_event("DataLoading");
    
    ctx.profiler().begin_event("Processing");
    process();
    ctx.profiler().end_event("Processing");
});
```

---

## 优化指南

### 步骤 1：识别瓶颈

```cpp
void analyze_bottlenecks(const ProfileSummary& summary) {
    // 查找关键路径
    auto critical_time = std::chrono::nanoseconds::zero();
    for (const auto& tp : summary.task_profiles) {
        if (tp.duration > critical_time) {
            critical_time = tp.duration;
            std::cout << "关键路径任务: " << tp.name << "\n";
        }
    }
    
    // 检查设备平衡
    double cpu_ratio = static_cast<double>(summary.cpu_time.count()) / 
                       (summary.cpu_time.count() + summary.gpu_time.count());
    std::cout << "CPU/GPU 比率: " << cpu_ratio * 100 << "% / "
              << (1 - cpu_ratio) * 100 << "%\n";
    
    // 检查空闲时间
    double ideal_time = summary.cpu_time.count() + summary.gpu_time.count();
    double idle_ratio = 1.0 - (ideal_time / summary.total_time.count());
    std::cout << "空闲时间: " << idle_ratio * 100 << "%\n";
}
```

### 步骤 2：常见优化

#### A. 增加并行性

```cpp
// 之前：串行
for (int i = 0; i < 100; ++i) {
    auto t = graph.add_task();
    // ... （无并行）
}

// 之后：并行批次
int batch_size = 10;
for (int i = 0; i < 100; i += batch_size) {
    TaskGroup batch("batch_" + std::to_string(i), graph);
    for (int j = i; j < std::min(i + batch_size, 100); ++j) {
        auto t = batch.add_task(DeviceType::Any);
        // ...
    }
}
```

#### B. 优化数据传输

```cpp
// 之前：每批都传输
for (auto& batch : batches) {
    auto t = graph.add_task(DeviceType::GPU);
    t->set_gpu_function([&batch](auto& ctx, auto stream) {
        // 每次迭代都 H2D 拷贝
        cudaMemcpyAsync(d_data, batch.data(), ...);
        kernel<<<...>>>(d_data);
    });
}

// 之后：持久化 GPU 内存
auto t = graph.add_task(DeviceType::CPU);
t->set_cpu_function([](auto& ctx) {
    void* d_data = ctx.allocate_gpu_memory(total_size);
    cudaMemcpyAsync(d_data, all_data, ...);
    ctx.set_output("gpu_data", d_data, total_size);
});
```

---

## 进一步阅读

- [API 参考](api-reference.md) - 分析器 API 详情
- [架构概览](architecture.md) - 系统设计
- [示例](examples.md) - 分析示例
