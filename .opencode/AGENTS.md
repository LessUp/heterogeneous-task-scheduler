# OpenCode guidance for HTS

Read `AGENTS.md` and `CLAUDE.md` first, then work from the active OpenSpec change.

## Default stance

- HTS is in closeout mode: stabilize, simplify, and remove low-value surface area.
- Use the active OpenSpec tasks as the execution order.
- Prefer CPU-only validation and the `cpu-only-debug` CMake preset for local iteration.
- Keep GitHub metadata and Pages changes aligned with repo files through `gh`.

## Tool boundaries

- No project-level MCP configuration is checked in by default.
- Only add new MCP or plugin integrations when they create a clear HTS-specific maintenance gain.
- Prefer repo-local guidance, lightweight editor settings, and `/review` checkpoints over broad tool
  proliferation.
