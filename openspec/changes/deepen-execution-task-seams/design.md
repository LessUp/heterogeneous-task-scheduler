## Context

HTS closeout work already extracted a shared scheduler core. The next highest-leverage seam is
`ExecutionEngine`, where orchestration logic remains duplicated across `src/cuda/execution_engine.cu`
and `src/cuda/cuda_stubs.cpp`. In parallel, `Task` currently stores both declarative properties and
mutable execution state, which couples graph-definition surfaces to runtime mutation details.

This batch prioritizes aggressive internal simplification over backward compatibility, while keeping
CPU-only validation as the primary executable safety net on this machine.

## Goals / Non-Goals

**Goals:**
- Compile one shared execution-engine orchestration implementation for CUDA-enabled and CPU-only
  builds.
- Isolate backend-specific behavior to thin CUDA/stub adapter hooks.
- Introduce explicit task-definition vs runtime-state boundaries and migrate core runtime call sites.
- Preserve default CPU-only closeout validation coverage for these seams.

**Non-Goals:**
- Prove real CUDA runtime behavior on this host.
- Redesign scheduler policy behavior or telemetry vocabulary beyond what seam extraction requires.
- Add new product features unrelated to runtime-core deepening.

## Decisions

### 1. Shared execution-engine core in `src/core`

Move common `ExecutionEngine` lifecycle logic (thread startup/shutdown, queueing, task state
transitions, context lookup, wait/load APIs) to a shared core translation unit compiled in all build
modes.

**Alternatives considered**
- Keep two implementations and patch both: rejected due to repeated drift and bug recurrence risk.
- Extract only helpers while retaining duplicate top-level methods: rejected because top-level methods
  carry most behavior coupling.

### 2. Backend-specific behavior remains adapter-level

Keep CUDA runtime specifics (stream synchronization semantics, error translation differences) in
backend-adjacent functions while shared orchestration calls into these hooks.

**Alternatives considered**
- Introduce a full plugin/runtime backend framework now: rejected as unnecessary complexity for
  closeout.
- Inline all CUDA/stub differences with preprocessor branches in one file: rejected for readability
  and testability concerns.

### 3. Split task definition from runtime state

Establish explicit types for immutable task definition (identity, priority, preferred placement, work
functions) versus mutable runtime state (execution state, actual device, timing, cancellation, runtime
context ownership). Core runtime modules consume definition and mutate runtime state through deliberate
interfaces instead of treating one `Task` object as both model and runtime record.

**Alternatives considered**
- Keep current `Task` shape and document conventions: rejected because conventions do not enforce the
  seam.
- Full ECS-style rewrite: rejected as excessive scope for closeout.

## Risks / Trade-offs

- **Integration churn across scheduler/execution/task graph call sites** -> Mitigation: stage with
  focused regression tests first, then migrate incrementally behind compatibility helpers where needed.
- **Breaking API expectations for downstream callers** -> Mitigation: mark breaking scope in change
  artifacts and update tests/docs in the same batch.
- **Backend drift despite shared core extraction** -> Mitigation: keep only minimal adapter functions
  outside shared core and cover behavior with CPU-only regression harness.

## Migration Plan

1. Add tests that fail under current duplication/coupling boundaries.
2. Extract shared execution-engine core and remove duplicated implementations.
3. Introduce task-definition/runtime-state split and migrate scheduler/runtime integration.
4. Run closeout baseline (`build --cpu-only`, tests, format, analyze, docs build).

Rollback path: revert this change as a unit to restore previous runtime model.

## Open Questions

- None for this batch; CUDA hardware execution verification is intentionally deferred.
