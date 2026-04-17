#!/bin/bash
# Test runner script for Heterogeneous Task Scheduler

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="build"

# Default values
VERBOSE=OFF
TEST_PATTERN=""
COVERAGE=OFF

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --verbose|-v)
            VERBOSE=ON
            shift
            ;;
        --pattern|-p)
            TEST_PATTERN="$2"
            shift 2
            ;;
        --coverage)
            COVERAGE=ON
            shift
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --verbose, -v     Enable verbose output"
            echo "  --pattern, -p P   Run tests matching pattern"
            echo "  --coverage        Generate coverage report"
            echo "  --build-dir DIR   Set build directory (default: build)"
            echo "  --help            Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=========================================="
echo "Running Tests"
echo "=========================================="

cd "$PROJECT_DIR/$BUILD_DIR"

if [ ! -f "CTestTestfile.cmake" ]; then
    echo "Error: Build directory not found. Run scripts/build.sh first."
    exit 1
fi

# Run tests
if [ -n "$TEST_PATTERN" ]; then
    echo "Running tests matching: $TEST_PATTERN"
    ctest --output-on-failure -R "$TEST_PATTERN" $( [ "$VERBOSE" = "ON" ] && echo "--verbose" )
else
    echo "Running all tests..."
    ctest --output-on-failure $( [ "$VERBOSE" = "ON" ] && echo "--verbose" )
fi

echo ""
echo "=========================================="
echo "All tests passed!"
echo "=========================================="

# Generate coverage report if enabled
if [ "$COVERAGE" = "ON" ]; then
    echo ""
    echo "Generating coverage report..."
    
    if command -v gcovr &> /dev/null; then
        gcovr -r .. --html-details coverage.html --exclude '.*tests/.*'
        echo "Coverage report generated: $PROJECT_DIR/$BUILD_DIR/coverage.html"
    else
        echo "Warning: gcovr not found. Install with: pip install gcovr"
    fi
fi
