# Changelog Directory

[![Keep a Changelog](https://img.shields.io/badge/Keep%20a%20Changelog-1.1.0-blue)](https://keepachangelog.com/)
[![Semantic Versioning](https://img.shields.io/badge/Semantic%20Versioning-2.0.0-blue)](https://semver.org/)

> Comprehensive changelog index with bilingual support | 带有双语支持的完整变更日志索引

---

## 🌐 Languages | 语言

- [English](#english)
- [简体中文](#中文)

---

## English

### Purpose

This directory contains detailed changelog entries for significant infrastructure and architectural changes. The main [CHANGELOG.md](../CHANGELOG.md) provides version summaries, while this directory offers in-depth technical documentation.

### Index

| Date | File | Type | Description |
|------|------|------|-------------|
| 2026-03-13 | [workflow-cpu-safe-ci.md](2026-03-13_workflow-cpu-safe-ci.md) | Infrastructure | Migration from CUDA containers to clang-format |
| 2026-03-10 | [workflow-deep-standardization.md](2026-03-10_workflow-deep-standardization.md) | Infrastructure | Workflow permissions and concurrency standardization |
| 2026-03-09 | [workflow-optimization.md](2026-03-09_workflow-optimization.md) | Infrastructure | Initial CI workflow configuration |

### Naming Convention

```
YYYY-MM-DD_topic-description.md
```

| Component | Format | Example |
|-----------|--------|---------|
| Date | ISO 8601 | `2026-03-13` |
| Topic | kebab-case | `workflow-optimization` |
| Extension | `.md` | Markdown |

### File Structure

Each changelog entry follows this professional format:

```markdown
# Title

**Date**: YYYY-MM-DD
**Type**: Feature | Bug Fix | Infrastructure | Documentation | Refactor
**Impact**: Critical | High | Medium | Low
**PR**: #XXX (or "-" if none)

## Summary

One-paragraph description of the change.

## Background

Context and motivation for the change.

## Changes

### Added
- New features

### Changed
- Modifications

### Removed
- Deleted features

### Fixed
- Bug fixes

## Technical Details

Code snippets, configuration, or technical explanations.

## Impact

- User impact
- Developer impact
- System impact

## Lessons Learned

Key takeaways from this change.

## References

Links to related resources.
```

---

## 中文

### 用途

本目录包含重大基础设施和架构变更的详细变更日志条目。主 [CHANGELOG.md](../CHANGELOG.md) 提供版本摘要，本目录提供深入的技术文档。

### 索引

| 日期 | 文件 | 类型 | 描述 |
|------|------|------|------|
| 2026-03-13 | [workflow-cpu-safe-ci.md](2026-03-13_workflow-cpu-safe-ci.md) | 基础设施 | 从 CUDA 容器迁移到 clang-format |
| 2026-03-10 | [workflow-deep-standardization.md](2026-03-10_workflow-deep-standardization.md) | 基础设施 | 工作流权限和并发控制标准化 |
| 2026-03-09 | [workflow-optimization.md](2026-03-09_workflow-optimization.md) | 基础设施 | 初始 CI 工作流配置 |

### 文件命名规范

```
YYYY-MM-DD_topic-description.md
```

| 组成部分 | 格式 | 示例 |
|---------|------|------|
| 日期 | ISO 8601 | `2026-03-13` |
| 主题 | 短横线连接 | `workflow-optimization` |
| 扩展名 | `.md` | Markdown |

### 文件结构

每个变更日志条目遵循以下专业格式：

```markdown
# 标题

**日期**: YYYY-MM-DD
**类型**: Feature | Bug Fix | Infrastructure | Documentation | Refactor
**影响**: Critical | High | Medium | Low
**PR**: #XXX (或 "-" 如果没有)

## 摘要

变更的一段描述。

## 背景

变更的上下文和动机。

## 变更

### 新增
- 新特性

### 修改
- 功能修改

### 移除
- 删除的功能

### 修复
- Bug 修复

## 技术细节

代码片段、配置或技术说明。

## 影响

- 用户影响
- 开发者影响
- 系统影响

## 经验教训

本次变更的主要收获。

## 参考

相关资源链接。
```

---

## Templates | 模板

See [templates/](templates/) directory for changelog entry templates.

---

## Links | 链接

- [Main Changelog](../CHANGELOG.md) | [主变更日志](../CHANGELOG.md)
- [GitHub Releases](https://github.com/LessUp/heterogeneous-task-scheduler/releases)
- [Contributing Guide](../CONTRIBUTING.md) | [贡献指南](../CONTRIBUTING.md)
