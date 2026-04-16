# .kiro Directory

[![Kiro](https://img.shields.io/badge/Kiro-Spec%20System-purple)]()
[![Version](https://img.shields.io/badge/Version-1.0.0-blue)]()

> Kiro 规范文档系统 — 项目需求、设计、任务一体化管理

---

## Purpose

`.kiro` 目录是项目规范文档的集中存储位置，采用 **Kiro 规范格式** 组织需求、设计和任务文档，确保项目开发过程的可追溯性和一致性。

### 核心价值

| 价值 | 描述 |
|------|------|
| **可追溯性** | 需求 → 设计 → 任务 → 实现 全链路追踪 |
| **一致性** | 统一的文档格式和结构 |
| **协作性** | 清晰的任务分解和状态跟踪 |
| **文档化** | 设计决策的永久记录 |

---

## Structure

```
.kiro/
├── README.md                              # 本文件 (索引和说明)
└── specs/                                 # 规范文档目录
    ├── README.md                          # 规范索引
    ├── heterogeneous-task-scheduler/      # 核心调度器规范
    │   ├── design.md                      # 架构设计
    │   ├── requirements.md                # 功能需求
    │   └── tasks.md                       # 实现任务
    └── project-quality-assessment/        # 项目质量评估规范
        ├── design.md                      # 质量设计
        ├── requirements.md                # 质量需求
        └── tasks.md                       # 改进任务
```

---

## Document Types

每个规范目录包含三个标准文档：

### 1. requirements.md — 需求文档

定义功能需求、验收标准和用户故事。

```markdown
### REQ-N: Requirement Title

**User Story:** As a <role>, I want <feature>.

| ID | Acceptance Criteria |
|----|---------------------|
| N.1 | WHEN condition, system SHALL behavior |
| N.2 | IF condition, component SHALL response |
```

### 2. design.md — 设计文档

描述架构设计、组件接口和正确性属性。

```markdown
## Components

### ComponentName

Brief description.

```cpp
class ComponentName {
    // Public interface
};
```

## Correctness Properties

| # | Property | Validates |
|---|----------|-----------|
| 1 | Property name | REQ-N.M |
```

### 3. tasks.md — 任务清单

分解实现步骤，跟踪进度。

```markdown
### ✅ Task N: Task Title

- [x] N.1 Subtask description
      _Requirements: REQ-N.M_
- [ ] N.2 Subtask description
      _Requirements: REQ-N.K_
```

---

## Status Indicators

| Symbol | Meaning | Usage |
|--------|---------|-------|
| ✅ | Complete | 任务已完成，测试通过 |
| 📋 | Planned | 计划中，尚未开始 |
| ⏸️ | Deferred | 推迟，优先级较低 |
| ⚠️ | Needs Attention | 需要关注或存在问题 |
| 🔄 | In Progress | 正在进行中 |

---

## Workflow

### 1. 创建新规范

```bash
# 创建新的规范目录
mkdir -p .kiro/specs/new-feature

# 创建三个标准文档
touch .kiro/specs/new-feature/{requirements,design,tasks}.md
```

### 2. 编写需求

从用户角度描述功能需求：

1. 定义用户故事
2. 编写验收标准
3. 建立需求追溯

### 3. 设计架构

根据需求设计系统：

1. 绘制架构图
2. 定义组件接口
3. 识别正确性属性

### 4. 分解任务

将设计转化为可执行任务：

1. 按阶段组织
2. 设置检查点
3. 关联需求编号

### 5. 更新状态

随着开发进展更新任务状态：

1. 开始任务 → 🔄 In Progress
2. 完成任务 → ✅ Complete
3. 遇到阻塞 → ⚠️ Needs Attention

---

## Integration

### 与 Git 集成

- `.gitignore`: CI 检查排除 `.kiro` 目录
- Commit message: 引用需求编号 (如 `feat: add feature (REQ-1.1)`)
- PR description: 链接到相关规范文档

### 与 CI 集成

```yaml
# .github/workflows/ci.yml
exclude-regex: '(^|/)(\.kiro)(/|$)'
```

---

## Best Practices

### 需求编写

- ✅ 使用 SHALL 表示强制要求
- ✅ 使用 SHOULD 表示推荐行为
- ✅ 每个需求有唯一编号
- ✅ 需求可测试、可验证

### 设计文档

- ✅ 包含架构图
- ✅ 定义公共接口
- ✅ 说明设计决策
- ✅ 列出正确性属性

### 任务清单

- ✅ 任务可独立完成
- ✅ 有明确的完成标准
- ✅ 关联需求编号
- ✅ 设置合理的检查点

---

## Related

| Resource | Link |
|----------|------|
| Specification Index | [specs/README.md](specs/README.md) |
| Main README | [../README.md](../README.md) |
| Contributing | [../CONTRIBUTING.md](../CONTRIBUTING.md) |

---

<p align="center">
  Made with ❤️ using the Kiro Specification System
</p>
