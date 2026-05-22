## Why

HTS still carries shallow runtime seams that hide correctness bugs behind permissive interfaces. The
current graph and runtime modules allow invalid states such as leaked internal graph storage, zero
worker configuration, and duplicate or foreign CUDA stream release, which makes closeout
stabilization fragile and expensive to maintain.

## What Changes

- **BREAKING** remove `TaskGraph::tasks()` as public storage exposure and replace it with explicit
  traversal/query interfaces that preserve DAG invariants.
- **BREAKING** reject invalid scheduler runtime configuration such as zero CPU threads or zero GPU
  streams instead of silently computing undefined load values.
- Harden runtime resource ownership so `StreamManager` rejects duplicate or foreign stream release
  instead of corrupting pool state.
- Ensure scheduler execution state is always cleaned up when task execution or validation throws.
- Add regression tests that lock down the stricter graph/runtime contract in CPU-only validation.

## Capabilities

### New Capabilities
- `runtime-invariants`: Defines the stricter graph traversal and runtime validation contract for HTS
  closeout.

### Modified Capabilities
- `closeout-readiness`: Closeout now requires identified graph/runtime defects to be fixed with
  regression coverage rather than left as implicit implementation debt.

## Impact

- Affected code: `include/hts/task_graph.hpp`, `include/hts/stream_manager.hpp`,
  `include/hts/scheduler.hpp`, `include/hts/types.hpp`, `src/core/task_graph.cpp`,
  `src/core/dependency_manager.cpp`, `src/cuda/scheduler.cu`, `src/cuda/execution_engine.cu`,
  `src/cuda/stream_manager.cu`, serializer and unit tests.
- Public API impact: callers must migrate off `TaskGraph::tasks()` and stop passing invalid runtime
  configuration or invalid stream handles.
- Validation impact: CPU-only build/test/analyze/format/docs remain the baseline, with new
  regression coverage added to unit tests.
