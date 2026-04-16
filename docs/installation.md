# Installation Guide

[![CMake](https://img.shields.io/badge/CMake-3.18+-blue)](https://cmake.org/)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green)](https://developer.nvidia.com/cuda-toolkit)

> HTS 安装和配置指南

---

## 系统要求

### 必需依赖

| 依赖 | 版本 | 说明 |
|------|------|------|
| CMake | >= 3.18 | 构建系统 |
| CUDA Toolkit | >= 11.0 | GPU 支持 |
| C++ 编译器 | C++17 | GCC 8+, Clang 7+, MSVC 2019+ |

### 可选依赖

| 依赖 | 说明 |
|------|------|
| Google Test | 自动下载 |
| RapidCheck | 自动下载 |

---

## 安装步骤

### Linux

```bash
# 1. 安装依赖 (Ubuntu)
sudo apt-get update
sudo apt-get install -y cmake g++ git

# 2. 安装 CUDA Toolkit (如未安装)
# 访问 https://developer.nvidia.com/cuda-downloads

# 3. 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 4. 构建
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)

# 5. 运行测试
ctest --output-on-failure
```

### Windows

```powershell
# 1. 安装 Visual Studio 2019+ 和 CUDA Toolkit

# 2. 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 3. 构建
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# 4. 运行测试
ctest -C Release --output-on-failure
```

---

## CMake 选项

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CMAKE_BUILD_TYPE` | `Release` | 构建类型 |
| `CMAKE_CUDA_ARCHITECTURES` | `70;75;80;86;89;90` | CUDA 架构 |
| `BUILD_TESTING` | `ON` | 构建测试 |

### 示例配置

```bash
# Debug 构建
cmake .. -DCMAKE_BUILD_TYPE=Debug

# 指定 CUDA 架构
cmake .. -DCMAKE_CUDA_ARCHITECTURES="80;86"

# 完整配置
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_ARCHITECTURES="80;86" \
  -DBUILD_TESTING=ON
```

---

## 验证安装

```bash
# 运行示例
./build/simple_dag

# 运行全部测试
cd build && ctest --output-on-failure
```

---

## 下一步

- [快速入门](quickstart.md)
- [API 参考](api/scheduler.md)
