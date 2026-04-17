#!/bin/bash
# Static analysis script for Heterogeneous Task Scheduler

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="build"

# Default values
FIX=OFF

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --fix)
            FIX=ON
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --fix      Auto-fix issues where possible"
            echo "  --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Running Static Analysis"
echo "=========================================="

cd "$PROJECT_DIR"

# Find clang-tidy
if command -v clang-tidy-17 &> /dev/null; then
    CLANG_TIDY="clang-tidy-17"
elif command -v clang-tidy &> /dev/null; then
    CLANG_TIDY="clang-tidy"
else
    echo "Warning: clang-tidy not found, skipping analysis"
    echo "Install with: sudo apt install clang-tidy"
    exit 0
fi

echo "Using: $CLANG_TIDY"
echo ""

# Check if compile_commands.json exists
if [ ! -f "$BUILD_DIR/compile_commands.json" ]; then
    echo "Error: compile_commands.json not found in $BUILD_DIR"
    echo "Please run 'scripts/build.sh' first"
    exit 1
fi

# Run clang-tidy
EXTRA_ARGS=""
if [ "$FIX" = "ON" ]; then
    EXTRA_ARGS="--fix"
fi

# Find all source files
FILES=$(find src -type f \( -name "*.cpp" -o -name "*.cu" \))

echo "Analyzing source files..."
ERRORS=0

for file in $FILES; do
    echo "  Analyzing: $file"
    if ! $CLANG_TIDY -p "$BUILD_DIR" $EXTRA_ARGS "$file" 2>/dev/null; then
        ERRORS=$((ERRORS + 1))
    fi
done

echo ""
if [ $ERRORS -eq 0 ]; then
    echo "=========================================="
    echo "✓ No issues found!"
    echo "=========================================="
else
    echo "=========================================="
    echo "Found $ERRORS file(s) with issues"
    if [ "$FIX" = "OFF" ]; then
        echo "Run 'scripts/analyze.sh --fix' to auto-fix issues"
    fi
    echo "=========================================="
    exit 1
fi
