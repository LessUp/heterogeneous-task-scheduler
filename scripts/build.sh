#!/bin/bash
# Build script for Heterogeneous Task Scheduler

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
ENABLE_TESTS=ON
CUDA_ARCH="70"
COVERAGE=OFF

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --no-tests)
            ENABLE_TESTS=OFF
            shift
            ;;
        --cuda-arch)
            CUDA_ARCH="$2"
            shift 2
            ;;
        --coverage)
            COVERAGE=ON
            shift
            ;;
        --clean)
            echo "Cleaning build directory..."
            rm -rf "$PROJECT_DIR/$BUILD_DIR"
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug          Build in Debug mode (default: Release)"
            echo "  --build-dir DIR  Set build directory (default: build)"
            echo "  --no-tests       Disable test building"
            echo "  --cuda-arch ARCH Set CUDA architecture (default: 70)"
            echo "  --coverage       Enable code coverage"
            echo "  --clean          Clean build directory before building"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Building Heterogeneous Task Scheduler"
echo "=========================================="
echo "Build Type: $BUILD_TYPE"
echo "Build Dir:  $BUILD_DIR"
echo "Tests:      $ENABLE_TESTS"
echo "CUDA Arch:  $CUDA_ARCH"
echo "Coverage:   $COVERAGE"
echo "=========================================="

cd "$PROJECT_DIR"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo ""
echo "Configuring CMake..."
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DHTS_BUILD_TESTS=$ENABLE_TESTS \
    -DHTS_ENABLE_COVERAGE=$COVERAGE \
    -DCMAKE_CUDA_ARCHITECTURES="$CUDA_ARCH"

# Build
echo ""
echo "Building..."
cmake --build . --parallel $(nproc)

echo ""
echo "=========================================="
echo "Build completed successfully!"
echo "=========================================="
echo "Build directory: $PROJECT_DIR/$BUILD_DIR"

if [ "$ENABLE_TESTS" = "ON" ]; then
    echo ""
    echo "To run tests:"
    echo "  cd $BUILD_DIR && ctest --output-on-failure"
fi
