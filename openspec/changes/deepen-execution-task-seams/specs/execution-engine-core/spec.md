## ADDED Requirements

### Requirement: Execution engine orchestration SHALL have one shared implementation
HTS SHALL compile one shared `ExecutionEngine` orchestration implementation for both CPU-only and
CUDA-enabled builds so that task submission, worker lifecycle, task state transitions, wait
coordination, and load reporting do not diverge by build mode.

#### Scenario: Build mode selection compiles execution engine
- **WHEN** HTS is built in CPU-only mode or with CUDA enabled
- **THEN** both build modes compile the same execution-engine orchestration logic rather than distinct
  duplicated implementations

### Requirement: Backend-specific execution details SHALL be adapter-level only
HTS SHALL keep backend-specific behavior at narrow adapter seams while shared execution orchestration
remains backend-agnostic.

#### Scenario: Runtime code review inspects execution engine wiring
- **WHEN** a maintainer traces execution-engine method definitions used by CPU-only and CUDA builds
- **THEN** shared lifecycle/orchestration behavior is centralized and only backend-specific calls
  differ between adapters

### Requirement: Execution lifecycle regressions SHALL be testable in CPU-only validation
HTS SHALL provide CPU-only unit coverage that exercises submission, completion/failure propagation, and
wait semantics for the shared execution-engine core.

#### Scenario: Maintainer runs default test suite on CPU-only host
- **WHEN** the maintainer executes the repository test baseline without CUDA hardware
- **THEN** execution-engine regression tests validate shared lifecycle behavior and fail on drift
