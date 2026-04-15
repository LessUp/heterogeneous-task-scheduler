# Contributing to Heterogeneous Task Scheduler (HTS)

感谢你对 HTS 项目的关注！我们欢迎各种形式的贡献。

[English](#english) | [中文](#中文)

---

## English

### Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Development Setup](#development-setup)
- [Code Style](#code-style)
- [Commit Convention](#commit-convention)
- [Pull Request Process](#pull-request-process)
- [Reporting Bugs](#reporting-bugs)
- [Feature Requests](#feature-requests)

### Code of Conduct

This project follows the [Contributor Covenant Code of Conduct](CODE_OF_CONDUCT.md). By participating, you agree to uphold this code.

### Development Setup

#### Requirements

| Requirement | Version |
|-------------|---------|
| CMake | >= 3.18 |
| CUDA Toolkit | >= 11.0 |
| C++ Compiler | GCC 8+, Clang 7+, or MSVC 2019+ |
| Git | Any recent version |

#### Build Steps

```bash
# Clone the repository
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Create build directory
mkdir build && cd build

# Configure (Debug mode for development)
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build . -j$(nproc)

# Run tests
ctest --output-on-failure
```

#### Run Examples

```bash
./build/simple_dag
./build/parallel_pipeline
./build/fluent_api
./build/advanced_features
```

### Code Style

#### C++ Naming Conventions

| Type | Convention | Example |
|------|------------|---------|
| Classes | PascalCase | `TaskGraph`, `MemoryPool` |
| Functions | snake_case | `add_task()`, `get_stats()` |
| Variables | snake_case | `task_id`, `memory_pool` |
| Constants | UPPER_SNAKE_CASE | `MAX_THREADS`, `MIN_BLOCK_SIZE` |
| Private members | trailing underscore | `id_`, `state_` |
| Namespaces | lowercase | `hts` |

#### Formatting

- **Indentation**: 4 spaces (no tabs)
- **Line width**: 100 characters
- **Braces**: K&R style (opening brace on same line)

#### Header File Guidelines

```cpp
#pragma once

// 1. Project headers
#include "hts/types.hpp"

// 2. C standard library
#include <cstdint>

// 3. C++ standard library
#include <memory>
#include <string>
#include <vector>

// 4. Third-party libraries
// (rarely needed in headers)

namespace hts {

/// @brief Brief description of the class
/// @details Detailed description if needed
class ExampleClass {
public:
    /// @brief Brief description of the method
    /// @param param_name Description of parameter
    /// @return Description of return value
    /// @throws ExceptionType When this exception is thrown
    void method(int param_name);

private:
    int private_member_;
};

} // namespace hts
```

#### Documentation Comments

Use Doxygen-style comments for all public APIs:

```cpp
/// @brief Add a new task to the graph
/// @param device Preferred execution device (CPU, GPU, or Any)
/// @return Shared pointer to the created task
/// @note The task is not scheduled until execute() is called
TaskPtr add_task(DeviceType device = DeviceType::Any);
```

### Commit Convention

We follow [Conventional Commits](https://www.conventionalcommits.org/):

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

#### Types

| Type | Description |
|------|-------------|
| `feat` | New feature |
| `fix` | Bug fix |
| `docs` | Documentation changes |
| `style` | Code style changes (formatting, no logic change) |
| `refactor` | Code refactoring |
| `perf` | Performance improvements |
| `test` | Adding or modifying tests |
| `chore` | Build process or tooling changes |

#### Examples

```
feat(scheduler): add support for task priorities

Add TaskPriority enum and priority-based scheduling.
Tasks with higher priority are executed first when
multiple tasks are ready.

Closes #123
```

```
fix(memory-pool): fix memory leak in block coalescing

The coalesce_blocks() function was not properly merging
adjacent free blocks, causing memory fragmentation.
```

### Pull Request Process

1. **Fork** the repository and create your branch:
   ```bash
   git checkout -b feature/your-feature-name
   # or
   git checkout -b fix/your-bug-fix
   ```

2. **Make changes** and ensure:
   - Code follows style guidelines
   - All tests pass
   - New features have tests
   - Documentation is updated

3. **Commit** with conventional commit messages:
   ```bash
   git add .
   git commit -m "feat(component): description"
   ```

4. **Push** to your fork:
   ```bash
   git push origin feature/your-feature-name
   ```

5. **Create Pull Request** on GitHub

#### PR Checklist

- [ ] Code follows project style guidelines
- [ ] All tests pass (`ctest --output-on-failure`)
- [ ] New features have corresponding tests
- [ ] Documentation updated (if applicable)
- [ ] CHANGELOG.md updated (if applicable)
- [ ] Commit messages follow Conventional Commits

### Reporting Bugs

Before reporting, please:

1. Search existing issues
2. Verify the issue is reproducible

Include in your report:

- **Environment**: OS, compiler, CUDA version
- **Steps to reproduce**: Detailed instructions
- **Expected behavior**: What should happen
- **Actual behavior**: What actually happens
- **Minimal code example**: If possible

### Feature Requests

We welcome feature suggestions! Please:

1. Search existing issues first
2. Describe the use case and problem to solve
3. Propose a solution if you have one in mind

---

## 中文

### 目录

- [行为准则](#行为准则)
- [开发环境设置](#开发环境设置)
- [代码风格](#代码风格)
- [提交规范](#提交规范)
- [Pull Request 流程](#pull-request-流程)
- [报告 Bug](#报告-bug)
- [功能建议](#功能建议)

### 行为准则

参与本项目即表示你同意遵守我们的 [行为准则](CODE_OF_CONDUCT.md)。

### 开发环境设置

#### 依赖要求

| 依赖 | 版本 |
|------|------|
| CMake | >= 3.18 |
| CUDA Toolkit | >= 11.0 |
| C++ 编译器 | GCC 8+, Clang 7+, 或 MSVC 2019+ |
| Git | 任意较新版本 |

#### 构建步骤

```bash
# 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 创建构建目录
mkdir build && cd build

# 配置（Debug 模式用于开发）
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 构建
cmake --build . -j$(nproc)

# 运行测试
ctest --output-on-failure
```

### 代码风格

#### 命名约定

| 类型 | 约定 | 示例 |
|------|------|------|
| 类名 | PascalCase | `TaskGraph`, `MemoryPool` |
| 函数名 | snake_case | `add_task()`, `get_stats()` |
| 变量名 | snake_case | `task_id`, `memory_pool` |
| 常量 | UPPER_SNAKE_CASE | `MAX_THREADS` |
| 私有成员 | 后缀下划线 | `id_`, `state_` |

#### 格式化

- **缩进**：4 个空格
- **行宽**：100 字符
- **大括号**：K&R 风格

#### 文档注释

使用 Doxygen 风格注释：

```cpp
/// @brief 向图中添加新任务
/// @param device 首选执行设备
/// @return 创建的任务的共享指针
TaskPtr add_task(DeviceType device = DeviceType::Any);
```

### 提交规范

遵循 [Conventional Commits](https://www.conventionalcommits.org/)：

```
<类型>(<范围>): <描述>
```

#### 类型

| 类型 | 描述 |
|------|------|
| `feat` | 新功能 |
| `fix` | Bug 修复 |
| `docs` | 文档更新 |
| `style` | 代码格式 |
| `refactor` | 重构 |
| `perf` | 性能优化 |
| `test` | 测试相关 |
| `chore` | 构建/工具 |

### Pull Request 流程

1. **Fork** 仓库并创建分支
2. **编写代码**，确保测试通过
3. **提交** 遵循提交规范
4. **推送** 到你的 Fork
5. **创建 PR** 并填写模板

#### PR 检查清单

- [ ] 代码遵循项目风格
- [ ] 所有测试通过
- [ ] 新功能有对应测试
- [ ] 文档已更新

### 报告 Bug

请先搜索现有 Issue，报告时包含：

- 环境信息（OS、编译器、CUDA 版本）
- 复现步骤
- 预期行为 vs 实际行为
- 最小代码示例

### 功能建议

欢迎功能建议！请描述：

- 问题背景
- 建议方案
- 使用场景

---

## Getting Help

- 📖 [README](README.md) and examples
- 🐛 [GitHub Issues](https://github.com/LessUp/heterogeneous-task-scheduler/issues)
- 📚 [API Documentation](https://lessup.github.io/heterogeneous-task-scheduler/)

感谢你的贡献！🎉
