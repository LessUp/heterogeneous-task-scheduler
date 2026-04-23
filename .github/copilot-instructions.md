# GitHub Copilot Instructions for HTS

## Project mode

HTS is in **closeout and stabilization mode**. Prefer simplification, correctness, and maintenance
reduction over new feature expansion.

## Required read order

1. `AGENTS.md`
2. `CLAUDE.md`
3. Active OpenSpec change artifacts in `openspec/changes/<change>/`
4. Touched source/config files

If a material repository change is requested and no active OpenSpec change exists, propose or refine
the change before broad implementation.

## Working rules

- Treat `openspec/specs/` as the durable spec surface and active change specs as the current delta.
- Preserve unrelated user edits in a dirty worktree.
- Prefer deleting or consolidating redundant docs, workflows, and config over adding parallel paths.
- Keep README, website, and GitHub metadata fact-based and consistent.
- Prefer repo-local, lightweight tooling guidance over heavyweight integrations.

## Validation defaults

Use existing project commands unless the active change updates them:

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

CPU-only validation is the default baseline unless the task specifically targets CUDA behavior.

## GitHub operations

Use `gh` when the active change requires updates to repository description, homepage URL, topics, or
GitHub Pages alignment.
