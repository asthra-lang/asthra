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

# Test compiler functionality before running BDD tests
echo ""
echo "=== Testing Asthra compiler functionality ==="
mkdir -p bdd-temp
cat > bdd-temp/test_simple.asthra << '\''EOF'\''
package main;

pub fn main(none) -> void {
    log("Hello from test");
    return ();
}
EOF

echo "Attempting to compile test file..."
if "$ASTHRA_COMPILER_PATH" -v -o bdd-temp/test_simple bdd-temp/test_simple.asthra 2>&1; then
    echo "✓ Compiler test successful"
    if [ -f "bdd-temp/test_simple" ]; then
        echo "✓ Executable created"
    fi
else
    echo "❌ Compiler test failed - checking dependencies..."
    echo "Checking compiler dependencies..."
    ldd "$ASTHRA_COMPILER_PATH" 2>&1 || echo "ldd not available"
fi

# Summary of all BDD tests
echo ""
echo "=== Running all BDD tests ==="
SUITES_RUN=0
SUITES_FAILED=0
TOTAL_TESTS_PASSED=0
TOTAL_TESTS_FAILED=0
TOTAL_TESTS_SKIPPED=0

for test in bdd/bin/bdd_unit_*; do
    if [ -x "$test" ]; then
        TEST_NAME=$(basename "$test")
        echo ""
        echo "=== Running $TEST_NAME ==="
        SUITES_RUN=$((SUITES_RUN + 1))
        # Use tee to show output while also saving to log
        $test --reporter spec 2>&1 | tee "bdd-logs/${TEST_NAME}.log"
        # Check exit status of the test command (not tee)
        if [ ${PIPESTATUS[0]} -eq 0 ]; then
            echo "✓ $TEST_NAME PASSED"
        else
            echo "✗ $TEST_NAME FAILED"
            SUITES_FAILED=$((SUITES_FAILED + 1))
        fi
        
        # Extract test counts from the summary line
        # Looking for pattern: "Passed: X", "Failed: Y", "Skipped: Z"
        if [ -f "bdd-logs/${TEST_NAME}.log" ]; then
            SUMMARY_LINE=$(grep -E "^\s*Passed:|Test Summary" "bdd-logs/${TEST_NAME}.log" | tail -1)
            if [[ "$SUMMARY_LINE" =~ Passed:[[:space:]]*([0-9]+) ]]; then
                TOTAL_TESTS_PASSED=$((TOTAL_TESTS_PASSED + ${BASH_REMATCH[1]}))
            fi
            if [[ "$SUMMARY_LINE" =~ Failed:[[:space:]]*([0-9]+) ]]; then
                TOTAL_TESTS_FAILED=$((TOTAL_TESTS_FAILED + ${BASH_REMATCH[1]}))
            fi
            if [[ "$SUMMARY_LINE" =~ Skipped:[[:space:]]*([0-9]+) ]]; then
                TOTAL_TESTS_SKIPPED=$((TOTAL_TESTS_SKIPPED + ${BASH_REMATCH[1]}))
            fi
        fi
    fi
done

echo ""
echo "========================================="
echo "BDD Test Suite Summary:"
echo "  Test Suites: $SUITES_RUN run, $SUITES_FAILED failed"
echo ""
echo "Aggregate Test Results:"
echo "  Passed:  $TOTAL_TESTS_PASSED"
echo "  Failed:  $TOTAL_TESTS_FAILED"
echo "  Skipped: $TOTAL_TESTS_SKIPPED"
echo "  Total:   $((TOTAL_TESTS_PASSED + TOTAL_TESTS_FAILED + TOTAL_TESTS_SKIPPED))"
echo "========================================="

if [ $SUITES_FAILED -gt 0 ]; then
    echo "❌ Some BDD test suites failed"
    exit 1
else
    echo "✅ All BDD test suites passed"
fi
'