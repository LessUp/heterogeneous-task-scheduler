## 1. Regression harness

- [x] 1.1 Add scheduler/policy regression tests that must survive shared-core extraction.
- [x] 1.2 Add focused CPU-only tests that prove policy adapters still construct and execute through the shared scheduler core.

## 2. Shared scheduler core

- [x] 2.1 Move duplicated `Scheduler` implementation into a shared source file compiled by both build modes.
- [x] 2.2 Remove duplicated scheduler logic from CUDA and CPU-only adapter files.

## 3. Policy seam deepening

- [x] 3.1 Move concrete scheduling policy adapter implementations out of the public header into source files.
- [x] 3.2 Narrow scheduler/policy header dependencies and fix any resulting include fallout.

## 4. Validation

- [x] 4.1 Run CPU-only build and full test baseline after the scheduler-core extraction.
- [x] 4.2 Run format, analyze, and docs build checks and resolve any regressions.
