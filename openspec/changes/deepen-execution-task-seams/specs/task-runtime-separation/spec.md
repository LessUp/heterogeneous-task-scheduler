## ADDED Requirements

### Requirement: Task definition SHALL be separate from mutable runtime state
HTS SHALL represent task definition metadata and executable functions separately from mutable runtime
state so graph construction and runtime execution concerns are not stored in one coupled object.

#### Scenario: Task is created before execution
- **WHEN** a task is defined and inserted into a task graph
- **THEN** immutable definition fields are available without requiring runtime-state mutation APIs

### Requirement: Runtime state mutations SHALL flow through explicit runtime interfaces
HTS SHALL route task execution state transitions, actual-device assignment, timing updates, and
cancellation state through runtime-state interfaces rather than mutating definition objects directly.

#### Scenario: Scheduler dispatches a task
- **WHEN** scheduler and execution engine coordinate task dispatch and completion
- **THEN** runtime mutation occurs through runtime-state surfaces that are distinct from definition
  storage

### Requirement: Task/runtime seam SHALL preserve CPU-only testability
HTS SHALL include CPU-only tests that verify task definitions remain stable while runtime state changes
during execution.

#### Scenario: Unit test observes task during execution lifecycle
- **WHEN** test code inspects definition and runtime fields before and after execution
- **THEN** definition metadata remains unchanged while runtime-state transitions reflect execution
