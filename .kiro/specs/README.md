# HTS Specification Documents

[![Specs](https://img.shields.io/badge/Specs-v1.1-blue)]()
[![Kiro](https://img.shields.io/badge/Format-Kiro-purple)]()
[![Status](https://img.shields.io/badge/Status-Complete-brightgreen)]()

> Heterogeneous Task Scheduler 项目规范文档索引 — 遵循 Kiro 规范格式

---

## Overview

本目录包含 HTS 项目的完整规范文档，采用 **Kiro 规范格式** 组织，确保需求、设计、任务的完整性和可追溯性。

### 规范层次

```
需求 (Requirements)
    ↓ 定义功能需求
设计 (Design)
    ↓ 描述架构和实现
任务 (Tasks)
    ↓ 分解实现步骤
实现 (Implementation)
    ↓ 完成功能
```

---

## Directory Structure

```
.kiro/specs/
├── README.md                              # 本文件 (索引)
│
├── heterogeneous-task-scheduler/          # 核心调度器规范
│   ├── design.md                          # 架构设计、组件接口、正确性属性
│   ├── requirements.md                    # 8 个功能需求、验收标准
│   └── tasks.md                           # 15 个实现任务 + 14 个属性测试
│
└── project-quality-assessment/            # 项目质量评估规范
    ├── design.md                          # 质量评估结果、改进方案
    ├── requirements.md                    # 10 个质量需求
    └── tasks.md                           # 12 个改进任务
```

---

## Document Index

### 🚀 Heterogeneous Task Scheduler

| Document | Purpose | Status | Lines |
|----------|---------|--------|-------|
| [design.md](heterogeneous-task-scheduler/design.md) | 架构设计、组件接口、正确性属性 | ✅ Complete | 241 |
| [requirements.md](heterogeneous-task-scheduler/requirements.md) | 8 个功能需求、验收标准、需求追溯 | ✅ Complete | 146 |
| [tasks.md](heterogeneous-task-scheduler/tasks.md) | 15 个实现任务、测试覆盖 | ✅ Complete | 183 |

**摘要**:
- 核心功能: DAG 任务调度、GPU 内存池、异步执行
- 组件数: 22 个头文件, 10 个源文件
- 测试: 18 个测试文件, 100% 单元测试覆盖
- 阶段: 4 个主要开发阶段

---

### 📊 Project Quality Assessment

| Document | Purpose | Status | Lines |
|----------|---------|--------|-------|
| [design.md](project-quality-assessment/design.md) | 质量评估、改进方案、优先级 | ✅ Complete | 189 |
| [requirements.md](project-quality-assessment/requirements.md) | 10 个质量需求、社区标准 | ✅ Complete | 162 |
| [tasks.md](project-quality-assessment/tasks.md) | 12 个改进任务、进度跟踪 | ✅ In Progress | 189 |

**摘要**:
- 已完成: 社区文件、GitHub 模板、CI/CD 管道
- 进行中: 待优化文档、安装支持、属性测试
- 质量评分: ⭐⭐⭐⭐⭐ (5/5)

---

## Specification Format

每个规范目录包含三个标准文档，遵循 Kiro 格式：

### 1️⃣ requirements.md — 需求文档

**目的**: 定义功能需求、验收标准、用户故事

**内容**:
- User Stories: 从用户角度描述需求
- Acceptance Criteria: 可验证的验收标准
- Requirements Traceability: 需求到实现的追溯矩阵

**示例**:
```markdown
### REQ-1: Task Definition

**User Story:** As a developer, I want to define tasks and dependencies as a DAG.

| ID | Acceptance Criteria |
|----|---------------------|
| 1.1 | TaskGraph SHALL allow creating tasks with unique IDs |
| 1.2 | WHEN adding dependency A→B, DependencyManager SHALL record B depends on A |
```

---

### 2️⃣ design.md — 设计文档

**目的**: 描述架构设计、组件接口、正确性属性

**内容**:
- Architecture Overview: 系统架构图
- Components: 组件详细设计和接口
- Correctness Properties: 需要验证的属性
- Error Handling: 错误处理策略
- Testing Strategy: 测试策略

**示例**:
```markdown
## Architecture

```
Scheduler
├── Dependency Manager
├── Scheduling Policy
└── Execution Engine
    ├── CPU Thread Pool
    └── GPU Stream Manager
```

## Correctness Properties

| # | Property | Validates |
|---|----------|-----------|
| 1 | Task Creation Invariants | REQ 1.1, 1.4 |
| 2 | Cycle Detection | REQ 1.3 |
```

---

### 3️⃣ tasks.md — 任务清单

**目的**: 分解实现任务、跟踪进度、设置检查点

**内容**:
- Phase Breakdown: 按开发阶段组织任务
- Task List: 详细子任务和完成标准
- Checkpoints: 阶段性验证点
- Status Tracking: 任务状态和优先级

**示例**:
```markdown
### ✅ Task 1: Project Structure and Core Types

- [x] 1.1 CMakeLists.txt with CUDA support
- [x] 1.2 Directory structure: src/, include/, tests/
- [x] 1.3 Core enums (DeviceType, TaskState)
- [ ] 1.4 Property test for Task creation (P1)
      _Requirements: REQ-1.1, REQ-1.4_
```

---

## Status Indicators

| Symbol | Meaning | Usage |
|--------|---------|-------|
| ✅ | Complete | 任务已完成、测试通过 |
| 📋 | Planned | 已计划、待开始 |
| ⏸️ | Deferred | 已推迟、优先级低 |
| 🔄 | In Progress | 正在进行中 |
| ⚠️ | Needs Attention | 需要关注 |

---

## Quick Links

### 规范文档

| 分类 | 文档 |
|------|------|
| 核心功能 | [Heterogeneous Task Scheduler](heterogeneous-task-scheduler/) |
| 质量改进 | [Project Quality Assessment](project-quality-assessment/) |

### 外部文档

| 资源 | 链接 |
|------|------|
| 主 README | [../README.md](../README.md) |
| 更新日志 | [../CHANGELOG.md](../CHANGELOG.md) |
| 贡献指南 | [../CONTRIBUTING.md](../CONTRIBUTING.md) |
| GitHub Issues | [Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues) |

---

## Usage Guidelines

### 对于贡献者

1. **开始新功能前**: 阅读 `requirements.md` 了解需求
2. **开始实现前**: 参考 `design.md` 了解架构
3. **实现过程中**: 跟踪 `tasks.md` 中的任务进度
4. **完成后**: 更新 `tasks.md` 状态为 ✅

### 对于维护者

1. **代码审查时**: 对照 `requirements.md` 验证需求
2. **架构评审时**: 参考 `design.md` 的设计决策
3. **发布规划时**: 查看 `tasks.md` 的剩余任务
4. **质量评估时**: 使用 `project-quality-assessment/` 的评估结果

### 对于用户

1. **了解功能**: 查看 `requirements.md` 的用户故事
2. **理解架构**: 阅读 `design.md` 的架构图
3. **报告问题**: 引用需求编号 (如 `REQ-1.1`)
4. **建议功能**: 描述用户故事

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| v1.1.0 | 2024-12-31 | 添加同步原语、事件系统、重试策略 |
| v1.0.0 | 2024-12-31 | 初始版本，核心调度器功能 |

---

## Conventions

### 需求编号

```
REQ-N: Requirement Title
    ├─ REQ-N.1: Sub-requirement
    ├─ REQ-N.2: Sub-requirement
    └─ REQ-N.M: Sub-requirement
```

### 任务编号

```
Task N: Task Title
    ├─ N.1: Subtask
    ├─ N.2: Subtask
    └─ N.M: Subtask
```

### 属性测试编号

```
Property N: Property Description
    _Validates: REQ-N.M_
```

---

## Templates

### 新规范目录

创建新的规范时，使用以下模板：

```bash
mkdir -p .kiro/specs/feature-name
touch .kiro/specs/feature-name/{requirements,design,tasks}.md
```

然后从 [Heterogeneous Task Scheduler](heterogeneous-task-scheduler/) 或 [Project Quality Assessment](project-quality-assessment/) 复制模板。

---

## Support

有问题或建议？

- 📖 [Kiro README](../README.md) — 规范系统说明
- 💬 [GitHub Discussions](https://github.com/LessUp/heterogeneous-task-scheduler/discussions)
- 🐛 [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)

---

<p align="center">
  <a href="../README.md">← Main README</a> ·
  <a href="../CHANGELOG.md">Changelog</a>
</p>

<p align="center">
  <b>HTS Specifications</b> · Kiro Format v1.0 · Last Updated: 2024-12-31
</p>
