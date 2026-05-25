# Guide

This site now documents only the maintained workflow: build, validate, understand the runtime layout,
and run the checked-in examples. Anything speculative or not backed by the current repository surface
was removed.

## Development baseline

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

Use this CPU-first baseline for local work and CI-style validation. CUDA-specific testing is additive,
not the default contributor path.

## Stable concepts

| Concept | What it does | Primary files |
|--------|---------------|---------------|
| `TaskGraph` | Stores tasks and dependencies as a DAG | `include/hts/task_graph.hpp`, `src/core/task_graph.cpp` |
| `TaskBuilder` | Fluent task creation on top of `TaskGraph` | `include/hts/task_builder.hpp` |
| `Scheduler` | Validates the graph, schedules ready tasks, records stats and timeline data | `include/hts/scheduler.hpp`, `src/core/scheduler.cpp` |
| `ExecutionEngine` | Dispatches work to CPU workers and CUDA streams | `include/hts/execution_engine.hpp`, `src/core/execution_engine.cpp` |
| `MemoryPool` / `StreamManager` | GPU services with CPU-only stubs available for validation | `include/hts/memory_pool.hpp`, `include/hts/stream_manager.hpp`, `src/cuda/` |

## Documentation boundaries

- The public API overview intentionally mirrors the headers under `include/hts/`.
- The examples page documents only the programs that actually exist under `examples/`.
- Release history lives only in the root [`CHANGELOG.md`](https://github.com/AICL-Lab/heterogeneous-task-scheduler/blob/main/CHANGELOG.md).

## Continue with

1. [Installation](/en/guide/installation) for build modes and prerequisites.
2. [Quick Start](/en/guide/quickstart) for a minimal CPU-only DAG.
3. [Architecture](/en/guide/architecture) for the execution flow and file layout.
