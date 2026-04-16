# .kiro Directory

> Kiro 规范文档系统

---

## Purpose

`.kiro` 目录用于存储项目规范文档，包括需求、设计和任务计划。

---

## Structure

```
.kiro/
└── specs/                    # Specification documents
    ├── README.md             # Index file
    ├── heterogeneous-task-scheduler/
    │   ├── design.md
    │   ├── requirements.md
    │   └── tasks.md
    └── project-quality-assessment/
        ├── design.md
        ├── requirements.md
        └── tasks.md
```

---

## Usage

规范文档遵循 Kiro 标准格式：

1. **design.md** - 架构设计和组件接口
2. **requirements.md** - 用户故事和验收标准
3. **tasks.md** - 实现任务和检查点

---

## Ignore

此目录已添加到 `.gitignore` 的 CI 检查排除列表中。
