#!/bin/bash
# HTS Code Coverage Script
# Generates HTML coverage report using gcovr

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="${PROJECT_ROOT}/build-coverage"

echo "=== HTS Code Coverage Report Generator ==="
echo

# Check for gcovr
if ! command -v gcovr &> /dev/null; then
    echo "Error: gcovr is not installed."
    echo "Install with: pip install gcovr"
    exit 1
fi

# Create coverage build directory
echo "Creating coverage build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with coverage enabled
echo "Configuring CMake with coverage enabled..."
cmake .. \
    -DHTS_CPU_ONLY=ON \
    -DHTS_ENABLE_COVERAGE=ON \
    -DHTS_BUILD_TESTS=ON \
    -DHTS_BUILD_EXAMPLES=OFF \
    -G Ninja

# Build
echo "Building..."
cmake --build .

# Run tests
echo "Running tests..."
ctest --output-on-failure

# Generate coverage report
echo "Generating coverage report..."
REPORT_DIR="${PROJECT_ROOT}/coverage-report"
mkdir -p "$REPORT_DIR"

gcovr \
    --root "$PROJECT_ROOT" \
    --filter "$PROJECT_ROOT/src" \
    --filter "$PROJECT_ROOT/include" \
    --html-details \
    --output "${REPORT_DIR}/index.html" \
    --exclude-lines-by-pattern '.*HTS_CPU_ONLY.*' \
    --exclude-lines-by-pattern '.*__gcov_flush.*'

echo
echo "=== Coverage Report Generated ==="
echo "Report location: ${REPORT_DIR}/index.html"
echo "Open with: xdg-open ${REPORT_DIR}/index.html"
