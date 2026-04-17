# Project Philosophy: Spec-Driven Development (SDD)

本项目严格遵循**规范驱动开发（Spec-Driven Development）**范式。所有的代码实现必须以 `/specs` 目录下的规范文档为唯一事实来源（Single Source of Truth）。

---

## Directory Context (目录说明)

| Directory | Purpose |
|-----------|---------|
| `/specs/product/` | 产品功能定义与验收标准 |
| `/specs/rfc/` | 技术设计文档（RFC）与架构决策 |
| `/specs/api/` | API 接口定义（OpenAPI, gRPC proto, etc.） |
| `/specs/db/` | 数据库 Schema 设计规范 |
| `/specs/testing/` | BDD 测试用例规范与验收测试定义 |
| `/docs/` | 用户文档（教程、指南、API 参考） |

---

## AI Agent Workflow Instructions (AI 工作流指令)

当你（AI）被要求开发一个新功能、修改现有功能或修复 Bug 时，**必须严格按照以下工作流执行，不可跳过任何步骤**：

### Step 1: Review Specs (审查与分析)

- 在编写任何代码之前，首先阅读 `/specs` 目录下相关的产品文档、RFC 和 API 定义。
- 如果用户指令与现有 Spec 冲突，请立即停止编码，并指出冲突点，询问用户是否需要先更新 Spec。

### Step 2: Spec-First Update (规范优先)

- 如果这是一个新功能，或者需要改变现有的接口/数据库结构，**必须首先提议修改或创建相应的 Spec 文档**（如 `openapi.yaml` 或 RFC 文档）。
- 等待用户确认 Spec 的修改后，才能进入代码编写阶段。

### Step 3: Implementation (代码实现)

- 编写代码时，必须 **100% 遵守 Spec 中的定义**（包括变量命名、API 路径、数据类型、状态码等）。
- 不要在代码中擅自添加 Spec 中未定义的功能（No Gold-Plating）。

### Step 4: Test Against Spec (测试验证)

- 根据 `/specs` 中的验收标准（Acceptance Criteria）编写单元测试和集成测试。
- 确保测试用例覆盖了 Spec 中描述的所有边界情况。

---

## Code Generation Rules (代码生成规则)

- 任何对外部暴露的 API 变更，必须同步修改 `/specs/api/` 或 `/specs/product/` 中对应的规范文件。
- 如果遇到不确定的技术细节，请查阅 `/specs/rfc/` 下的架构约定，不要自行捏造设计模式。
- 所有文档变更应同时在 `/docs/en/` 和 `/docs/zh-CN/` 中反映，保持双语一致性。

---

## Documentation Standards (文档标准)

- `README.md` 默认为英文，并链接到中文版 (`README.zh-CN.md`)。
- `/docs/` 中所有面向用户的文档必须有英文和中文两个版本。
- Spec 文档优先使用英文编写，必要时可提供中文翻译。

---

## Project-Specific Context (项目特定上下文)

### Project Type

本项目是一个 **C++/CUDA 异构任务调度框架**，主要特性：

- DAG 任务图管理与依赖追踪
- GPU 内存池（Buddy System 分配器）
- CPU 线程池 + CUDA Streams 异步执行
- 多种调度策略（GPU-first, CPU-first, Round-robin）
- 性能监控与分析

### Key Components

| Component | Description | Spec Link |
|-----------|-------------|-----------|
| Scheduler | 核心调度器 | [RFC-001](specs/rfc/001-core-architecture.md) |
| TaskGraph | DAG 管理 | [RFC-001](specs/rfc/001-core-architecture.md) |
| MemoryPool | GPU 内存池 | [RFC-002](specs/rfc/002-memory-pool.md) |
| ExecutionEngine | 执行引擎 | [RFC-003](specs/rfc/003-execution-engine.md) |
| Error Handling | 错误处理 | [RFC-004](specs/rfc/004-error-handling.md) |
| Profiler | 性能监控 | [RFC-005](specs/rfc/005-profiling.md) |

### Code Style

- **命名约定**：类名 PascalCase，函数/变量 snake_case
- **格式化**：4 空格缩进，100 字符行宽
- **注释**：公共 API 使用 Doxygen 风格

### Commit Convention

遵循 [Conventional Commits](https://www.conventionalcommits.org/)：

```
<type>(<scope>): <description>

Types: feat, fix, docs, style, refactor, perf, test, chore
```

---

## Related Documents

- [Specifications Index](specs/README.md)
- [Contributing Guide](CONTRIBUTING.md)
- [API Reference](docs/en/api-reference.md)
