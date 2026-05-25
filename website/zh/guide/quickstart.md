# 快速开始

先走 CPU-only 路径。这样可以在没有 CUDA 硬件的情况下验证真实的调度器、任务图、示例和测试。

## 先运行仓库自带示例

```bash
scripts/build.sh --cpu-only
./build/simple_dag
```

对应源码是 [`examples/simple_dag.cpp`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/examples/simple_dag.cpp)。

## 使用真实 API 的最小 DAG

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    hts::Scheduler scheduler;
    hts::TaskBuilder builder(scheduler.graph());

    int left = 0;
    int right = 0;
    int total = 0;

    auto left_task = builder.name("left")
        .device(hts::DeviceType::CPU)
        .cpu([&](hts::TaskContext &) { left = 21; })
        .build();

    auto right_task = builder.name("right")
        .device(hts::DeviceType::CPU)
        .cpu([&](hts::TaskContext &) { right = 21; })
        .build();

    builder.name("join")
        .device(hts::DeviceType::CPU)
        .priority(hts::TaskPriority::High)
        .after(left_task)
        .after(right_task)
        .cpu([&](hts::TaskContext &) {
            total = left + right;
            std::cout << "total = " << total << '\n';
        })
        .build();

    scheduler.execute();
    return total == 42 ? 0 : 1;
}
```

## 这段代码做了什么

1. 创建 `Scheduler`，它内部持有当前工作用的 `TaskGraph`。
2. 使用 `TaskBuilder` 定义三个 CPU 任务。
3. 用 `.after(...)` 表达依赖关系。
4. 调用 `scheduler.execute()`，由调度器先校验图，再完成分发执行。

## 下一步

- [安装](./installation) 查看构建模式与验证命令。
- [架构](./architecture) 了解 `Scheduler`、`TaskGraph` 与执行引擎之间的关系。
- [示例](../examples/) 查看仓库内真实存在的可运行程序。
