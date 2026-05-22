## Why

HTS still carries its shallowest runtime module in `Scheduler`: orchestration, policy dispatch,
profiling, events, stats, and timeline logic are fused together and duplicated across CUDA and
CPU-only implementations. That duplication weakens locality, amplifies bug-fix cost, and blocks the
larger closeout refactors around runtime depth.

## What Changes

- Extract the scheduler execution loop into a shared runtime-core implementation used by both CUDA
  and CPU-only builds.
- Move concrete scheduling policy implementations out of the public header so the policy seam is an
  interface plus adapters, not a header-only implementation bundle.
- Narrow the public scheduler header to depend less directly on runtime implementation details while
  preserving the current public API shape.
- Add regression tests around scheduler-core behavior so CPU-only validation can lock down the shared
  implementation without requiring CUDA hardware.

## Capabilities

### New Capabilities
- `runtime-core-seams`: Defines the shared scheduler-runtime core and policy adapter contract used by
  both CPU-only and CUDA builds.

### Modified Capabilities
- `engineering-surface`: CPU-only validation remains a first-class path and now covers the shared
  scheduler core that both build modes depend on.

## Impact

- Affected code: `include/hts/scheduler.hpp`, `include/hts/scheduling_policy.hpp`,
  `src/cuda/scheduler.cu`, `src/cuda/cuda_stubs.cpp`, new shared runtime-core source files, CMake,
  and scheduler/policy tests.
- Build impact: both CPU-only and CUDA builds will link the same scheduler-core implementation.
- Maintenance impact: bug fixes in scheduler orchestration move from duplicated edits to one module.
