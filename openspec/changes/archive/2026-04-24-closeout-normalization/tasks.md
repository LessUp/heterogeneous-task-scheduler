## 1. OpenSpec governance reset

- [x] 1.1 Add `openspec/config.yaml` and normalize the canonical `openspec/` and `specs` layout.
- [x] 1.2 Replace placeholder live spec scaffolding with durable HTS closeout governance structure.
- [x] 1.3 Rewrite `AGENTS.md` around the HTS closeout workflow, review gates, and OpenSpec-first rules.

## 2. Governance and AI instructions

- [x] 2.1 Create a project-specific `CLAUDE.md` for HTS closeout and maintenance work.
- [x] 2.2 Add `.github/copilot-instructions.md` and align contributor-facing templates and guidance.
- [x] 2.3 Add a lightweight versioned hook setup and document when to use `/review` during major work.

## 3. Engineering surface rationalization

- [x] 3.1 Collapse duplicated GitHub Actions into one canonical validation workflow and one Pages deployment workflow.
- [x] 3.2 Fix script and configuration drift across build, test, analysis, version pinning, and editor/LSP settings.
- [x] 3.3 Record the minimal HTS stance for MCP, OpenCode, plugins, and other optional tool integrations.

## 4. Project showcase and public surfaces

- [x] 4.1 Rewrite the maintained README, contributing, changelog, and supporting docs to remove low-value duplication.
- [x] 4.2 Redesign the VitePress site structure and homepage copy around product explanation, architecture, and examples.
- [x] 4.3 Update GitHub About metadata, homepage URL, and curated topics via `gh` to match the maintained landing surface.

## 5. Stabilization and closeout validation

- [x] 5.1 Re-establish a trustworthy CPU-only validation baseline for build, tests, formatting, and documentation.
- [x] 5.2 Sweep source, tests, scripts, and docs for concrete defects or spec drift, then fix the blocking issues.
- [x] 5.3 Re-run the closeout validation matrix and reconcile any remaining mismatches in specs or tasks.

## 6. Archive readiness

- [x] 6.1 Review the normalized repository diff, including remote GitHub settings, against the closeout specs.
- [x] 6.2 Run a formal `/review` checkpoint on the major repository normalization changes and address the findings.
- [x] 6.3 Archive `closeout-normalization` once repository state, public surfaces, and validation results match the specs.
