## 1. Regression harness

- [x] 1.1 Add failing tests for invalid scheduler configuration and scheduler exception cleanup.
- [x] 1.2 Add failing tests for duplicate/foreign stream release and graph traversal consumers.

## 2. Graph and runtime hardening

- [x] 2.1 Replace `TaskGraph::tasks()` with explicit traversal/query APIs and update internal users.
- [x] 2.2 Enforce scheduler configuration invariants and guaranteed execution-state cleanup.
- [x] 2.3 Enforce stream lease ownership invariants in `StreamManager`.

## 3. Validation

- [x] 3.1 Run CPU-only build and full test baseline after the refactor.
- [x] 3.2 Run format, analyze, and docs build checks and resolve any regressions.
