# 错误处理

> HTS 中强大的错误管理和恢复策略

---

## 目录

- [概览](#概览)
- [错误类型](#错误类型)
- [错误回调](#错误回调)
- [重试策略](#重试策略)
- [错误传播](#错误传播)
- [最佳实践](#最佳实践)
- [高级模式](#高级模式)

---

## 概览

HTS 提供全面的错误处理系统，支持：

- 即时通知的**错误回调**
- 瞬态故障的**重试策略**
- 依赖任务的**错误传播**
- **优雅降级**和恢复

---

## 错误类型

### 任务级错误

```cpp
enum class TaskErrorType {
    None,           // 无错误
    Exception,      // 抛出 C++ 异常
    CUDAError,      // CUDA 运行时错误
    LogicError,     // 逻辑/前置条件违反
    RuntimeError,   // 一般运行时错误
    OutOfMemory,    // 内存分配失败
    Timeout,        // 超时
    Cancelled       // 任务被取消
};
```

### 调度器级错误

```cpp
enum class SchedulerErrorType {
    CycleDetected,      // 依赖图有环
    InvalidTask,        // 任务引用了无效 ID
    MemoryExhausted,    // 内存池耗尽
    DeviceUnavailable,  // 需要 GPU 但不可用
    ExecutionFailed     // 一般执行失败
};
```

---

## 错误回调

### 全局错误处理器

```cpp
Scheduler scheduler;

// 设置全局错误回调
scheduler.set_error_callback(
    [](TaskId task_id, const std::string& message, TaskErrorType type) {
        std::cerr << "[错误] 任务 " << task_id 
                  << " 失败，类型: " << error_type_name(type)
                  << ": " << message << "\n";
        
        // 额外操作：
        // - 记录到文件
        // - 发送告警
        // - 更新指标
        // - 触发恢复
    }
);
```

### 每任务错误处理器

```cpp
auto task = scheduler.graph().add_task(DeviceType::CPU);

task->set_error_callback(
    [](const Task& task, const std::string& message) {
        std::cerr << "关键任务 " << task.name() 
                  << " 失败: " << message << "\n";
        
        // 此特定任务的自定义处理
        notify_admin(task.name(), message);
    }
);
```

---

## 重试策略

### 内置重试策略

```cpp
#include <hts/retry_policy.hpp>

// 1. 不重试（默认）
task->set_retry_policy(RetryPolicyFactory::no_retry());

// 2. 固定延迟
// 重试 3 次，每次延迟 100ms
task->set_retry_policy(
    RetryPolicyFactory::fixed(3, std::chrono::milliseconds{100})
);

// 3. 指数退避
// 重试 5 次: 100ms, 200ms, 400ms, 800ms, 1600ms
task->set_retry_policy(
    RetryPolicyFactory::exponential(5, std::chrono::milliseconds{100})
);

// 4. 抖动退避
// 添加随机性(±25%)防止惊群效应
task->set_retry_policy(
    RetryPolicyFactory::jittered(5, std::chrono::milliseconds{100})
);
```

### 条件重试

```cpp
// 仅重试瞬态错误
auto retry_policy = ConditionalRetryPolicy::transient_errors(
    RetryPolicyFactory::exponential(5)
);

task->set_retry_policy(std::move(retry_policy));
```

---

## 错误传播

### 默认行为

任务失败时，所有依赖任务自动取消：

```
任务 A (失败)
   │
   ├──→ 任务 B (已取消)
   │
   └──→ 任务 C (已取消)
            │
            └──→ 任务 D (已取消)
```

---

## 最佳实践

### 1. 区分可重试与不可重试

```cpp
task->set_cpu_function([](TaskContext& ctx) {
    try {
        process_data();
    } catch (const NetworkTimeout& e) {
        // 瞬态 - 应该重试
        ctx.set_error(e.what(), true);  // retryable = true
    } catch (const InvalidData& e) {
        // 永久 - 不重试
        ctx.set_error(e.what(), false); // retryable = false
    }
});
```

### 2. 设置适当的重试限制

```cpp
// 网络请求：合理的重试次数
auto network_task = graph.add_task(DeviceType::CPU);
network_task->set_retry_policy(RetryPolicyFactory::exponential(3));

// GPU 计算：较少重试（开销大）
auto gpu_task = graph.add_task(DeviceType::GPU);
gpu_task->set_retry_policy(RetryPolicyFactory::fixed(1));

// 关键路径：不重试，快速失败
auto critical_task = graph.add_task(DeviceType::CPU);
critical_task->set_retry_policy(RetryPolicyFactory::no_retry());
```

### 3. 使用指数退避

```cpp
// 防止压垮失败的服务
auto policy = RetryPolicyFactory::exponential(
    5,                                    // 最多 5 次尝试
    std::chrono::milliseconds{100},      // 初始延迟
    std::chrono::seconds{30}             // 最大延迟上限
);
```

### 4. 记录上下文信息

```cpp
scheduler.set_error_callback([](TaskId id, const std::string& msg, 
                                 TaskErrorType type) {
    auto& task = scheduler.graph().get_task(id);
    
    std::cerr << "错误:\n"
              << "  任务: " << task.name() << " (" << id << ")\n"
              << "  类型: " << error_type_name(type) << "\n"
              << "  消息: " << msg << "\n"
              << "  尝试: " << task.retry_count() << "/" 
              << task.max_retries() << "\n"
              << "  时间戳: " << current_timestamp() << "\n";
});
```

---

## 高级模式

### 模式 1：死信队列

```cpp
class DeadLetterQueue {
    std::queue<std::pair<TaskId, std::string>> failed_tasks_;
    
public:
    void enqueue(TaskId id, const std::string& reason) {
        failed_tasks_.push({id, reason});
        persist_to_disk(id, reason);
    }
    
    void process_retries() {
        while (!failed_tasks_.empty()) {
            auto [id, reason] = failed_tasks_.front();
            failed_tasks_.pop();
            
            if (should_retry(id)) {
                retry_task(id);
            }
        }
    }
};

// 使用
DeadLetterQueue dlq;
scheduler.set_error_callback([&dlq](TaskId id, const std::string& msg, auto) {
    dlq.enqueue(id, msg);
});
```

### 模式 2：超时处理

```cpp
task->set_timeout(std::chrono::seconds{30});

task->set_cpu_function([](TaskContext& ctx) {
    // 检查取消标志
    while (!ctx.is_cancelled()) {
        do_work();
        
        // 定期检查
        if (++iterations % 100 == 0) {
            if (ctx.is_cancelled()) {
                cleanup();
                return;
            }
        }
    }
});
```

---

## 错误码参考

| 错误码 | 描述 | 可重试 |
|--------|------|--------|
| `Success` | 无错误 | - |
| `Unknown` | 未知错误 | 可能 |
| `OutOfMemory` | GPU/CPU 内存耗尽 | 是 |
| `CUDAError` | CUDA 运行时错误 | 视情况而定 |
| `NetworkError` | 网络故障 | 是 |
| `TimeoutError` | 操作超时 | 是 |
| `InvalidData` | 数据验证失败 | 否 |
| `NotFound` | 资源未找到 | 否 |
| `PermissionDenied` | 访问被拒绝 | 否 |
| `Cancelled` | 任务被取消 | 否 |
