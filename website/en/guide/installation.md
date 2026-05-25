# Installation

The repository supports two practical build modes:

1. **CPU-only** for local development, CI, and most documentation work.
2. **CUDA-enabled** when `nvcc` is available and you need the real GPU path.

## Requirements

| Requirement | Notes |
|-------------|-------|
| CMake >= 3.18 | Required by the top-level `CMakeLists.txt` |
| C++17 compiler | GCC 8+, Clang 7+, or MSVC 2019+ is the documented baseline |
| CUDA Toolkit | Optional; required only for non-CPU-only builds |
| Node.js 20.x | Needed only for the VitePress docs site under `website/` |

## Recommended contributor setup

```bash
git clone https://github.com/AICL-Lab/heterogeneous-task-scheduler.git
cd heterogeneous-task-scheduler

scripts/build.sh --cpu-only
scripts/test.sh
```

This produces the library, examples, and tests in `build/` without requiring CUDA hardware.

## Full validation baseline

```bash
scripts/build.sh --cpu-only
scripts/test.sh
scripts/format.sh --check
scripts/analyze.sh
cd website && npm run docs:build
```

## Direct CMake workflow

If you want compile commands or tighter IDE integration, use the preset documented in the README:

```bash
cmake --preset cpu-only-debug
cmake --build --preset cpu-only-debug
ctest --preset cpu-only-debug
```

## CUDA build

When CUDA is installed and `nvcc` is on `PATH`, build without `--cpu-only`:

```bash
scripts/build.sh
```

The top-level `CMakeLists.txt` will fail fast if CUDA is missing and the build is not explicitly set to
CPU-only mode.

## Notes

- `scripts/build.sh` builds tests and examples by default.
- `scripts/test.sh` expects the chosen build directory to exist already.
- The docs site is intentionally independent from the C++ build; it only depends on Node packages under `website/`.
