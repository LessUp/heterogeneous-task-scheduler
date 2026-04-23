# AGENTS.md

## Mission

HTS is in **closeout mode**. Optimize for stabilization, architectural cleanup, workflow
simplification, and credible project presentation. Do not add speculative features or generic
process cargo-culting.

## Source of truth

- Durable specs live in `openspec/specs/`.
- Active change requirements live in `openspec/changes/<change>/`.
- `specs` is a compatibility symlink to `openspec/specs`.
- Material repository changes MUST be driven by an active OpenSpec change.
- If implementation and specs diverge, update the change artifacts first, then code.

## Required OpenSpec workflow

1. **Explore first when unclear**
   - Use `/opsx:explore` to investigate scope, trade-offs, and architectural consequences.
2. **Propose before material work**
   - Use `/opsx:propose "<change-name>"` for any non-trivial repo, workflow, code, or docs change.
   - Keep one active closeout-oriented change at a time unless parallel changes are clearly safer.
3. **Apply tasks in order**
   - Use `/opsx:apply <change-name>` and complete tasks sequentially unless the task graph proves
     the work is independent.
4. **Review at milestone boundaries**
   - Run `/review` after major governance changes, workflow rewrites, broad doc rewrites, and before
     merge-ready closeout checkpoints.
5. **Archive only after validation**
   - Use `/opsx:archive <change-name>` only after repo state, specs, and validation results match.

## Closeout operating rules

- Prefer **long-running autopilot sessions** over `/fleet`.
- Only use `/fleet` when the work naturally decomposes into independent streams.
- Preserve unrelated worktree changes; do not revert user edits outside the active scope.
- Remove or merge redundant docs, workflows, and config instead of layering new files on top.
- Public-facing claims must be evidence-based and consistent across README, website, and GitHub
  metadata.
- Prefer repo-local guidance and lightweight tooling over heavyweight MCP/plugin stacks.

## Repository map

- `include/hts/`: public headers
- `src/core/`: CPU-side scheduler and graph primitives
- `src/cuda/`: CUDA implementations and CPU-only stubs
- `tests/`: unit and integration tests
- `scripts/`: build, test, format, and analysis entry points
- `website/`: VitePress-based GitHub Pages site
- `.github/workflows/`: canonical CI and Pages automation
- `openspec/`: change artifacts and durable specs

## Validation baseline

Use the existing repo commands unless a change explicitly updates them:

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh          # after a build that generates compile_commands.json
cd website && npm run docs:build
```

CPU-only validation is the default baseline for automation unless the task specifically requires
CUDA-capable execution.

## Documentation rules

- Keep only docs that govern delivery or help users evaluate HTS.
- Prefer one maintained explanation over multiple overlapping documents.
- Treat `specs.backup/` as reference-only historical material.
- Keep AGENT/Claude/Copilot guidance specific to HTS closeout and maintenance work.

## GitHub operations

- Use `gh` for repository About metadata, topics, and homepage updates when the active change calls
  for remote alignment.
- Keep GitHub Pages content, workflow config, and repository metadata consistent with the maintained
  site.

## Tooling stance

- GitHub Copilot, Claude, and repo-local editor/LSP settings are the primary supported AI/dev
  surfaces.
- Add MCP servers, plugins, or alternative tool integrations only when they provide a clear HTS
  maintenance benefit and do not meaningfully increase cognitive overhead.
