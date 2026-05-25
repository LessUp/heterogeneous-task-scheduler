# Contributing to Heterogeneous Task Scheduler (HTS)

[English](#english) | [中文](#中文)

---

## English

### Principles

- Favor simplification, correctness, and maintenance reduction over feature sprawl.
- Keep public claims in sync across code, README, GitHub Pages, and `CHANGELOG.md`.
- Preserve unrelated work when preparing a change.

### Development setup

#### Requirements

| Requirement | Version |
|-------------|---------|
| CMake | >= 3.18 |
| CUDA Toolkit | >= 11.0 (optional for CPU-only development) |
| C++ Compiler | GCC 8+, Clang 7+, or MSVC 2019+ |
| Node.js | 20.x for the docs site |

#### Recommended baseline

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

For a local workspace with compile commands:

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

### Code style

| Area | Rule |
|------|------|
| Naming | PascalCase for types, snake_case for functions and variables |
| Formatting | 4 spaces, K&R braces, line width 100 |
| Public APIs | Use Doxygen-style comments when documentation adds real value |

Prefer focused edits, explicit invariants, and tests that cover real behavior rather than mocks.

### Documentation expectations

- Update `README.md` / `README.zh-CN.md` when contributor-facing guidance changes.
- Update GitHub Pages content when public behavior or examples change.
- Keep release history only in the root `CHANGELOG.md`.

### Pull requests

Before opening a PR:

1. Keep the change scoped to one coherent problem.
2. Add or update tests for code changes.
3. Run the validation baseline.
4. Update documentation when behavior or workflows change.
5. Use a clear Conventional Commit-style subject line when committing.

#### PR checklist

- [ ] Build and tests pass
- [ ] Formatting and analysis pass
- [ ] Docs updated when needed
- [ ] `CHANGELOG.md` updated when needed

### Reporting bugs

Please include:

- OS, compiler, CUDA version, and build mode
- Reproduction steps
- Expected behavior and actual behavior
- Minimal code sample when possible

### Feature requests

Describe the problem first, then the proposed change. Requests that simplify the codebase,
maintenance flow, or documentation surface are preferred over speculative expansion.

---

## 中文

### 基本原则

- 优先选择简化、正确性和低维护成本，而不是继续堆叠功能。
- 保持代码、README、GitHub Pages 与 `CHANGELOG.md` 的公开描述一致。
- 提交改动时保留与本任务无关的已有工作。

### 开发环境

#### 依赖要求

| 依赖 | 版本 |
|------|------|
| CMake | >= 3.18 |
| CUDA Toolkit | >= 11.0（CPU-only 开发可选） |
| C++ 编译器 | GCC 8+、Clang 7+ 或 MSVC 2019+ |
| Node.js | 文档站点使用 20.x |

#### 推荐验证基线

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

如需本地 compile commands 工作区：

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

### 代码风格

| 范围 | 规则 |
|------|------|
| 命名 | 类型用 PascalCase，函数和变量用 snake_case |
| 格式 | 4 空格缩进、K&R 大括号、100 列行宽 |
| 公共 API | 只有在确实能帮助理解时才补充 Doxygen 风格注释 |

优先做聚焦的小改动，显式表达约束，并让测试覆盖真实行为而不是 mock 本身。

### 文档要求

- 贡献者入口变化时更新 `README.md` / `README.zh-CN.md`
- 公共行为或示例变化时更新 GitHub Pages 内容
- 变更历史只保留在根目录 `CHANGELOG.md`

### Pull Request

发起 PR 前请确认：

1. 改动只解决一个清晰的问题。
2. 涉及代码时补充或更新测试。
3. 运行完整验证基线。
4. 行为或流程变化时同步更新文档。
5. 提交信息使用清晰的 Conventional Commit 风格主题。

#### PR 检查清单

- [ ] 构建与测试通过
- [ ] 格式检查与分析通过
- [ ] 需要时已更新文档
- [ ] 需要时已更新 `CHANGELOG.md`

### 报告 Bug

请尽量包含：

- 操作系统、编译器、CUDA 版本与构建模式
- 复现步骤
- 预期行为与实际行为
- 可最小复现的代码示例

### 功能建议

先说明问题，再说明建议方案。相比继续膨胀功能，更欢迎能简化代码、流程或文档面的改动。
