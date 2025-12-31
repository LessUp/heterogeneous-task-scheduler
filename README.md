# Heterogeneous Task Scheduler (HTS)

一个 C++ 框架，用于在 CPU 和 GPU 之间调度和执行任务 DAG（有向无环图）。

## 特性

- **DAG 任务依赖管理**：自动检测循环依赖，确保正确的执行顺序
- **GPU 内存池**：使用伙伴系统算法，避免频繁的 cudaMalloc/cudaFree
- **异步并发执行**：CPU 线程池 + CUDA 流，最大化硬件利用率
- **负载均衡**：基于设备负载自动分配任务
- **性能监控**：执行时间统计和时间线可视化

## 依赖

- CMake >= 3.18
- CUDA Toolkit
- C++17 编译器
- Google Test (自动下载)
- RapidCheck (自动下载)

## 构建

```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

## 运行测试

```bash
cd build
ctest --output-on-failure
```

## 使用示例

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    // 创建调度器
    hts::SchedulerConfig config;
    config.memory_pool_size = 256 * 1024 * 1024;  // 256 MB
    config.cpu_thread_count = 4;
    config.gpu_stream_count = 4;
    
    hts::Scheduler scheduler(config);
    
    // 创建任务
    auto task1 = scheduler.graph().add_task(hts::DeviceType::CPU);
    auto task2 = scheduler.graph().add_task(hts::DeviceType::GPU);
    auto task3 = scheduler.graph().add_task(hts::DeviceType::Any);
    
    // 设置 CPU 任务函数
    task1->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 1 running on CPU\n";
    });
    
    // 设置 GPU 任务函数
    task2->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        // 在这里启动 CUDA kernel
        std::cout << "Task 2 running on GPU\n";
    });
    
    // 设置可在任意设备运行的任务
    task3->set_cpu_function([](hts::TaskContext& ctx) {
        std::cout << "Task 3 running on CPU\n";
    });
    task3->set_gpu_function([](hts::TaskContext& ctx, cudaStream_t stream) {
        std::cout << "Task 3 running on GPU\n";
    });
    
    // 添加依赖关系
    scheduler.graph().add_dependency(task1->id(), task2->id());
    scheduler.graph().add_dependency(task1->id(), task3->id());
    
    // 设置错误回调
    scheduler.set_error_callback([](hts::TaskId id, const std::string& msg) {
        std::cerr << "Task " << id << " failed: " << msg << "\n";
    });
    
    // 同步执行
    scheduler.execute();
    
    // 或异步执行
    // auto future = scheduler.execute_async();
    // future.get();
    
    // 获取统计信息
    auto stats = scheduler.get_stats();
    std::cout << "Total time: " << stats.total_time.count() / 1e6 << " ms\n";
    std::cout << "Memory used: " << stats.memory_stats.used_bytes << " bytes\n";
    
    // 生成时间线 JSON
    std::string timeline = scheduler.generate_timeline_json();
    
    return 0;
}
```

## 架构

```
┌─────────────────────────────────────────────────────────────────┐
│                        User Application                          │
├─────────────────────────────────────────────────────────────────┤
│                      TaskGraph Builder API                       │
├─────────────────────────────────────────────────────────────────┤
│                          Scheduler                               │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ Dependency      │  │ Device          │  │ Performance     │  │
│  │ Manager         │  │ Manager         │  │ Monitor         │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                      Execution Engine                            │
│  ┌─────────────────┐  ┌─────────────────┐  ┌─────────────────┐  │
│  │ CPU Executor    │  │ GPU Executor    │  │ Stream Manager  │  │
│  └─────────────────┘  └─────────────────┘  └─────────────────┘  │
├─────────────────────────────────────────────────────────────────┤
│                       Memory Pool                                │
└─────────────────────────────────────────────────────────────────┘
```

## 许可证

MIT License
