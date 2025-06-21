#!/bin/bash

# Asthra Coverage Test Runner
# Runs all tests with coverage instrumentation

set -e  # Exit on any error

echo "🔧 Asthra Code Coverage Test Suite"
echo "=================================="

# Clean previous coverage data
echo "🧹 Cleaning previous coverage data..."
find . -name "*.gcda" -delete
find . -name "*.gcno" -delete
rm -f coverage.info
rm -rf coverage_html

# Build runtime if needed
echo "🏗️  Building runtime..."
make runtime >/dev/null 2>&1

# Test compilation and execution function
run_coverage_test() {
    local test_name="$1"
    local test_files="$2"
    local stub_files="$3"
    
    echo "🧪 Running $test_name..."
    
    # Compile with coverage
    clang -O0 -g --coverage -I. -Iruntime -Isrc \
        -o "build/$test_name" $test_files $stub_files \
        build/libasthra_runtime.a -lpthread -lm
    
    # Run test
    "./build/$test_name"
    echo "✅ $test_name: PASSED"
    echo
}

# Run FFI Memory Tests
echo "📊 Running FFI Memory Tests..."
run_coverage_test "test_ffi_memory_allocation" \
    "tests/ffi/test_ffi_memory_allocation.c" \
    "tests/ffi/test_ffi_memory_common.c tests/ffi/test_ffi_stubs.c"

run_coverage_test "test_ffi_memory_slices" \
    "tests/ffi/test_ffi_memory_slices.c" \
    "tests/ffi/test_ffi_memory_common.c tests/ffi/test_ffi_stubs.c"

run_coverage_test "test_ffi_memory_strings" \
    "tests/ffi/test_ffi_memory_strings.c" \
    "tests/ffi/test_ffi_memory_common.c tests/ffi/test_ffi_stubs.c"

# Run Basic Runtime Tests
echo "📊 Running Basic Runtime Tests..."
run_coverage_test "test_basic" \
    "tests/basic/test_basic.c" \
    "tests/basic/basic_compiler_stubs.c"

# Generate coverage report
echo "📈 Generating coverage report..."
if command -v lcov >/dev/null 2>&1; then
    lcov --capture --directory . --output-file coverage.info >/dev/null 2>&1
    genhtml coverage.info --output-directory coverage_html >/dev/null 2>&1
    echo "📄 Coverage report generated in coverage_html/index.html"
else
    echo "⚠️  lcov not found - install it to generate HTML reports"
fi

# Count coverage files
coverage_files=$(find . -name "*.gcda" | wc -l)
echo "📁 Generated $coverage_files coverage data files"

echo
echo "🎯 All tests completed successfully!"
echo "   - FFI Memory Allocation: ✅"
echo "   - FFI Memory Slices: ✅" 
echo "   - FFI Memory Strings: ✅"
echo "   - Basic Runtime: ✅"
echo
echo "🚀 Ready for further coverage expansion!" 
