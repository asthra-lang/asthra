#!/bin/bash
# Script to run Linux/GCC GitHub Actions tests locally
# Based on .github/workflows/build-and-test.yml

set -e

echo "=== Running GitHub Actions Linux/GCC Tests Locally ==="
echo ""

# Parse command line options
BUILD_TYPE="Release"
CLEAN_BUILD=false
SKIP_BUILD=false
SPECIFIC_TESTS=""

while [[ $# -gt 0 ]]; do
    case $1 in
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --clean)
            CLEAN_BUILD=true
            shift
            ;;
        --skip-build)
            SKIP_BUILD=true
            shift
            ;;
        --test-category)
            SPECIFIC_TESTS="-L $2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [OPTIONS]"
            echo ""
            echo "Options:"
            echo "  --debug          Build in Debug mode (avoids LTO issues)"
            echo "  --clean          Clean build directory before building"
            echo "  --skip-build     Skip building and run tests only"
            echo "  --test-category  Run specific test category (e.g., lexer, parser)"
            echo "  --help           Show this help message"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            echo "Use --help for usage information"
            exit 1
            ;;
    esac
done

# Step 1: Check dependencies
echo "Step 1: Checking dependencies..."
echo "Required packages: build-essential, cmake, libc6-dev, pkg-config, gperf"
echo "Optional packages: valgrind, clang-tools, llvm, libjson-c-dev"

# Check if required tools are installed
if ! command -v gcc &> /dev/null; then
    echo "ERROR: GCC not found. Install with: sudo apt-get install build-essential"
    exit 1
fi

if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Install with: sudo apt-get install cmake"
    exit 1
fi

echo "✓ Basic dependencies found"
echo ""

# Step 2: Set up environment
echo "Step 2: Setting up build environment..."
export CC=gcc
export CXX=g++
echo "CC=$CC"
echo "CXX=$CXX"
echo "Build Type: $BUILD_TYPE"
echo ""

# Step 3: Configure CMake
if [ "$SKIP_BUILD" = false ]; then
    echo "Step 3: Configuring CMake..."
    
    if [ "$CLEAN_BUILD" = true ]; then
        echo "Cleaning build directory..."
        rm -rf build
    fi
    
    mkdir -p build
    cd build
    
    # Configure with appropriate flags to avoid LTO issues
    cmake .. \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DCMAKE_C_COMPILER=gcc \
        -DCMAKE_CXX_COMPILER=g++ \
        -DCMAKE_C_FLAGS="-Wno-error=maybe-uninitialized" \
        -DCMAKE_CXX_FLAGS="-Wno-error=maybe-uninitialized"
    
    echo "✓ CMake configuration complete"
    echo ""
else
    cd build 2>/dev/null || { echo "ERROR: Build directory not found. Run without --skip-build first."; exit 1; }
fi

# Initialize JOBS variable
JOBS=$(nproc 2>/dev/null || echo 2)

# Step 4: Build Asthra compiler
if [ "$SKIP_BUILD" = false ]; then
    echo "Step 4: Building Asthra compiler..."
    echo "Building with $JOBS parallel jobs"
    
    if ! cmake --build . --config "$BUILD_TYPE" -j$JOBS; then
        echo ""
        echo "ERROR: Build failed. Common solutions:"
        echo "- Try building with --debug flag to avoid LTO issues"
        echo "- Use --clean flag to start fresh"
        echo "- Check for missing dependencies"
        exit 1
    fi
    
    echo "✓ Asthra compiler built"
    echo ""
fi

# Step 5: Build test executables
if [ "$SKIP_BUILD" = false ]; then
    echo "Step 5: Building test executables..."
    
    if [ "$BUILD_TYPE" = "Release" ]; then
        echo "Note: Building in Release mode may cause LTO issues with some tests"
    fi
    
    if ! cmake --build . --target build-tests -j$JOBS; then
        echo ""
        echo "WARNING: Some tests failed to build"
        echo "This is often due to LTO issues in Release mode"
        echo "Try running with --debug flag for better compatibility"
        echo ""
        echo "Continuing with available tests..."
    fi
    
    echo ""
fi

# Step 6: Run tests
echo "Step 6: Running tests..."

# Verify build directory and test executables
if [ ! -d "bin" ]; then
    echo "ERROR: No bin directory found. Build may have failed."
    exit 1
fi

# Count available test executables
TEST_COUNT=$(find bin -name "*test*" -type f 2>/dev/null | wc -l)
echo "Found $TEST_COUNT test executables"

if [ $TEST_COUNT -eq 0 ]; then
    echo "ERROR: No test executables found. Build may have failed."
    echo "Try running with --clean --debug flags"
    exit 1
fi

# Make test executables executable
chmod +x bin/*test* 2>/dev/null || true

# Run tests with appropriate options
CTEST_ARGS="--output-on-failure"
if [ -n "$SPECIFIC_TESTS" ]; then
    CTEST_ARGS="$CTEST_ARGS $SPECIFIC_TESTS"
    echo "Running specific test category: $SPECIFIC_TESTS"
else
    CTEST_ARGS="$CTEST_ARGS --parallel $JOBS"
    echo "Running all tests with $JOBS parallel jobs"
fi

# Run tests and capture result
if ctest $CTEST_ARGS; then
    echo ""
    echo "✓ All tests passed!"
    TEST_STATUS="PASSED"
else
    TEST_RESULT=$?
    echo ""
    echo "⚠ Some tests failed (exit code: $TEST_RESULT)"
    echo ""
    
    # Show list of failed tests
    if [ -f "Testing/Temporary/LastTestsFailed.log" ]; then
        echo "Failed tests:"
        cat Testing/Temporary/LastTestsFailed.log | while read line; do
            echo "  - $line"
        done
        echo ""
    fi
    
    echo "To debug failures:"
    echo "  ctest --rerun-failed --output-on-failure  # Re-run failed tests"
    echo "  ctest -R <test_name> --verbose            # Run specific test with details"
    TEST_STATUS="FAILED"
fi

echo ""
echo "=== Test Summary ==="
echo "Build Type: $BUILD_TYPE"
echo "Test Status: $TEST_STATUS"
echo "Test Executables Found: $TEST_COUNT"
echo ""
echo "Quick test commands:"
echo "  $0 --skip-build                           # Run tests without rebuilding"
echo "  $0 --skip-build --test-category lexer     # Run only lexer tests"
echo "  $0 --skip-build --test-category parser    # Run only parser tests"
echo "  $0 --skip-build --test-category semantic  # Run only semantic tests"
echo "  $0 --skip-build --test-category codegen   # Run only codegen tests"
echo ""
echo "Advanced options:"
echo "  $0 --debug                                # Build in Debug mode (avoids LTO issues)"
echo "  $0 --clean --debug                        # Clean build in Debug mode"
echo ""
echo "Manual test commands (from build directory):"
echo "  ctest -L <category>                       # Run specific category"
echo "  ctest --verbose                           # Verbose output"
echo "  ctest -R <pattern>                        # Run tests matching pattern"
echo "  ctest --rerun-failed                      # Re-run only failed tests"
echo ""
if [ "$BUILD_TYPE" = "Release" ]; then
    echo "Note: You're using Release mode. If you encounter build errors,"
    echo "      try running with --debug flag to avoid LTO optimization issues."
fi