# 示例

这里列出的都是 `examples/` 目录中真实存在、并且默认会参与构建的示例程序。

## 构建

```bash
scripts/build.sh --cpu-only
```

可执行文件会输出到 `build/` 目录。

## CPU-only 示例

| 文件 | 可执行文件 | 作用 |
|------|------------|------|
| `examples/simple_dag.cpp` | `./build/simple_dag` | 最小 fork-join DAG 与统计输出 |
| `examples/parallel_pipeline.cpp` | `./build/parallel_pipeline` | 加载 / 并行处理 / 聚合流水线 |
| `examples/error_handling.cpp` | `./build/error_handling` | 失败回调流程 |
| `examples/fluent_api.cpp` | `./build/fluent_api` | `TaskBuilder` 用法 |
| `examples/task_groups.cpp` | `./build/task_groups` | TaskGroup 建图方式 |
| `examples/profiling.cpp` | `./build/profiling` | 调度统计与分析输出 |
| `examples/scheduling_policies.cpp` | `./build/scheduling_policies` | 内建策略切换 |
| `examples/graph_visualization.cpp` | `./build/graph_visualization` | JSON / DOT 导出 |
| `examples/advanced_features.cpp` | `./build/advanced_features` | barrier、future、event、resource limit |

## CUDA 专用示例

| 文件 | 可执行文件 | 说明 |
|------|------------|------|
| `examples/gpu_computation.cu` | `./build/gpu_computation` | 需要 CUDA 构建，即运行不带 `--cpu-only` 的 `scripts/build.sh` |

## 推荐顺序

1. 先看 `simple_dag`。
2. 再看 `parallel_pipeline`。
3. 需要特定功能时，再查 `profiling`、`scheduling_policies`、`graph_visualization` 等定向示例。
