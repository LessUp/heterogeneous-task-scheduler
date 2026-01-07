# Contributing to Heterogeneous Task Scheduler (HTS)

感谢你对 HTS 项目的关注！我们欢迎各种形式的贡献，包括 bug 报告、功能建议、文档改进和代码贡献。

## 目录

- [行为准则](#行为准则)
- [开发环境设置](#开发环境设置)
- [代码风格](#代码风格)
- [提交规范](#提交规范)
- [Pull Request 流程](#pull-request-流程)
- [报告 Bug](#报告-bug)
- [功能建议](#功能建议)

## 行为准则

参与本项目即表示你同意遵守我们的 [行为准则](CODE_OF_CONDUCT.md)。请确保你的行为符合社区标准。

## 开发环境设置

### 依赖要求

- CMake >= 3.18
- CUDA Toolkit >= 11.0
- C++17 兼容编译器 (GCC >= 8, Clang >= 7, MSVC >= 2019)
- Git

### 构建步骤

```bash
# 克隆仓库
git clone https://github.com/your-username/HeterogeneousTaskScheduler.git
cd HeterogeneousTaskScheduler

# 创建构建目录
mkdir build && cd build

# 配置项目
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 构建
make -j$(nproc)

# 运行测试
ctest --output-on-failure
```

### 运行示例

```bash
./simple_dag
./parallel_pipeline
./fluent_api
```

## 代码风格

### C++ 代码规范

我们遵循以下代码风格指南：

1. **命名约定**
   - 类名：`PascalCase` (例如 `TaskGraph`, `MemoryPool`)
   - 函数名：`snake_case` (例如 `add_task`, `get_stats`)
   - 变量名：`snake_case` (例如 `task_id`, `memory_pool`)
   - 常量：`UPPER_SNAKE_CASE` (例如 `MAX_THREADS`)
   - 私有成员：后缀下划线 (例如 `id_`, `state_`)

2. **格式化**
   - 缩进：4 个空格
   - 行宽：100 字符
   - 大括号：K&R 风格

3. **头文件**
   - 使用 `#pragma once` 作为头文件保护
   - 按以下顺序包含头文件：
     1. 对应的头文件
     2. C 标准库
     3. C++ 标准库
     4. 第三方库
     5. 项目头文件

4. **文档注释**
   - 使用 Doxygen 风格的注释
   - 所有公共 API 必须有文档注释

```cpp
/// @brief 简短描述
/// @param param_name 参数描述
/// @return 返回值描述
/// @throws 异常描述
```

### 示例代码

```cpp
#pragma once

#include "hts/types.hpp"
#include <memory>
#include <string>

namespace hts {

/// @brief 任务类，表示可在 CPU 或 GPU 上执行的计算单元
class Task {
public:
    /// @brief 构造函数
    /// @param id 任务唯一标识符
    /// @param device 首选执行设备
    explicit Task(TaskId id, DeviceType device = DeviceType::Any);
    
    /// @brief 获取任务 ID
    /// @return 任务唯一标识符
    TaskId id() const { return id_; }

private:
    TaskId id_;
    DeviceType device_;
};

} // namespace hts
```

## 提交规范

我们使用 [Conventional Commits](https://www.conventionalcommits.org/) 规范：

```
<type>(<scope>): <description>

[optional body]

[optional footer(s)]
```

### 类型 (type)

- `feat`: 新功能
- `fix`: Bug 修复
- `docs`: 文档更新
- `style`: 代码格式（不影响代码运行的变动）
- `refactor`: 重构（既不是新功能也不是修复 bug）
- `perf`: 性能优化
- `test`: 测试相关
- `chore`: 构建过程或辅助工具的变动

### 示例

```
feat(scheduler): add support for task priorities

Add TaskPriority enum and priority-based scheduling.
Tasks with higher priority are executed first.

Closes #123
```

```
fix(memory-pool): fix memory leak in block coalescing

The coalesce_blocks() function was not properly merging
adjacent free blocks, causing memory fragmentation.
```

## Pull Request 流程

1. **Fork 仓库** 并克隆到本地

2. **创建分支**
   ```bash
   git checkout -b feature/your-feature-name
   # 或
   git checkout -b fix/your-bug-fix
   ```

3. **进行修改**
   - 编写代码
   - 添加测试
   - 更新文档

4. **确保测试通过**
   ```bash
   cd build
   ctest --output-on-failure
   ```

5. **提交更改**
   ```bash
   git add .
   git commit -m "feat(component): description"
   ```

6. **推送到 Fork**
   ```bash
   git push origin feature/your-feature-name
   ```

7. **创建 Pull Request**
   - 填写 PR 模板
   - 关联相关 Issue
   - 等待 CI 检查通过
   - 等待代码审查

### PR 检查清单

- [ ] 代码遵循项目代码风格
- [ ] 所有测试通过
- [ ] 新功能有对应的测试
- [ ] 文档已更新（如适用）
- [ ] CHANGELOG.md 已更新（如适用）
- [ ] 提交信息遵循 Conventional Commits 规范

## 报告 Bug

在报告 Bug 之前，请：

1. 搜索现有 Issue，确认问题未被报告
2. 确认问题可以复现

报告 Bug 时，请提供：

- **环境信息**：操作系统、编译器版本、CUDA 版本
- **复现步骤**：详细的步骤说明
- **预期行为**：你期望发生什么
- **实际行为**：实际发生了什么
- **错误日志**：相关的错误信息或日志

## 功能建议

我们欢迎功能建议！请在提交之前：

1. 搜索现有 Issue，确认功能未被建议
2. 考虑该功能是否符合项目目标

提交功能建议时，请描述：

- **问题背景**：你想解决什么问题？
- **建议方案**：你建议如何解决？
- **替代方案**：你考虑过哪些替代方案？
- **附加信息**：任何有助于理解建议的信息

## 获取帮助

如果你有任何问题，可以：

- 查看 [README](README.md) 和示例代码
- 搜索或创建 [Issue](https://github.com/your-username/HeterogeneousTaskScheduler/issues)
- 查看 [API 文档](docs/api/)

感谢你的贡献！🎉
