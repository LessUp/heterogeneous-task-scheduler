## Why

The scheduler deepening batch removed one high-risk duplication seam, but `ExecutionEngine` is still
duplicated between CUDA and CPU-only stub paths and `Task` still mixes immutable definition data with
mutable runtime state. These seams are now the largest remaining source of bug-prone drift and low
locality in the runtime core.

## What Changes

- Extract a shared `ExecutionEngine` core implementation that both CUDA and CPU-only builds compile.
- Keep CUDA-vs-stub differences behind thin adapter surfaces instead of duplicating orchestration.
- **BREAKING**: split task definition concerns from runtime state/context concerns and migrate
  scheduler/runtime call sites to the new seam.
- Add regression tests that lock down execution lifecycle and task/runtime separation behavior on
  CPU-only machines.

## Capabilities

### New Capabilities
- `execution-engine-core`: Defines one shared execution-engine orchestration path across build modes.
- `task-runtime-separation`: Defines a hard seam between immutable task definition and mutable runtime
  state/context.

### Modified Capabilities
- `engineering-surface`: CPU-only validation remains first-class and SHALL cover shared
  execution-engine behavior plus the new task/runtime seam.

## Impact

- Affected code: `include/hts/execution_engine.hpp`, `src/cuda/execution_engine.cu`,
  `src/cuda/cuda_stubs.cpp`, new shared core source files, task model headers/sources, scheduler and
  task graph integration paths, tests, and CMake source wiring.
- API impact: task model and runtime integration surfaces can change in non-backward-compatible ways
  in service of long-term maintainability.
- Maintenance impact: runtime bug fixes move from duplicated edits to shared core updates with clearer
  ownership boundaries.
