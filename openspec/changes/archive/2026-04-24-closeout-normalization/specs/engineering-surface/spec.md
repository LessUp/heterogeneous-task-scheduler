## ADDED Requirements

### Requirement: Automation surface SHALL be singular and low-noise

The repository SHALL provide one canonical validation workflow for code and documentation quality and one canonical GitHub Pages deployment workflow, and competing or duplicated workflows that serve the same outcome SHALL not remain active.

#### Scenario: GitHub Actions is inspected after normalization

- **WHEN** a maintainer reviews `.github/workflows/`
- **THEN** each workflow has a distinct purpose and the repository does not contain duplicate Pages
  or overlapping validation pipelines

### Requirement: Local scripts SHALL match the real build system

Tracked developer scripts SHALL use supported CMake options, documented tool versions, and repository-supported execution paths, and CPU-only validation SHALL remain a first-class path for local and CI environments that lack CUDA hardware.

#### Scenario: Contributor uses the documented build and test scripts

- **WHEN** a contributor follows the repository's scripted local workflow on a CPU-only environment
- **THEN** the scripts execute supported configuration paths and do not rely on undefined CMake
  options or obsolete build assumptions

### Requirement: Repository SHALL define a minimal checked-in tooling contract

The repository SHALL provide checked-in guidance for Copilot, Claude, and editor/LSP usage that is specific to HTS, and the default tooling contract SHALL favor lightweight repo-local configuration over heavyweight integrations that increase maintenance or context cost without clear value.

#### Scenario: AI-assisted contributor configures a workstation

- **WHEN** the contributor consults the repository for AI/editor setup
- **THEN** they can find the sanctioned instructions, lightweight defaults, and tool boundaries
  without needing undocumented global setup

### Requirement: Hooks SHALL provide fast non-destructive quality gates

If versioned git hooks are provided, they SHALL run fast checks that reinforce repository standards without rewriting unrelated files or blocking work on unsupported environments.

#### Scenario: Contributor installs project hooks

- **WHEN** a contributor enables the tracked hook setup
- **THEN** the hooks run targeted validations that match repository standards and avoid destructive
  mutation of unrelated worktree content
