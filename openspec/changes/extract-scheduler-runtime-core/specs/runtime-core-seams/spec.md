## ADDED Requirements

### Requirement: Scheduler orchestration SHALL have one shared implementation
HTS SHALL compile one shared scheduler-core implementation for both CPU-only and CUDA builds so that
graph validation, dependency progression, task dispatch, failure propagation, profiling hooks, and
timeline/statistics updates do not drift across build modes.

#### Scenario: CPU-only and CUDA builds use scheduler core
- **WHEN** HTS is built in CPU-only mode or with CUDA enabled
- **THEN** both build modes compile the same scheduler-core orchestration logic rather than separate
  duplicated implementations

### Requirement: Scheduling policy SHALL be expressed as interface plus adapters
HTS SHALL keep `SchedulingPolicy` as the public seam and move concrete policy adapter implementations
out of the public header so callers depend on the interface contract rather than inline
implementation details.

#### Scenario: Caller includes scheduling policy header
- **WHEN** a caller includes the public scheduling policy header
- **THEN** they receive the policy interface contract without needing concrete adapter implementation
  bodies inline in the header

### Requirement: Scheduler-core behavior SHALL stay testable on CPU-only validation
HTS SHALL provide CPU-only regression coverage for scheduler-core behavior so the shared
implementation can be validated without requiring CUDA hardware.

#### Scenario: Maintainer validates scheduler refactor on CPU-only machine
- **WHEN** the maintainer runs the default CPU-only validation baseline
- **THEN** scheduler-core regression tests execute and verify the shared orchestration behavior
