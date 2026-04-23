## Why

The repository has drifted away from a reliable OpenSpec-driven workflow and now mixes placeholder
specs, duplicated automation, redundant documentation, and inconsistent contributor guidance. This
cleanup is needed now so HTS can reach a stable, credible closeout state instead of accumulating
more architectural and process drift.

## What Changes

- Rebuild the repository governance baseline around a strict OpenSpec source of truth, including
  the required directory/config contract and project-specific agent instructions.
- Replace stale or generic documentation with concise, HTS-specific guidance for contributors,
  AI-assisted development, quality gates, and finish-stage maintenance.
- Consolidate GitHub Actions, hooks, editor/tooling setup, and version pinning into a lean
  engineering surface that matches the actual C++/CUDA project.
- Redesign GitHub Pages and GitHub repository metadata so public-facing surfaces explain the
  scheduler clearly instead of mirroring README content or shipping conflicting site pipelines.
- Audit the implementation against the rebuilt specs, establish a trustworthy validation baseline,
  and fix the concrete issues that block a closeout-ready release.

## Capabilities

### New Capabilities

- `project-governance`: Defines the canonical OpenSpec layout, governance documents, agent guidance,
  review checkpoints, and closeout-oriented development workflow for the repository.
- `engineering-surface`: Defines the required CI/workflow layout, hook strategy, editor/LSP and
  AI-tooling configuration, version pinning, and low-noise automation boundaries for HTS.
- `project-showcase`: Defines the required GitHub Pages structure, repository About metadata, and
  public project positioning for onboarding and discovery.
- `closeout-readiness`: Defines the stabilization baseline, bug-fix expectations, validation scope,
  and completion criteria for preparing the project for low-maintenance archival readiness.

### Modified Capabilities

- None.

## Impact

- Affected areas: `openspec/`, `specs` symlink target, `AGENTS.md`, new `CLAUDE.md`,
  `.github/workflows/`, `.github/` templates, `README*`, `CONTRIBUTING.md`, `CHANGELOG.md`,
  `changelog/`, `website/`, editor/tooling config files, and targeted source/tests touched by the
  stabilization pass.
- External systems: GitHub repository About metadata, topics, homepage URL, and Pages deployment.
- Risk profile: high repository churn but low intended public API change; existing user worktree
  edits must be preserved during implementation.
