# 异构任务调度器（HTS）

[![GitHub Release](https://img.shields.io/github/v/release/AICL-Lab/heterogeneous-task-scheduler?include_prereleases&logo=github)](https://github.com/AICL-Lab/heterogeneous-task-scheduler/releases)
[![CI](https://img.shields.io/github/actions/workflow/status/AICL-Lab/heterogeneous-task-scheduler/ci.yml?label=CI)](https://github.com/AICL-Lab/heterogeneous-task-scheduler/actions/workflows/ci.yml)
[![Docs](https://img.shields.io/badge/docs-GitHub%20Pages-blue)](https://aicl-lab.github.io/heterogeneous-task-scheduler/)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

[English](README.md) | 简体中文

> 面向 CPU / GPU 工作负载的 C++17 DAG 调度库，并提供适合本地开发与 CI 的 CPU-only 路径。

## HTS 提供什么

- 以 DAG 为中心的任务编排能力，包含 `TaskGraph`、依赖追踪与可插拔调度策略
- CPU 与 CUDA 执行路径，并提供无 CUDA 硬件环境可用的 CPU-only stub
- barrier、future、retry、profiling、event、resource limit 等运行时工具
- 围绕本地验证与事实性文档的轻量维护流程

## 快速开始

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
```

如需生成 compile commands 工作区，可使用 CPU-only preset：

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

## 验证基线

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

## 仓库结构

| 路径 | 用途 |
|------|------|
| `include/hts/` | 公共头文件 |
| `src/core/` | 共享调度、任务图与运行时核心 |
| `src/cuda/` | CUDA 实现与 CPU-only stub |
| `tests/` | 单元与集成测试 |
| `examples/` | 可运行示例 |
| `scripts/` | 构建、测试、格式检查、分析与辅助脚本 |
| `docs/adr/` | 架构决策记录 |
| `website/` | GitHub Pages 的 VitePress 源码 |
| `CHANGELOG.md` | 唯一项目变更历史 |

## 文档入口

- **GitHub Pages：** 架构、API、示例与使用说明
- **`CONTRIBUTING.md`：** 贡献流程与评审要求
- **`CHANGELOG.md`：** 发布记录与重要变更
- **`docs/adr/`：** 长期保留的架构决策

## 参与贡献

保持改动聚焦、表述准确、维护成本低。代码行为发生变化时同步更新文档；涉及代码时补充或调整测试；提交 PR 前运行完整验证基线。

## 许可证

项目基于 [MIT License](LICENSE) 发布。
