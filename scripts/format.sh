#!/bin/bash
# Code formatting script for Heterogeneous Task Scheduler

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

# Default values
CHECK_ONLY=OFF
VERBOSE=OFF

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --check)
            CHECK_ONLY=ON
            shift
            ;;
        --verbose|-v)
            VERBOSE=ON
            shift
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --check         Check formatting without modifying files"
            echo "  --verbose, -v   Show detailed output"
            echo "  --help          Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=========================================="
if [ "$CHECK_ONLY" = "ON" ]; then
    echo "Checking Code Formatting"
else
    echo "Formatting Code"
fi
echo "=========================================="

cd "$PROJECT_DIR"

# Find clang-format
if command -v clang-format-17 &> /dev/null; then
    CLANG_FORMAT="clang-format-17"
elif command -v clang-format &> /dev/null; then
    CLANG_FORMAT="clang-format"
else
    echo "Error: clang-format not found"
    exit 1
fi

CLANG_FORMAT_VERSION=$($CLANG_FORMAT --version | grep -oP '\d+\.\d+' | head -1)
echo "Using: $CLANG_FORMAT (version $CLANG_FORMAT_VERSION)"
echo ""

# Find all C++ files
FILES=$(find src include tests -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.cu" -o -name "*.cuh" \))

if [ "$CHECK_ONLY" = "ON" ]; then
    # Check mode - report files that need formatting
    ERRORS=0
    for file in $FILES; do
        if ! $CLANG_FORMAT --dry-run --Werror -style=file "$file" 2>/dev/null; then
            echo "  ❌ $file"
            ERRORS=$((ERRORS + 1))
        elif [ "$VERBOSE" = "ON" ]; then
            echo "  ✓ $file"
        fi
    done
    
    if [ $ERRORS -gt 0 ]; then
        echo ""
        echo "Found $ERRORS file(s) that need formatting"
        echo "Run 'scripts/format.sh' to fix"
        exit 1
    else
        echo "✓ All files are properly formatted"
    fi
else
    # Format mode - fix files in place
    COUNT=0
    for file in $FILES; do
        if [ "$VERBOSE" = "ON" ]; then
            echo "  Formatting: $file"
        fi
        $CLANG_FORMAT -i -style=file "$file"
        COUNT=$((COUNT + 1))
    done
    
    echo "✓ Formatted $COUNT file(s)"
fi

echo ""
echo "=========================================="
echo "Done!"
echo "=========================================="
