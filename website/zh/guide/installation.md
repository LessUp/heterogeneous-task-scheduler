# 安装

仓库只保留两种实际有意义的构建模式：

1. **CPU-only**：本地开发、CI、文档构建的默认入口。
2. **CUDA-enabled**：机器上已有 `nvcc`，并且确实需要 GPU 执行路径时再启用。

## 依赖要求

| 依赖 | 说明 |
|------|------|
| CMake >= 3.18 | 顶层 `CMakeLists.txt` 的要求 |
| C++17 编译器 | 文档基线为 GCC 8+、Clang 7+、MSVC 2019+ |
| CUDA Toolkit | 仅在非 CPU-only 构建时需要 |
| Node.js 20.x | 仅用于 `website/` 下的 VitePress 文档站点 |

## 推荐开发环境

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
```

这样会在 `build/` 中生成库、示例与测试，而且不依赖 CUDA 硬件。

## 完整验证基线

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

## 直接使用 CMake preset

如果你需要 compile commands 或更明确的 IDE 工作区，使用 README 中的 preset：

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

## CUDA 构建

当 CUDA 已安装且 `nvcc` 在 `PATH` 中时，可以不带 `--cpu-only`：

```bash
scripts/build.sh
```

如果未显式选择 CPU-only 且系统找不到 CUDA，顶层 `CMakeLists.txt` 会直接报错，而不是静默降级。

## 说明

- `scripts/build.sh` 默认会构建测试和示例。
- `scripts/test.sh` 依赖目标构建目录已存在。
- 文档站点与 C++ 构建解耦；它只依赖 `website/` 下的 Node 包。
