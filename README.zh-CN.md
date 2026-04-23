# 异构任务调度器（HTS）

[![GitHub Release](https://img.shields.io/github/v/release/LessUp/heterogeneous-task-scheduler?include_prereleases&logo=github)](https://github.com/LessUp/heterogeneous-task-scheduler/releases)
[![CI](https://img.shields.io/github/actions/workflow/status/LessUp/heterogeneous-task-scheduler/ci.yml?label=CI)](https://github.com/LessUp/heterogeneous-task-scheduler/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://lessup.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![CUDA](https://img.shields.io/badge/CUDA-optional-green.svg)](https://developer.nvidia.com/cuda-toolkit)

[English](README.md) | 简体中文

> 面向 CPU / GPU 混合负载的 C++17 DAG 调度库，并提供适合本地开发和 CI 的 CPU-only 回退路径。

## HTS 提供什么

- **以 DAG 为中心的调度模型**：提供 `TaskGraph`、依赖追踪与可插拔调度策略
- **CPU / GPU 混合执行能力**：支持 CPU 任务、CUDA 执行路径和 CPU-only stub
- **运行时工具集**：包含 barrier、future、retry、event、resource limiter 与 profiler
- **务实的工程化表面**：提供 CPU-only 验证脚本、GitHub Pages 文档和 OpenSpec 驱动流程

## 快速开始

```bash
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 贡献者和 CI 的推荐基线
scripts/build.sh --cpu-only
scripts/test.sh
```

如需本地 LSP / compile commands 工作区，可使用 `cpu-only-debug` preset：

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

## 进一步阅读

| 资源 | 用途 |
|------|------|
| [GitHub Pages](https://lessup.github.io/heterogeneous-task-scheduler/) | 产品概览、架构、API 与示例 |
| [docs/README.md](docs/README.md) | 仓库内文档索引 |
| [examples/](examples/) | 可运行示例 |
| [specs/README.md](specs/README.md) | 持久 OpenSpec 规范入口 |
| [CONTRIBUTING.md](CONTRIBUTING.md) | 贡献流程与本地质量门 |

## 仓库结构

| 路径 | 用途 |
|------|------|
| `include/hts/` | 公共头文件 |
| `src/core/` | 核心调度与任务图实现 |
| `src/cuda/` | CUDA 实现与 CPU-only stub |
| `tests/` | 单元测试与集成测试 |
| `scripts/` | 构建、测试、格式检查、分析与 hook 安装入口 |
| `website/` | 用于 GitHub Pages 的 VitePress 站点 |
| `openspec/` | 活跃变更与持久规范 |

## 验证命令

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

## 贡献方式

对代码、文档、workflow 或项目展示面的重大修改，预期都应经过一个活跃的 OpenSpec change：

1. 范围不清楚时先用 `/opsx:explore`
2. 使用 `/opsx:propose "<change-name>"`
3. 使用 `/opsx:apply <change-name>`
4. 在关键里程碑运行 `/review`
5. 验证通过后使用 `/opsx:archive <change-name>`

详细规则见 [AGENTS.md](AGENTS.md)、[CLAUDE.md](CLAUDE.md) 和
[CONTRIBUTING.md](CONTRIBUTING.md)。

## 许可证

项目基于 [MIT License](LICENSE) 发布。
