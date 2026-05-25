# Heterogeneous Task Scheduler (HTS)

[![GitHub Release](https://img.shields.io/github/v/release/AICL-Lab/heterogeneous-task-scheduler?include_prereleases&logo=github)](https://github.com/AICL-Lab/heterogeneous-task-scheduler/releases)
[![CI](https://github.com/AICL-Lab/heterogeneous-task-scheduler/actions/workflows/ci.yml/badge.svg)](https://github.com/AICL-Lab/heterogeneous-task-scheduler/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://aicl-lab.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

[English](README.md) | [简体中文](README.zh-CN.md)

> C++17 DAG scheduler for CPU/GPU workloads, with a CPU-only path for local development and CI.

## What HTS provides

- DAG-first task orchestration with `TaskGraph`, dependency tracking, and pluggable scheduling policy
- CPU and CUDA execution paths with CPU-only stubs for environments without CUDA hardware
- Runtime utilities such as task barriers, futures, retries, profiling, events, and resource limits
- A lightweight contributor workflow centered on local validation and factual documentation

## Quick start

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
```

For a compile-commands workspace, use the CPU-only preset:

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

## Validation baseline

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

## Repository layout

| Path | Purpose |
|------|---------|
| `include/hts/` | Public headers |
| `src/core/` | Shared scheduler, graph, and runtime primitives |
| `src/cuda/` | CUDA implementations and CPU-only stubs |
| `tests/` | Unit and integration coverage |
| `examples/` | Runnable sample programs |
| `scripts/` | Build, test, format, analysis, and helper entry points |
| `docs/adr/` | Architecture decision records |
| `website/` | VitePress source for GitHub Pages |
| `CHANGELOG.md` | Single project changelog |

## Documentation

- **GitHub Pages:** architecture, API, examples, and user-facing guidance
- **`CONTRIBUTING.md`:** contributor workflow and review expectations
- **`CHANGELOG.md`:** release history and notable changes
- **`docs/adr/`:** durable architectural decisions

## Contributing

Keep changes small, factual, and easy to maintain. Update docs when behavior changes, add or adjust
tests when code changes, and run the validation baseline before opening a pull request.

## License

Released under the [MIT License](LICENSE).
