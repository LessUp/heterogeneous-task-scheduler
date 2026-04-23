## Context

HTS is a C++/CUDA library that already has real source code, tests, build scripts, GitHub Actions,
and a VitePress website, but its governance layer has drifted. The current repository mixes a
partially migrated OpenSpec layout, placeholder durable specs, duplicated Pages workflows, verbose
and overlapping contributor docs, and missing AI/editor guidance files. The user wants an
aggressive normalization pass that makes the project easier to finish, easier to reason about, and
safer to hand to later long-running autopilot sessions without relying on `/fleet`.

Key constraints:

- The worktree already contains unrelated user edits and must be handled non-destructively.
- Public library API churn should be minimized; this change focuses on governance, engineering
  surfaces, project presentation, and targeted stabilization.
- OpenSpec must become the authoritative driver of future work instead of an afterthought.
- GitHub repository metadata and Pages configuration may be changed directly through `gh`.
- Any new tooling or automation must earn its place; heavy integrations that burn context or create
  maintenance overhead are out of scope unless clearly justified.

## Goals / Non-Goals

**Goals:**

- Restore a canonical OpenSpec repository contract, including config, durable capability specs, and
  a single closeout-oriented change that governs the remaining work.
- Reduce documentation and workflow sprawl by removing or rewriting low-value material.
- Define a minimal but complete contributor/agent workflow across AGENTS, CLAUDE, Copilot
  instructions, hooks, review gates, and editor/LSP setup.
- Collapse Pages and CI into coherent, low-noise pipelines.
- Reposition the public-facing website and GitHub metadata around clear product explanation and
  credible onboarding.
- Establish a trustworthy baseline for closeout validation and fix the issues that block it.

**Non-Goals:**

- Adding new end-user scheduler features unrelated to closeout readiness.
- Introducing a broad new platform of third-party developer services or heavyweight MCPs.
- Preserving every historical documentation artifact if it no longer provides project-specific value.
- Maintaining multiple parallel documentation sites or duplicate contributor workflows.

## Decisions

### Decision: Drive all remaining cleanup through one active closeout change

The repository will use a single OpenSpec change, `closeout-normalization`, as the coordinating
artifact for governance reset, documentation consolidation, workflow rationalization, website
repositioning, and stabilization.

**Rationale:** The repository is already suffering from drift across many surfaces. One active
change keeps scope visible, prevents parallel process divergence, and gives later automation a
single source of tasks.

**Alternatives considered:**

- Split the work into many small changes: rejected because it would recreate process sprawl and
  increase coordination overhead.
- Make direct repo edits first and document later: rejected because it would repeat the current
  spec drift problem.

### Decision: Rebuild durable specs around capabilities, not document categories

The live change will define durable capabilities for governance, engineering surface, project
showcase, and closeout readiness. Existing placeholder category READMEs in `openspec/specs/` are
not treated as sufficient specification.

**Rationale:** The current `openspec/specs/` tree is structurally present but semantically hollow.
Capability specs create testable requirements that can drive implementation and archival.

**Alternatives considered:**

- Keep category READMEs as the primary spec layer: rejected because they do not encode normative,
  testable behavior.
- Restore the entire previous spec corpus unchanged: rejected because the repo needs selective,
  current-state requirements, not a blind resurrection of stale material.

### Decision: Standardize on one website stack and one deployment path

VitePress will be the canonical GitHub Pages implementation, and the legacy Jekyll Pages workflow
will be removed.

**Rationale:** The repository already has a more complete VitePress site. Keeping both Jekyll and
VitePress doubles maintenance cost, creates inconsistent outputs, and wastes CI runs.

**Alternatives considered:**

- Keep both pipelines for backward compatibility: rejected because the user explicitly wants less
  noise and fewer meaningless workflows.
- Revert fully to Jekyll: rejected because it would discard the more capable existing site without a
  closeout benefit.

### Decision: Prefer a minimal repo-local tooling contract

The repository will define a small, explicit tooling surface: `AGENTS.md`, `CLAUDE.md`,
`.github/copilot-instructions.md`, lightweight editor/LSP config, a versioned hook setup, and
documented use of `/review`. New MCP or plugin integrations will only be added if they clearly
improve HTS maintenance without high context cost.

**Rationale:** The user wants best-practice AI-assisted development, but also wants to avoid bloated
or generic tooling. A small repo-local contract is easier to audit and maintain.

**Alternatives considered:**

- Configure many tools symmetrically (Copilot, Claude, OpenCode, MCP, plugins) regardless of value:
  rejected because it adds complexity without guaranteeing better outcomes.
- Leave tooling implicit: rejected because the current repo already shows drift from missing guidance.

### Decision: Sequence implementation from governance reset to stabilization

Implementation will first repair the governance and repository contract, then consolidate docs and
engineering surfaces, then reposition Pages and GitHub metadata, and finally perform the targeted
bug sweep against the new baseline.

**Rationale:** This order avoids stabilizing code against a moving governance target and ensures the
public-facing surfaces reflect the normalized repository rather than stale content.

**Alternatives considered:**

- Start with bug fixing: rejected because the project lacks a trustworthy spec/process baseline.
- Start with website polish only: rejected because public presentation should follow internal
  normalization, not mask it.

## Risks / Trade-offs

- **[High repo churn]** Removing or rewriting broad documentation and workflow surfaces could
  accidentally drop useful material -> Mitigation: inspect before deleting, preserve only material
  that remains project-specific and actionable.
- **[Worktree interference]** Existing user edits may overlap with touched files -> Mitigation:
  review diffs carefully, avoid destructive commands, and isolate changes to the required scope.
- **[Over-specification]** Governance specs could become another pile of generic prose ->
  Mitigation: keep requirements directly tied to HTS closeout and observable repository behavior.
- **[Validation blind spots]** Baseline build/test issues may surface late if automation remains
  inconsistent -> Mitigation: define and execute a closeout validation matrix before archive.
- **[Remote metadata drift]** GitHub About/Pages settings may diverge from repository files ->
  Mitigation: make remote changes as part of the same workstream and document the intended steady
  state in specs and tasks.

## Migration Plan

1. Add the closeout change artifacts and durable capability specs.
2. Normalize the OpenSpec directory/config contract and governing documents.
3. Consolidate contributor guidance, AI/tooling instructions, and workflow scripts/config.
4. Collapse duplicate Pages/CI flows and redesign the VitePress site/navigation/copy.
5. Update GitHub About metadata, topics, and homepage URL via `gh`.
6. Run the closeout validation baseline, fix the blocking issues, and update tasks/specs as needed.
7. Archive the change after the repository matches the durable capability specs.

## Open Questions

- Whether any content in `specs.backup/` needs to be selectively restored into the new durable
  capability specs rather than archived as historical reference.
- Which source-level defects uncovered by the stabilization pass require spec changes versus pure
  implementation fixes.
