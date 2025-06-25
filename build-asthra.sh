#!/usr/bin/env bash
set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[BUILD]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1" >&2
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

# Trap errors and clean up
trap 'print_error "Build failed on line $LINENO"' ERR

# Check if cmake is available
if ! command -v cmake &> /dev/null; then
    print_error "cmake is not installed or not in PATH"
    exit 1
fi

# Parse command line arguments
PARALLEL_JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)
CLEAN_BUILD=false
VERBOSE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            PARALLEL_JOBS="$2"
            shift 2
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --verbose|-v)
            VERBOSE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [OPTIONS]"
            echo "Options:"
            echo "  -j, --jobs <N>    Number of parallel jobs (default: auto-detect)"
            echo "  --clean           Force clean build"
            echo "  --verbose, -v     Enable verbose output"
            echo "  -h, --help        Show this help message"
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            exit 1
            ;;
    esac
done

# Set cmake verbosity
CMAKE_VERBOSE=""
if [ "$VERBOSE" = true ]; then
    CMAKE_VERBOSE="--verbose"
fi

# Configure build
print_status "Configuring build directory..."
if ! cmake -B build; then
    print_error "CMake configuration failed"
    exit 1
fi

# Clean if requested
if [ "$CLEAN_BUILD" = true ]; then
    print_status "Cleaning build artifacts..."
    if ! cmake --build build --target clean $CMAKE_VERBOSE; then
        print_warning "Clean target failed, continuing anyway"
    fi
fi

# Build asthra compiler
print_status "Building asthra compiler with $PARALLEL_JOBS parallel jobs..."
if ! cmake --build build --target asthra -j"$PARALLEL_JOBS" $CMAKE_VERBOSE; then
    print_error "Build failed"
    exit 1
fi

print_status "Build completed successfully!"

# Check if the binary was created
if [ -f "build/bin/asthra" ]; then
    print_status "Asthra compiler built at: $(pwd)/build/asthra"
else
    print_error "Expected output binary not found at build/asthra"
    exit 1
fi
