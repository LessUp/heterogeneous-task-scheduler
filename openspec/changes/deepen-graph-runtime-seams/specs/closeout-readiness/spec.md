## MODIFIED Requirements

### Requirement: Spec drift and concrete defects SHALL be resolved or tracked before archive
Defects or behavior mismatches uncovered during the closeout sweep SHALL either be fixed with
regression coverage or captured explicitly in the active OpenSpec tasks before the change is
archived.

#### Scenario: Bug sweep finds a mismatch
- **WHEN** validation reveals code, docs, or automation that no longer matches the intended
  repository behavior
- **THEN** the mismatch is fixed with regression coverage or recorded as an explicit closeout task
  rather than silently ignored
