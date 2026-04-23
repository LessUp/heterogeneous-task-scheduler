# Heterogeneous Task Scheduler (HTS)

[![GitHub Release](https://img.shields.io/github/v/release/LessUp/heterogeneous-task-scheduler?include_prereleases&logo=github)](https://github.com/LessUp/heterogeneous-task-scheduler/releases)
[![CI](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml/badge.svg)](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-optional-green.svg)](https://developer.nvidia.com/cuda-toolkit)

[English](README.md) | [简体中文](README.zh-CN.md)

> C++17 task-scheduler library for DAG-shaped workloads across CPU and GPU, with CPU-only fallback
> for local development and CI.

## What HTS provides

- **DAG-first scheduling** with `TaskGraph`, dependency tracking, and pluggable scheduling policies
- **Mixed CPU/GPU execution** through CPU tasks, CUDA-backed execution paths, and CPU-only stubs
- **Runtime utilities** including task barriers, futures, retries, events, resource limiting, and
  profiling
- **A practical engineering surface** with CPU-only validation scripts, GitHub Pages docs, and
  OpenSpec-driven change management

## Quick start

```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Recommended baseline for contributors and CI
scripts/build.sh --cpu-only
scripts/test.sh
```

For a local workspace setup with compile commands, use the `cpu-only-debug` preset:

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

## Learn more

| Resource | Purpose |
|----------|---------|
| [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/) | Product overview, architecture, API, and examples |
| [docs/README.md](docs/README.md) | Source documentation index |
| [examples/](examples/) | Runnable sample programs |
| [specs/README.md](specs/README.md) | Durable OpenSpec surface |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Contribution workflow and local guard rails |

## Repository layout

| Path | Purpose |
|------|---------|
| `include/hts/` | Public headers |
| `src/core/` | Core scheduler and graph primitives |
| `src/cuda/` | CUDA implementation and CPU-only stubs |
| `tests/` | Unit and integration tests |
| `scripts/` | Build, test, formatting, analysis, and hook installation entry points |
| `website/` | VitePress site for GitHub Pages |
| `openspec/` | Active changes and durable specs |

## Validation commands

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

## Contributing

Material changes to code, docs, workflows, or public project surfaces are expected to go through an
active OpenSpec change. Start with:

1. `/opsx:explore` if scope is unclear
2. `/opsx:propose "<change-name>"`
3. `/opsx:apply <change-name>`
4. `/review` at major milestones
5. `/opsx:archive <change-name>` after validation

See [AGENTS.md](AGENTS.md), [CLAUDE.md](CLAUDE.md), and [CONTRIBUTING.md](CONTRIBUTING.md) for the
project-specific workflow.

## License

Released under the [MIT License](LICENSE).
