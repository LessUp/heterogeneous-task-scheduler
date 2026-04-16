# Installation Guide

[![CMake](https://img.shields.io/badge/CMake-3.18+-blue)](https://cmake.org/)
[![CUDA](https://img.shields.io/badge/CUDA-11.0+-green)](https://developer.nvidia.com/cuda-toolkit)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.cppreference.com/w/cpp/17)

> Complete installation and configuration guide for HTS

---

## Table of Contents

- [System Requirements](#system-requirements)
- [Installing Dependencies](#installing-dependencies)
- [Building HTS](#building-hts)
- [CMake Options](#cmake-options)
- [Verification](#verification)
- [Troubleshooting](#troubleshooting)

---

## System Requirements

### Required Dependencies

| Dependency | Minimum Version | Recommended Version | Notes |
|------------|-----------------|---------------------|-------|
| CMake | 3.18 | 3.25+ | Build system |
| CUDA Toolkit | 11.0 | 12.0+ | GPU support |
| C++ Compiler | C++17 | C++20 | GCC 8+, Clang 7+, MSVC 2019+ |
| Git | 2.0 | Latest | Version control |

### Hardware Requirements

| Component | Minimum | Recommended |
|-----------|---------|-------------|
| CPU | x86_64 | Modern multi-core |
| GPU | Compute Capability 5.0+ | 7.0+ (Turing/Ampere) |
| RAM | 4GB | 16GB+ |
| GPU Memory | 2GB | 8GB+ |

### Optional Dependencies

| Dependency | Purpose | Auto-installed |
|------------|---------|----------------|
| Google Test | Unit testing | Yes |
| RapidCheck | Property testing | Yes |
| Doxygen | Documentation | No |

---

## Installing Dependencies

### Ubuntu/Debian

```bash
# Update package list
sudo apt-get update

# Install build essentials
sudo apt-get install -y build-essential cmake git

# Install CUDA Toolkit
# Visit: https://developer.nvidia.com/cuda-downloads
# Or use:
wget https://developer.download.nvidia.com/compute/cuda/repos/ubuntu2204/x86_64/cuda-keyring_1.1-1_all.deb
sudo dpkg -i cuda-keyring_1.1-1_all.deb
sudo apt-get update
sudo apt-get install -y cuda-toolkit

# Verify installation
nvcc --version
nvidia-smi
```

### CentOS/RHEL/Fedora

```bash
# CentOS/RHEL
sudo yum install -y gcc-c++ cmake git

# Fedora
sudo dnf install -y gcc-c++ cmake git

# Install CUDA
# Follow: https://developer.nvidia.com/cuda-downloads
```

### Windows

1. **Install Visual Studio 2019+**
   - Select "Desktop development with C++" workload

2. **Install CUDA Toolkit**
   - Download from [NVIDIA](https://developer.nvidia.com/cuda-downloads)
   - Run installer with default options

3. **Install CMake**
   - Download from [cmake.org](https://cmake.org/download/)
   - Add to PATH during installation

4. **Install Git**
   - Download from [git-scm.com](https://git-scm.com/download/win)

### macOS

> ⚠️ **Note**: macOS does not support NVIDIA GPUs natively. HTS will work in CPU-only mode.

```bash
# Install dependencies via Homebrew
brew install cmake git

# Install LLVM for C++17 support
brew install llvm
```

---

## Building HTS

### Standard Build (Linux/macOS)

```bash
# Clone the repository
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Create build directory
mkdir build && cd build

# Configure
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build with all available cores
make -j$(nproc)
```

### Standard Build (Windows)

```powershell
# Clone
git clone https://github.com/LessUp/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Create build directory
mkdir build
cd build

# Configure (Visual Studio 2022)
cmake .. -G "Visual Studio 17 2022" -A x64

# Build
cmake --build . --config Release --parallel
```

### Debug Build

```bash
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)
```

### Release Build with Optimizations

```bash
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_ARCHITECTURES="70;75;80;86;89;90"
make -j$(nproc)
```

---

## CMake Options

### Build Configuration

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Release` | Build type: Debug, Release, RelWithDebInfo, MinSizeRel |
| `CMAKE_CUDA_ARCHITECTURES` | `70;75;80;86;89;90` | Target CUDA architectures |
| `BUILD_TESTING` | `ON` | Build unit tests |
| `CMAKE_INSTALL_PREFIX` | `/usr/local` | Installation directory |

### Examples

```bash
# Debug build with tests
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON

# Release build for specific GPU (e.g., RTX 4090)
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_CUDA_ARCHITECTURES="89"

# Install to custom location
cmake .. \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_INSTALL_PREFIX=$HOME/.local
```

### CUDA Architecture Reference

| Architecture | Compute Capability | Examples |
|--------------|-------------------|----------|
| 50 | 5.0 | GTX 750 Ti |
| 60 | 6.0 | GTX 1060 |
| 70 | 7.0 | V100, Titan V |
| 75 | 7.5 | RTX 2080, T4 |
| 80 | 8.0 | A100 |
| 86 | 8.6 | RTX 3080/3090 |
| 89 | 8.9 | RTX 4090, L4 |
| 90 | 9.0 | H100 |

---

## Verification

### Run Tests

```bash
cd build

# Run all tests
ctest --output-on-failure

# Run with verbose output
ctest -V

# Run specific test
ctest -R test_scheduler
```

### Run Examples

```bash
# Basic examples
./simple_dag
./fluent_api
./parallel_pipeline

# Advanced examples
./advanced_features
./profiling
./scheduling_policies
```

### Check Installation

```cpp
#include <hts/heterogeneous_task_scheduler.hpp>
#include <iostream>

int main() {
    std::cout << "HTS Version Check\n";
    std::cout << "=================\n";
    
    // Check CUDA availability
    auto& cuda_utils = hts::CudaUtils::instance();
    std::cout << "CUDA Available: " << cuda_utils.is_available() << "\n";
    
    if (cuda_utils.is_available()) {
        auto info = cuda_utils.get_device_info(0);
        std::cout << "Device: " << info.name << "\n";
        std::cout << "Memory: " << info.total_memory / (1024*1024) << " MB\n";
    }
    
    return 0;
}
```

---

## Troubleshooting

### CMake cannot find CUDA

```bash
# Specify CUDA path explicitly
cmake .. -DCMAKE_CUDA_COMPILER=/usr/local/cuda/bin/nvcc
```

### Missing CUDA libraries

```bash
# Add to PATH and LD_LIBRARY_PATH
export PATH=/usr/local/cuda/bin:$PATH
export LD_LIBRARY_PATH=/usr/local/cuda/lib64:$LD_LIBRARY_PATH
```

### Build fails with "No CUDA GPUs" error

```bash
# CPU-only build (for development without GPU)
cmake .. -DHTS_ENABLE_CUDA=OFF
```

### Tests fail with CUDA errors

- Ensure NVIDIA drivers are installed: `nvidia-smi`
- Check CUDA version compatibility
- Verify GPU compute capability matches CMAKE_CUDA_ARCHITECTURES

---

## Next Steps

- [Quick Start Guide](quickstart.md) - Write your first HTS program
- [Architecture Overview](architecture.md) - Understand system design
- [API Reference](api-reference.md) - Explore the API
