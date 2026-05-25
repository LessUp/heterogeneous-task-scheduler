---
layout: home
hero:
  name: HTS
  text: Heterogeneous Task Scheduler
  tagline: C++17 DAG scheduling library with a CPU-only validation path and optional CUDA execution.
  actions:
    - theme: brand
      text: Guide
      link: /en/guide/
    - theme: alt
      text: API
      link: /en/api/
    - theme: alt
      text: Examples
      link: /en/examples/
features:
  - title: One maintained doc surface
    details: GitHub Pages now covers only stable guide, API, and example pages. Speculative whitepapers, research notes, and placeholder benchmarks are intentionally removed.
  - title: CPU-first contributor workflow
    details: Local development starts with the CPU-only build, full test suite, formatting check, static analysis, and docs build before any CUDA-specific work.
  - title: Focused runtime library
    details: The maintained core is the task graph, scheduler, execution engine, memory pool, scheduling policies, and a small set of runtime utilities.
---

## Recommended starting path

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
```

Then use the [Guide](/en/guide/) for the development workflow, [API Overview](/en/api/) for the stable
public headers, and [Examples](/en/examples/) for runnable programs under `examples/`.

## Repository boundaries

| Surface | What stays here |
|---------|------------------|
| `README.md` / `README.zh-CN.md` | Project entry points and validation baseline |
| `website/` | Maintained guide, API overview, and example pages |
| `docs/adr/` | Long-lived architecture decisions |
| `CHANGELOG.md` | The single project changelog |

## What was removed

- Whitepaper and research branches that were not maintained as part of the real code workflow
- Placeholder benchmark pages that referenced binaries the repository does not build
- Stale API pages that documented methods and types that do not exist in the current library
