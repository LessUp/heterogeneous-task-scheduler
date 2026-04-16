# 安装指南

[![CMake](https://img.shields.io/badge/CMake-3.18+-blue)](https://cmake.org/)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green)](https://developer.nvidia.com/cuda-toolkit)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

> HTS 完整安装和配置指南

---

## 目录

- [系统要求](#系统要求)
- [安装依赖](#安装依赖)
- [构建 HTS](#构建-hts)
- [CMake 选项](#cmake-选项)
- [验证安装](#验证安装)
- [故障排除](#故障排除)

---

## 系统要求

### 必需依赖

| 依赖项 | 最低版本 | 推荐版本 | 说明 |
|--------|---------|---------|------|
| CMake | 3.18 | 3.25+ | 构建系统 |
| CUDA Toolkit | 11.0 | 12.0+ | GPU 支持 |
| C++ 编译器 | C++17 | C++20 | GCC 8+, Clang 7+, MSVC 2019+ |
| Git | 2.0 | 最新 | 版本控制 |

### 硬件要求

| 组件 | 最低配置 | 推荐配置 |
|------|---------|---------|
| CPU | x86_64 | 现代多核处理器 |
| GPU | 计算能力 5.0+ | 7.0+ (Turing/Ampere) |
| 内存 | 4GB | 16GB+ |
| GPU 显存 | 2GB | 8GB+ |

### 可选依赖

| 依赖项 | 用途 | 自动安装 |
|--------|------|---------|
| Google Test | 单元测试 | 是 |
| RapidCheck | 属性测试 | 是 |
| Doxygen | 文档生成 | 否 |

---

## 安装依赖

### Ubuntu/Debian

```bash
# 更新软件包列表
sudo apt-get update

# 安装构建工具
sudo apt-get install -y build-essential cmake git

# 安装 CUDA Toolkit
# 访问: https://developer.nvidia.com/cuda-downloads
# 或使用:
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install -y cuda-toolkit

# 验证安装
nvcc --version
nvidia-smi
```

### CentOS/RHEL/Fedora

```bash
# CentOS/RHEL
sudo yum install -y gcc-c++ cmake git

# Fedora
sudo dnf install -y gcc-c++ cmake git

# 安装 CUDA
# 参照: https://developer.nvidia.com/cuda-downloads
```

### Windows

1. **安装 Visual Studio 2019+**
   - 选择 "使用 C++ 的桌面开发" 工作负载

2. **安装 CUDA Toolkit**
   - 从 [NVIDIA](https://developer.nvidia.com/cuda-downloads) 下载
   - 运行安装程序，使用默认选项

3. **安装 CMake**
   - 从 [cmake.org](https://cmake.org/download/) 下载
   - 安装时添加到 PATH

4. **安装 Git**
   - 从 [git-scm.com](https://git-scm.com/download/win) 下载

### macOS

> ⚠️ **注意**: macOS 不支持 NVIDIA GPU。HTS 将以仅 CPU 模式运行。

```bash
# 通过 Homebrew 安装依赖
brew install cmake git

# 安装 LLVM 以支持 C++17
brew install llvm
```

---

## 构建 HTS

### 标准构建（Linux/macOS）

```bash
# 克隆仓库
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 创建构建目录
mkdir build && cd build

# 配置
cmake .. -DCMAKE_BUILD_TYPE=Release

# 使用所有可用核心构建
make -j$(nproc)
```

### 标准构建（Windows）

```powershell
# 克隆
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# 创建构建目录
mkdir build
cd build

# 配置（Visual Studio 2022）
cmake .. -G "Visual Studio 17 2022" -A x64

# 构建
cmake --build . --config Release --parallel
```

### 调试构建

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### 发布构建（带优化）

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_ARCHITECTURES="70;75;80;86;89;90"
make -j$(nproc)
```

---

## CMake 选项

### 构建配置

| 选项 | 默认值 | 说明 |
|------|--------|------|
| `CMAKE_BUILD_TYPE` | `Release` | 构建类型: Debug, Release, RelWithDebInfo, MinSizeRel |
| `CMAKE_CUDA_ARCHITECTURES` | `70;75;80;86;89;90` | 目标 CUDA 架构 |
| `BUILD_TESTING` | `ON` | 构建单元测试 |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | 安装目录 |

### 配置示例

```bash
# 带测试的调试构建
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# 针对特定 GPU 的发布构建（如 RTX 4090）
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_ARCHITECTURES="89"

# 完整配置
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_ARCHITECTURES="80;86" \
  -DBUILD_TESTING=ON
```

### CUDA 架构参考

| 架构 | 计算能力 | 示例显卡 |
|------|---------|---------|
| 50 | 5.0 | GTX 750 Ti |
| 60 | 6.0 | GTX 1060 |
| 70 | 7.0 | V100, Titan V |
| 75 | 7.5 | RTX 2080, T4 |
| 80 | 8.0 | A100 |
| 86 | 8.6 | RTX 3080/3090 |
| 89 | 8.9 | RTX 4090, L4 |
| 90 | 9.0 | H100 |

---

## 验证安装

### 运行测试

```bash
cd build

# 运行所有测试
ctest --output-on-failure

# 详细输出
ctest -V

# 运行特定测试
ctest -R test_scheduler
```

### 运行示例

```bash
# 基础示例
./simple_dag
./fluent_api
./parallel_pipeline

# 高级示例
./advanced_features
./profiling
./scheduling_policies
```

### 检查安装

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    std::cout << "HTS 版本检查\n";
    std::cout << "============\n";
    
    // 检查 CUDA 可用性
    auto& cuda_utils = hts::CudaUtils::instance();
    std::cout << "CUDA 可用: " << cuda_utils.is_available() << "\n";
    
    if (cuda_utils.is_available()) {
        auto info = cuda_utils.get_device_info(0);
        std::cout << "设备: " << info.name << "\n";
        std::cout << "显存: " << info.total_memory / (1024*1024) << " MB\n";
    }
    
    return 0;
}
```

---

## 故障排除

### CMake 找不到 CUDA

```bash
# 显式指定 CUDA 路径
cmake .. -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc
```

### 缺少 CUDA 库

```bash
# 添加到 PATH 和 LD_LIBRARY_PATH
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
```

### 构建失败，提示 "No CUDA GPUs" 错误

```bash
# 仅 CPU 构建（无 GPU 开发环境）
cmake .. -DHTS_ENABLE_CUDA=OFF
```

### 测试出现 CUDA 错误

- 确保已安装 NVIDIA 驱动: `nvidia-smi`
- 检查 CUDA 版本兼容性
- 验证 GPU 计算能力与 CMAKE_CUDA_ARCHITECTURES 匹配

---

## 下一步

- [快速入门](quickstart.md) - 编写您的第一个 HTS 程序
- [架构概览](architecture.md) - 理解系统设计
- [API 参考](api-reference.md) - 探索 API
