# Examples

These are the example programs that actually exist under `examples/` and are built by default.

## Build

```bash
scripts/build.sh --cpu-only
```

Executables are emitted into `build/`.

## CPU-only examples

| File | Binary | Purpose |
|------|--------|---------|
| `examples/simple_dag.cpp` | `./build/simple_dag` | Small fork-join DAG with stats output |
| `examples/parallel_pipeline.cpp` | `./build/parallel_pipeline` | Load/process/aggregate pipeline with parallel middle stage |
| `examples/error_handling.cpp` | `./build/error_handling` | Failure callback flow |
| `examples/fluent_api.cpp` | `./build/fluent_api` | `TaskBuilder` usage |
| `examples/task_groups.cpp` | `./build/task_groups` | Grouped task construction |
| `examples/profiling.cpp` | `./build/profiling` | Scheduler profiling and report generation |
| `examples/scheduling_policies.cpp` | `./build/scheduling_policies` | Built-in policy selection |
| `examples/graph_visualization.cpp` | `./build/graph_visualization` | JSON and DOT export |
| `examples/advanced_features.cpp` | `./build/advanced_features` | Barriers, futures, events, and resource limiting |

## CUDA-only example

| File | Binary | Notes |
|------|--------|-------|
| `examples/gpu_computation.cu` | `./build/gpu_computation` | Requires a CUDA-enabled build (`scripts/build.sh` without `--cpu-only`) |

## Suggested order

1. Start with `simple_dag`.
2. Move to `parallel_pipeline`.
3. Use the focused examples (`profiling`, `scheduling_policies`, `graph_visualization`) as lookup material for specific utilities.
