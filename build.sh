#!/bin/bash
set -euo pipefail

# Build script for Asthra compiler
# Usage: ./build.sh [--clean]

# Store the original directory and ensure we return to it
ORIGINAL_DIR=$(pwd)
SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

# Ensure we're in the project root directory
cd "$SCRIPT_DIR"

# Cleanup function to restore directory
cleanup() {
    cd "$ORIGINAL_DIR"
}
trap cleanup EXIT

CLEAN_BUILD=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--clean]"
            echo "  --clean    Perform a clean build (removes all build artifacts first)"
            echo "  --help     Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

echo "Building Asthra compiler..."

# Clean build artifacts if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Performing clean build - removing all build artifacts..."
    if ! cmake --build build --target clean 2>/dev/null; then
        echo "Warning: Clean target failed or build directory doesn't exist"
    fi
fi

# Configure build directory
if ! cmake -B build; then
    echo "Error: CMake configuration failed"
    exit 1
fi

# Build compiler
if ! cmake --build build; then
    echo "Error: Compiler build failed"
    exit 1
fi

# Build test executables
echo "Building test executables..."
if ! cmake --build build --target build-tests; then
    echo "Error: Test build failed"
    exit 1
fi

echo "Build completed successfully!"
