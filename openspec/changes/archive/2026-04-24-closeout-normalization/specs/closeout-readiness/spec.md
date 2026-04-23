## ADDED Requirements

### Requirement: Closeout SHALL define a trustworthy validation baseline

Before the repository is considered closeout-ready, it SHALL define and execute a validation baseline that covers CPU-only buildability, automated tests, formatting or static checks, and the maintained documentation or Pages build path.

#### Scenario: Maintainer evaluates closeout readiness

- **WHEN** the maintainer checks whether HTS is ready for low-maintenance archival posture
- **THEN** a documented validation baseline exists and can be executed against the normalized repo

### Requirement: Spec drift and concrete defects SHALL be resolved or tracked before archive

Defects or behavior mismatches uncovered during the closeout sweep SHALL either be fixed or captured explicitly in the active OpenSpec tasks before the change is archived.

#### Scenario: Bug sweep finds a mismatch

- **WHEN** validation reveals code, docs, or automation that no longer matches the intended
  repository behavior
- **THEN** the mismatch is fixed or recorded as an explicit closeout task rather than silently
  ignored

### Requirement: Closeout tasks SHALL support long-running autopilot sessions

The active closeout change SHALL organize tasks into coherent batches that can be executed in long-running autopilot sessions without depending on `/fleet` by default.

#### Scenario: Maintainer hands the change to automation

- **WHEN** an AI agent begins implementing the closeout tasks
- **THEN** the tasks are ordered into mergeable batches with clear dependencies and review gates

### Requirement: Closeout implementation SHALL preserve unrelated user work

Normalization and stabilization work SHALL avoid destructive cleanup of unrelated pre-existing changes in the repository worktree.

#### Scenario: Active worktree contains unrelated edits

- **WHEN** the closeout change touches files in a dirty repository
- **THEN** implementation preserves unrelated edits and limits modifications to the required scope
