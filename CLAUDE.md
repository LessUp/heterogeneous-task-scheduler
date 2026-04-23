# CLAUDE.md

## Role

You are working on **HTS closeout and stabilization**, not open-ended feature expansion. Favor
clarity, reliability, and reduction of maintenance surface.

## Read this first

1. `AGENTS.md`
2. `openspec/changes/<active-change>/proposal.md`
3. `openspec/changes/<active-change>/design.md`
4. `openspec/changes/<active-change>/specs/**/*.md`
5. `openspec/changes/<active-change>/tasks.md`

If there is no active change for material work, create or refine one before editing code or broad
project files.

## Default workflow

1. Explore ambiguity before coding.
2. Keep work tied to the active OpenSpec task you are completing.
3. Update the task checkbox immediately after a task is done.
4. Use `/review` after major governance, workflow, documentation, or broad refactor milestones.
5. Archive only after validation and remote GitHub metadata are aligned with the repo state.

## Project priorities

- Preserve unrelated user edits in a dirty worktree.
- Prefer deleting or merging low-value files over keeping redundant surface area.
- Keep public claims factual and evidence-backed.
- Prefer CPU-only validation paths unless CUDA behavior is the subject of the task.
- Keep AI/editor/tooling setup lightweight and repo-local.

## Supported commands and checks

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

Use `gh` when the active change requires GitHub metadata, topics, homepage, or Pages alignment.

## Avoid

- Adding speculative product features
- Reintroducing duplicate workflows or duplicate documentation paths
- Adding heavyweight MCP/plugin/tooling integrations without a clear HTS-specific payoff
- Treating `specs.backup/` as the live source of truth
