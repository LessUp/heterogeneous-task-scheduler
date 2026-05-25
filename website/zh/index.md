---
layout: home
hero:
  name: HTS
  text: 异构任务调度器
  tagline: C++17 DAG 调度库，提供 CPU-only 验证路径，并可在具备 CUDA 时启用 GPU 执行。
  actions:
    - theme: brand
      text: 指南
      link: /zh/guide/
    - theme: alt
      text: API
      link: /zh/api/
    - theme: alt
      text: 示例
      link: /zh/examples/
features:
  - title: 单一可维护文档面
    details: GitHub Pages 只保留稳定的指南、API 概览和示例页面；预研白皮书、研究综述和占位基准页已移除。
  - title: CPU-first 开发流程
    details: 日常开发优先走 CPU-only 构建、完整测试、格式检查、静态分析与文档构建，再进入 CUDA 场景。
  - title: 聚焦的运行时核心
    details: 持续维护的核心只包括任务图、调度器、执行引擎、内存池、调度策略和少量运行时工具。
---

## 推荐入口

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
```

随后通过 [指南](/zh/guide/) 了解开发流程，通过 [API 概览](/zh/api/) 查看稳定公共头文件，
并在 [示例](/zh/examples/) 页面中找到 `examples/` 下的可运行程序。

## 仓库边界

| 面 | 保留内容 |
|----|----------|
| `README.md` / `README.zh-CN.md` | 项目入口与验证基线 |
| `website/` | 持续维护的指南、API 概览、示例页面 |
| `docs/adr/` | 长期保留的架构决策 |
| `CHANGELOG.md` | 唯一项目变更历史 |

## 已移除内容

- 未纳入真实代码工作流维护的白皮书与研究分支
- 指向仓库中并不存在构建产物的占位 benchmark 页面
- 与当前库实现不一致的旧 API 细页
