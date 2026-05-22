## Context

HTS closeout favors simplification, correctness, and maintenance reduction. Two shallow modules are
still carrying too much risk:

1. `TaskGraph` exposes its backing `unordered_map` through `tasks()`, so callers depend on storage
   layout instead of a graph interface.
2. Runtime execution tolerates invalid state: zero worker counts feed divide-by-zero load math,
   `StreamManager` accepts duplicate or foreign releases, and `Scheduler::executing_` is only reset
   on the happy path.

These problems are tightly coupled. The graph leak encourages reach-through code in
`DependencyManager` and `GraphSerializer`, while weak runtime invariants let invalid states survive
until much later in execution.

## Goals / Non-Goals

**Goals:**
- Deepen the graph module by replacing storage exposure with explicit traversal/query seams.
- Centralize runtime invariant validation at construction or release points instead of downstream
  arithmetic or silent pool corruption.
- Preserve CPU-only closeout validation as the primary regression seam.
- Add regression tests before fixes so the stricter contract is locked in.

**Non-Goals:**
- Re-architect the entire scheduler into separate orchestration modules in this change.
- Redesign the task data model or policy model.
- Change GitHub metadata, docs IA, or public product positioning.

## Decisions

### 1. Remove raw graph storage exposure

`TaskGraph::tasks()` will be removed and replaced with explicit read APIs oriented around caller
intent. The initial seam is:

- `std::vector<TaskId> task_ids() const`
- existing `get_task`, `get_successors`, `get_predecessors`, `size`, `empty`

This is intentionally smaller than exposing iterators or callbacks. `DependencyManager` and
`GraphSerializer` only need task identity traversal plus existing queries, so a narrow interface
improves locality without introducing template-heavy traversal helpers.

**Alternatives considered**
- Keep `tasks()` and document “read-only” usage: rejected because it preserves shallow coupling.
- Add a callback-based traversal API only: rejected because `task_ids()` is simpler to test and
  easier for callers to adopt.

### 2. Fail fast on invalid runtime configuration

`Scheduler` construction will validate `SchedulerConfig` and throw `std::invalid_argument` if
`cpu_thread_count == 0` or `gpu_stream_count == 0`. This is a deliberate breaking change: invalid
configuration should fail at the seam where it is introduced, not later via NaN/inf utilization.

`ExecutionEngine` will assume validated input and keep its load math simple.

**Alternatives considered**
- Clamp zero values to one: rejected because it silently changes caller intent.
- Return zero load when count is zero: rejected because it hides a configuration bug.

### 3. Make stream ownership explicit

`StreamManager` will track which streams are owned and currently leased. `release_stream()` will
throw on foreign or duplicate release. This creates a real ownership seam and prevents queue
corruption that could otherwise hand out the same stream multiple times.

**Alternatives considered**
- Ignore invalid releases: rejected because it preserves silent corruption.
- Debug-only assertions: rejected because release misuse is a correctness defect, not debug-only
  hygiene.

### 4. Use scope-based execution cleanup

`Scheduler::execute_internal()` will use a small scope guard or equivalent local RAII helper so
`executing_` always resets, including validation failure or task exception propagation. This keeps
teardown and rerun behavior local to the scheduler implementation.

**Alternatives considered**
- Scatter `executing_ = false` across catch blocks: rejected because it is brittle and easy to miss.

## Risks / Trade-offs

- **Breaking API adoption cost** -> Mitigation: keep replacement graph queries minimal and update all
  in-tree call sites/tests in one change.
- **Stricter runtime validation may break undocumented callers** -> Mitigation: throw descriptive
  exceptions and cover them with unit tests.
- **CPU-only tests cannot exercise real CUDA concurrency** -> Mitigation: validate ownership logic at
  the stub-compatible seam and preserve existing GPU tests for CUDA-enabled environments.

## Migration Plan

1. Add regression tests for graph traversal usage, invalid scheduler config, duplicate/foreign stream
   release, and scheduler exception cleanup.
2. Implement graph traversal API and update internal users.
3. Add runtime validation and stream lease tracking.
4. Run CPU-only build/test/format/analyze/docs baseline.

Rollback is straightforward because the change is isolated to runtime/graph modules; revert the
change if downstream API breakage is unacceptable.

## Open Questions

- None. The chosen scope is intentionally narrower than the larger scheduler/module split surfaced
  during exploration.
