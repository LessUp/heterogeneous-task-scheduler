# HTS Specification Documents

[![Specs](https://img.shields.io/badge/Specs-v1.1-blue)]()

> Heterogeneous Task Scheduler 项目规范文档索引

---

## Overview

本目录包含 HTS 项目的完整规范文档，采用 Kiro 规范格式组织。

---

## Directory Structure

```
.kiro/specs/
├── README.md                              # This file
├── heterogeneous-task-scheduler/          # Core scheduler specs
│   ├── design.md                          # Architecture & component design
│   ├── requirements.md                    # Functional requirements
│   └── tasks.md                           # Implementation tasks
└── project-quality-assessment/            # Quality assessment specs
    ├── design.md                          # Quality evaluation results
    ├── requirements.md                    # Quality requirements
    └── tasks.md                           # Improvement tasks
```

---

## Document Index

### Heterogeneous Task Scheduler

| Document | Purpose | Status |
|----------|---------|--------|
| [design.md](heterogeneous-task-scheduler/design.md) | 架构设计、组件接口、正确性属性 | ✅ Complete |
| [requirements.md](heterogeneous-task-scheduler/requirements.md) | 8 个功能需求、验收标准 | ✅ Complete |
| [tasks.md](heterogeneous-task-scheduler/tasks.md) | 15 个实现任务 | ✅ Complete |

### Project Quality Assessment

| Document | Purpose | Status |
|----------|---------|--------|
| [design.md](project-quality-assessment/design.md) | 质量评估、改进方案 | ✅ Complete |
| [requirements.md](project-quality-assessment/requirements.md) | 10 个质量需求 | ✅ Complete |
| [tasks.md](project-quality-assessment/tasks.md) | 12 个改进任务 | ✅ In Progress |

---

## Specification Format

每个规范目录包含三个标准文档：

| Document | Description |
|----------|-------------|
| **design.md** | 架构设计、组件接口、数据模型、正确性属性 |
| **requirements.md** | 用户故事、验收标准、需求追溯 |
| **tasks.md** | 实现任务、检查点、优先级标记 |

---

## Status Indicators

| Symbol | Meaning |
|--------|---------|
| ✅ | Complete |
| 📋 | Planned |
| ⏸️ | Deferred |
| ⚠️ | Needs Attention |

---

## Related Links

- [Main README](../../README.md)
- [CHANGELOG](../../CHANGELOG.md)
- [CONTRIBUTING](../../CONTRIBUTING.md)
