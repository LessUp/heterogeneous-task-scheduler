## Context

Exploration showed one dominant shallow module: `Scheduler`. Its interface is wide enough that
callers already know too much, and its implementation is copied almost verbatim in
`src/cuda/scheduler.cu` and `src/cuda/cuda_stubs.cpp`. Concrete `SchedulingPolicy` adapters also
live in the public header, which keeps the policy seam shallow and forces recompilation across the
tree.

This batch is intentionally narrower than the full runtime deepening roadmap. It focuses on the
shared scheduler core first because that yields the highest leverage, reduces duplication fastest,
and creates a better seam for later extraction of execution-engine core and task/runtime-state
splits.

## Goals / Non-Goals

**Goals:**
- Create one shared scheduler-core implementation used by both build modes.
- Separate the policy interface from concrete policy adapter implementations.
- Keep the current public `Scheduler` API stable while reducing implementation coupling in the
  header.
- Preserve CPU-only validation as the default regression seam.

**Non-Goals:**
- Fully refactor `ExecutionEngine`, `StreamManager`, or `MemoryPool` into shared core modules in this
  batch.
- Complete the `Task` vs runtime-state split in this batch.
- Redesign profiling or event vocabulary beyond what the shared scheduler core needs.

## Decisions

### 1. Extract shared scheduler implementation into common source

`Scheduler` method definitions that do not directly require CUDA runtime calls will move into a new
shared source file compiled in both build modes. `src/cuda/scheduler.cu` and `src/cuda/cuda_stubs.cpp`
become thin build-mode adapters or disappear for scheduler logic entirely.

**Alternatives considered**
- Keep duplicate implementations and “sync carefully”: rejected because it preserves low locality.
- Extract only helper functions and leave the main loop duplicated: rejected because the main loop is
  the shallow part that carries most bug risk.

### 2. Keep `SchedulingPolicy` as interface, move adapters to source

The public header will keep the policy interface and factory/access points needed by callers, but
concrete policy adapter logic moves to `.cpp` implementations. This lowers rebuild cost and turns the
policy seam into a real interface/adapter split.

**Alternatives considered**
- Leave adapters inline in the header: rejected because it keeps the seam shallow.
- Introduce plugin loading now: rejected because it adds complexity without closeout payoff.

### 3. Stage the runtime deepening roadmap

This change only lands scheduler-core extraction plus policy deepening. The next likely batches are:
`ExecutionEngine` core extraction, then task-definition/runtime-state separation.

**Alternatives considered**
- Refactor scheduler, engine, and task model in one change: rejected because regression risk is too
  high for one batch.

## Risks / Trade-offs

- **Build-mode skew during extraction** -> Mitigation: both build modes compile the same scheduler
  source before deleting duplicates.
- **Policy move can break tests or construction sites** -> Mitigation: keep public class names and
  constructor behavior stable while moving method definitions out of line.
- **Header narrowing can expose hidden include dependencies** -> Mitigation: rebuild full CPU-only
  baseline and fix includes explicitly.

## Migration Plan

1. Add regression coverage for scheduler-core behavior and policy usage that must survive extraction.
2. Introduce shared scheduler-core source and switch both build modes to it.
3. Move concrete policy adapter implementations out of the header and fix include fallout.
4. Run CPU-only build/test/format/analyze/docs baseline.

Rollback is low-risk: revert the shared source introduction and restore duplicated scheduler logic if
integration issues appear.

## Open Questions

- None for this batch. The larger execution-engine and task/runtime-state refactors remain explicitly
  out of scope.
