# Installation

This guide will help you install and set up HTS in your project.

## System Requirements

- **C++17** compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- **CMake** 3.18 or higher
- **CUDA Toolkit** 11.0+ (optional, for GPU support)
- **Linux** (recommended), **Windows**, or **macOS**

## Installation Methods

### Option 1: Build from Source (Recommended)

```bash
# Clone the repository
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build the library
make -j$(nproc)

# Optional: Install to system
sudo make install
```

### Option 2: CPU-Only Build

If you don't have CUDA or want to build without GPU support:

```bash
cmake .. -DCMAKE_BUILD_TYPE=Release -DHTS_CPU_ONLY=ON
make -j$(nproc)
```

### Option 3: Using as Dependency

#### With CMake (FetchContent)

```cmake
include(FetchContent)
FetchContent_Declare(
    hts
    GIT_REPOSITORY https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
    GIT_TAG        v1.2.0  # or your preferred version
)
FetchContent_MakeAvailable(hts)

# Link to your target
target_link_libraries(your_target PRIVATE hts_lib)
```

#### With CMake (find_package)

After installing HTS to your system:

```cmake
find_package(hts REQUIRED)
target_link_libraries(your_target PRIVATE hts_lib)
```

## Build Options

HTS provides several CMake options for customization:

| Option | Description | Default |
|--------|-------------|---------|
| `HTS_BUILD_TESTS` | Build unit and integration tests | `ON` |
| `HTS_BUILD_EXAMPLES` | Build example programs | `ON` |
| `HTS_CPU_ONLY` | Build without CUDA support | `OFF` |
| `HTS_ENABLE_COVERAGE` | Enable code coverage | `OFF` |
| `CMAKE_CUDA_ARCHITECTURES` | Target CUDA architecture | `70` |

### Example Configurations

**Full CUDA build with tests:**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DHTS_BUILD_TESTS=ON \
         -DCMAKE_CUDA_ARCHITECTURES=80
```

**Minimal CPU-only build:**
```bash
cmake .. -DCMAKE_BUILD_TYPE=Release \
         -DHTS_CPU_ONLY=ON \
         -DHTS_BUILD_TESTS=OFF \
         -DHTS_BUILD_EXAMPLES=OFF
```

## Verify Installation

After building, you can run the tests to verify everything works:

```bash
cd build
ctest --output-on-failure
```

You should see all tests passing:

```
100% tests passed, 0 tests failed out of 45
```

## Using the Scripts

HTS includes helpful automation scripts in the `scripts/` directory:

```bash
# Build with default options
scripts/build.sh

# CPU-only build
scripts/build.sh --cpu-only

# Run tests
scripts/test.sh

# Format code
scripts/format.sh

# Static analysis
scripts/analyze.sh
```

## Next Steps

- [Quick Start](/en/guide/quickstart) — Build your first DAG
- [Architecture](/en/guide/architecture) — Understand HTS internals
- [Examples](/en/examples/) — Browse example projects
