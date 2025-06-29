#!/bin/bash
set -e

echo "=== Building Podman container for BDD tests ==="
podman build -f Dockerfile.bdd-test -t asthra-bdd-test .

echo ""
echo "=== Running BDD tests in container ==="
podman run --rm -t \
    -v "$(pwd):/workspace:ro" \
    asthra-bdd-test \
    bash -c '
set -e

echo "=== Container environment ==="
echo "OS: $(lsb_release -d | cut -f2)"
echo "Clang version: $(clang-18 --version | head -1)"
echo "LLVM version: $(llc-18 --version | head -2 | tail -1)"
echo "User: $(whoami)"
echo "Working directory: $(pwd)"
echo ""

echo "=== Copying source to writable location ==="
cd /home/runner
cp -r /workspace asthra-test
cd asthra-test

echo "=== Creating build directory ==="
mkdir -p build
cd build

echo ""
echo "=== Configuring CMake (verbose) ==="
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_C_COMPILER=clang-18 \
    -DCMAKE_CXX_COMPILER=clang++-18 \
    -DBUILD_BDD_TESTS=ON \
    -DUSE_SYSTEM_JSON_C=OFF \
    -DCMAKE_VERBOSE_MAKEFILE=ON 2>&1 | tee cmake-configure.log

echo ""
echo "=== Building Asthra with BDD tests (verbose) ==="
cmake --build . --parallel $(nproc) 2>&1 | tee cmake-build.log || {
    echo "Build failed! Last 50 lines of output:"
    tail -50 cmake-build.log
    exit 1
}

echo ""
echo "=== Building BDD test executables ==="
cmake --build . --target build-tests 2>&1 | tee cmake-build-tests.log

echo ""
echo "=== Running BDD unit tests ==="
mkdir -p bdd-logs

# Find the asthra compiler binary
ASTHRA_COMPILER_PATH="$(find . -name "asthra" -type f -executable | grep -E "(bin/asthra|asthra)$" | head -1)"
if [ -n "$ASTHRA_COMPILER_PATH" ] && [ -f "$ASTHRA_COMPILER_PATH" ]; then
    echo "✓ Asthra compiler found at: $ASTHRA_COMPILER_PATH"
    ls -la "$ASTHRA_COMPILER_PATH"
    export ASTHRA_COMPILER_PATH
else
    echo "❌ Asthra compiler not found"
    exit 1
fi

# Run specific BDD test to see detailed output
if [ -f "bdd/bin/bdd_unit_compiler_basic" ]; then
    echo ""
    echo "=== Running compiler basic unit tests with detailed output ==="
    
    # First, let'\''s see what the compiler outputs when we try to compile a simple test
    echo "Creating test file..."
    mkdir -p bdd-temp
    cat > bdd-temp/test_simple.asthra << '\''EOF'\''
package main;

pub fn main(none) -> void {
    log("Hello from test");
    return ();
}
EOF
    
    echo "Attempting to compile test file with verbose output..."
    ./bin/asthra -v -o bdd-temp/test_simple bdd-temp/test_simple.asthra 2>&1 || true
    
    echo ""
    echo "Now running BDD test..."
    ./bdd/bin/bdd_unit_compiler_basic --reporter spec 2>&1 | tee bdd-logs/compiler_basic_detailed.log || true
fi

# Summary of all BDD tests
echo ""
echo "=== Running all BDD tests ==="
TESTS_RUN=0
TESTS_FAILED=0

for test in bdd/bin/bdd_unit_*; do
    if [ -x "$test" ]; then
        TEST_NAME=$(basename "$test")
        echo -n "Running $TEST_NAME... "
        TESTS_RUN=$((TESTS_RUN + 1))
        if $test --reporter spec > "bdd-logs/${TEST_NAME}.log" 2>&1; then
            echo "✓ PASSED"
        else
            echo "✗ FAILED"
            TESTS_FAILED=$((TESTS_FAILED + 1))
            echo "  See bdd-logs/${TEST_NAME}.log for details"
            # Show first few lines of failure
            echo "  First 10 lines of failure:"
            head -10 "bdd-logs/${TEST_NAME}.log" | sed '\''s/^/    /'\''
        fi
    fi
done

echo ""
echo "BDD Test Summary: $TESTS_RUN tests run, $TESTS_FAILED failed"

if [ $TESTS_FAILED -gt 0 ]; then
    echo "❌ Some BDD tests failed"
    exit 1
else
    echo "✅ All BDD tests passed"
fi
'