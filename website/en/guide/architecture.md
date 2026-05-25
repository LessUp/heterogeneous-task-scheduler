# Architecture

This page documents the current implementation layout, not an aspirational future design.

## Runtime layers

| Layer | Main types | Implementation files |
|-------|------------|----------------------|
| Graph definition | `Task`, `TaskGraph`, `TaskBuilder` | `include/hts/task.hpp`, `include/hts/task_graph.hpp`, `include/hts/task_builder.hpp`, `src/core/task.cpp`, `src/core/task_graph.cpp` |
| Scheduling | `Scheduler`, `SchedulingPolicy`, `DependencyManager` | `include/hts/scheduler.hpp`, `include/hts/scheduling_policy.hpp`, `src/core/scheduler.cpp`, `src/core/scheduling_policy.cpp`, `src/core/dependency_manager.cpp` |
| Execution | `ExecutionEngine`, `MemoryPool`, `StreamManager` | `include/hts/execution_engine.hpp`, `include/hts/memory_pool.hpp`, `include/hts/stream_manager.hpp`, `src/core/execution_engine.cpp`, `src/cuda/` |
| Utilities | `TaskContext`, `Profiler`, `GraphSerializer`, `RetryPolicy`, `EventSystem`, `ResourceLimiter` | Public headers under `include/hts/` plus tests in `tests/unit/` |

## Execution flow

The current `Scheduler` implementation in `src/core/scheduler.cpp` follows this sequence:

1. Validate the in-memory `TaskGraph`.
2. Create a `DependencyManager` for the current run.
3. Collect ready tasks and ask the active `SchedulingPolicy` to order them.
4. Choose CPU or GPU execution for each task with `select_device(...)`.
5. Submit work to the `ExecutionEngine`.
6. Mark completed tasks, unblock successors, and continue until the graph drains.
7. Record `ExecutionStats` and timeline data for the finished run.

## CPU-only versus CUDA paths

- **CPU-only mode** builds the shared runtime plus `src/cuda/cuda_stubs.cpp`.
- **CUDA mode** enables `src/cuda/memory_pool.cu` and `src/cuda/stream_manager.cu`.
- The public headers keep CUDA types forward-declared in `include/hts/types.hpp` so most code does not
  need to include the CUDA runtime directly.

## Why the docs are intentionally smaller now

Previous site pages mixed real architecture notes with theoretical papers, placeholder performance claims,
and APIs that the code no longer exposed. The maintained boundary is now simple:

- This page describes file-level structure that exists in the repository today.
- The [API Overview](/en/api/) maps directly to public headers.
- The [Examples](/en/examples/) page points only to example programs that actually compile in this tree.
