## ADDED Requirements

### Requirement: Repository SHALL expose a canonical OpenSpec contract

The repository SHALL store OpenSpec configuration in `openspec/config.yaml`, active changes in `openspec/changes/`, archived changes in `openspec/changes/archive/`, and durable capability specs in `openspec/specs/<capability>/spec.md`, and the compatibility `specs` path SHALL resolve to `openspec/specs`.

#### Scenario: Maintainer audits the repository layout

- **WHEN** a maintainer inspects the repository after the governance reset
- **THEN** the canonical OpenSpec paths exist and are the only source of live specification truth

### Requirement: Governance documents SHALL be HTS-specific and actionable

The repository SHALL provide project-specific governance guidance in `AGENTS.md`, `CLAUDE.md`, and contributor-facing workflow documents, and those documents SHALL describe the HTS closeout workflow, OpenSpec usage, review gates, and quality expectations without generic filler text.

#### Scenario: Contributor reads the governance files

- **WHEN** a contributor or AI agent reads the governing documents before making a material change
- **THEN** they can identify the required OpenSpec workflow, review checkpoints, and closeout
  expectations without consulting redundant secondary documents

### Requirement: Material repository changes SHALL be traced to an active OpenSpec change

The repository MUST trace any material change to repository structure, engineering workflow, public documentation, or product positioning to an active OpenSpec change until that change is archived.

#### Scenario: Maintainer proposes a broad repository adjustment

- **WHEN** the adjustment changes repository behavior or project guidance in a non-trivial way
- **THEN** the work is recorded in an active OpenSpec change before implementation proceeds

### Requirement: Persistent documentation SHALL be curated for signal

Persistent top-level documentation SHALL either govern project delivery or explain HTS to users, and placeholder, duplicated, or low-value documentation SHALL be merged, archived, or removed.

#### Scenario: Documentation inventory is reviewed during closeout

- **WHEN** a file does not add project-specific value beyond another maintained document
- **THEN** it is consolidated, archived, or deleted instead of remaining as redundant surface area
