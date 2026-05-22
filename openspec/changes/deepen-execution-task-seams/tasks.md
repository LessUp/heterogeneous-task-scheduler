## 1. Regression harness

- [x] 1.1 Add execution-engine regression tests that lock down shared lifecycle behavior (submit, wait, failure propagation, load reporting) on CPU-only validation.
- [x] 1.2 Add tests that enforce task-definition/runtime-state separation semantics during execution.

## 2. Shared execution-engine core extraction

- [x] 2.1 Move duplicated `ExecutionEngine` orchestration into a shared core source compiled by both CPU-only and CUDA builds.
- [x] 2.2 Remove duplicated execution-engine implementations from CUDA and CPU-only stub files, leaving only backend-specific adapter differences.
- [x] 2.3 Update build wiring and includes to compile/link the shared execution-engine core in both build modes.

## 3. Task/runtime seam deepening

- [x] 3.1 Introduce explicit task-definition and runtime-state structures (or equivalent seams) and migrate core runtime modules to use them.
- [x] 3.2 Refactor scheduler/execution integration to mutate runtime state through explicit interfaces instead of mutating definition objects.
- [x] 3.3 Update dependent graph/runtime utilities and tests for the new task/runtime seam, including documented breaking API surface changes.

## 4. Validation

- [x] 4.1 Run CPU-only build and full test baseline after extraction and task/runtime split.
- [x] 4.2 Run format, analysis, and docs build checks and resolve regressions.
