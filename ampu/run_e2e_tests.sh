#!/bin/bash

# End-to-End Test Runner for Ampu CLI and Asthra Language Features
# This script runs comprehensive tests covering both CLI functionality and language features

set -e  # Exit on any error

echo "🚀 Ampu & Asthra Test Runner"
echo "============================"

# Build the project first
echo "📦 Building Ampu..."
cargo build

echo ""
echo "🧪 Running Comprehensive Test Suite"
echo "==================================="

# Function to run tests with a description
run_test() {
    local description="$1"
    local command="$2"
    
    echo ""
    echo "🔍 $description"
    echo "Command: $command"
    echo "---"
    
    if eval "$command"; then
        echo "✅ PASSED"
    else
        echo "❌ FAILED"
        return 1
    fi
}

# =============================================================================
# CLI FUNCTIONALITY TESTS
# =============================================================================

echo ""
echo "🟢 Running CLI Functionality Tests"
echo "=================================="

run_test "Version command" \
    "cargo test test_version_command --test e2e_tests -- --exact --nocapture"

run_test "Help command" \
    "cargo test test_help_command --test e2e_tests -- --exact --nocapture"

run_test "Invalid command handling" \
    "cargo test test_invalid_command --test e2e_tests -- --exact --nocapture"

run_test "Environment variable handling" \
    "cargo test test_environment_variables --test e2e_tests -- --exact --nocapture"

# Run project management tests
echo ""
echo "🔧 Running Project Management Tests"
echo "==================================="

run_test "Project initialization" \
    "cargo test test_project_initialization --test e2e_tests -- --exact --nocapture" || true

run_test "Library initialization" \
    "cargo test test_library_initialization --test e2e_tests -- --exact --nocapture" || true

run_test "Build without project (error handling)" \
    "cargo test test_build_without_project --test e2e_tests -- --exact --nocapture" || true

# Run validation tests
echo ""
echo "🔍 Running CLI Validation Tests"
echo "==============================="

run_test "Access control validation" \
    "cargo test test_access_control_validation --test e2e_tests -- --exact --nocapture" || true

run_test "Invalid TOML handling" \
    "cargo test test_invalid_asthra_toml --test e2e_tests -- --exact --nocapture" || true

# =============================================================================
# ASTHRA LANGUAGE FEATURE TESTS
# =============================================================================

echo ""
echo "🎯 Running Asthra Language Feature Tests"
echo "========================================"

# Basic syntax tests
echo ""
echo "📝 Basic Language Syntax Tests"
echo "------------------------------"

run_test "Basic function declaration" \
    "cargo test test_basic_function_declaration --test language_tests -- --exact --nocapture" || true

run_test "Import declarations" \
    "cargo test test_import_declarations --test language_tests -- --exact --nocapture" || true

run_test "Import aliases (spec 6.2)" \
    "cargo test test_import_aliases --test language_tests -- --exact --nocapture" || true

# Type system tests
echo ""
echo "🔢 Type System Tests"
echo "-------------------"

run_test "Primitive types" \
    "cargo test test_primitive_types --test language_tests -- --exact --nocapture" || true

run_test "Slice types (spec section 4)" \
    "cargo test test_slice_types --test language_tests -- --exact --nocapture" || true

run_test "Pointer types (spec section 3)" \
    "cargo test test_pointer_types --test language_tests -- --exact --nocapture" || true

run_test "Type inference (spec section 6.1)" \
    "cargo test test_type_inference --test language_tests -- --exact --nocapture" || true

# String operations tests
echo ""
echo "📝 String Operations Tests (Spec Section 6)"
echo "-------------------------------------------"

run_test "String concatenation with + operator" \
    "cargo test test_string_concatenation --test language_tests -- --exact --nocapture" || true

# Result type and error handling
echo ""
echo "⚠️  Result Type and Error Handling Tests (Spec Section 5)"
echo "--------------------------------------------------------"

run_test "Result type declarations" \
    "cargo test test_result_type_declaration --test language_tests -- --exact --nocapture" || true

# FFI and memory model tests
echo ""
echo "🔗 FFI and Memory Model Tests (Spec Section 3)"
echo "----------------------------------------------"

run_test "Extern function declarations" \
    "cargo test test_extern_function_declaration --test language_tests -- --exact --nocapture" || true

run_test "Ownership annotations" \
    "cargo test test_ownership_annotations --test language_tests -- --exact --nocapture" || true

# Control flow tests
echo ""
echo "🔄 Control Flow Tests"
echo "--------------------"

run_test "If/else statements" \
    "cargo test test_if_else_statements --test language_tests -- --exact --nocapture" || true

run_test "For loops with range" \
    "cargo test test_for_loop --test language_tests -- --exact --nocapture" || true

# Observability tests
echo ""
echo "📊 Observability System Tests (Spec Section 7)"
echo "----------------------------------------------"

run_test "Built-in logging primitives" \
    "cargo test test_logging_primitives --test language_tests -- --exact --nocapture" || true

# Access control tests
echo ""
echo "🔒 Access Control Tests (Spec Section 1.6)"
echo "------------------------------------------"

run_test "Internal import restrictions" \
    "cargo test test_internal_import_restriction --test language_tests -- --exact --nocapture" || true

# Build integration tests
echo ""
echo "🏗️  Build Integration Tests"
echo "---------------------------"

run_test "Complete build process (Asthra to executable)" \
    "cargo test test_complete_build_process --test language_tests -- --exact --nocapture" || true

# Error case tests
echo ""
echo "❌ Language Error Case Tests"
echo "----------------------------"

run_test "Invalid syntax error detection" \
    "cargo test test_invalid_syntax_error --test language_tests -- --exact --nocapture" || true

run_test "Type mismatch error detection" \
    "cargo test test_type_mismatch_error --test language_tests -- --exact --nocapture" || true

# =============================================================================
# COMPREHENSIVE TEST RUNS
# =============================================================================

echo ""
echo "🎯 Running Complete Test Suites"
echo "==============================="

echo ""
echo "📋 CLI Test Suite Summary"
echo "-------------------------"
if cargo test --test e2e_tests; then
    echo "✅ CLI tests completed!"
    CLI_PASSED=$(cargo test --test e2e_tests 2>&1 | grep -o '[0-9]* passed' | head -1 | grep -o '[0-9]*' || echo "0")
    CLI_FAILED=$(cargo test --test e2e_tests 2>&1 | grep -o '[0-9]* failed' | head -1 | grep -o '[0-9]*' || echo "0")
    echo "📊 CLI Results: $CLI_PASSED passed, $CLI_FAILED failed"
else
    echo "⚠️  Some CLI tests failed (expected for unimplemented features)"
    CLI_PASSED=$(cargo test --test e2e_tests 2>&1 | grep -o '[0-9]* passed' | head -1 | grep -o '[0-9]*' || echo "0")
    CLI_FAILED=$(cargo test --test e2e_tests 2>&1 | grep -o '[0-9]* failed' | head -1 | grep -o '[0-9]*' || echo "0")
    echo "📊 CLI Results: $CLI_PASSED passed, $CLI_FAILED failed"
fi

echo ""
echo "🎯 Language Feature Test Suite Summary"
echo "-------------------------------------"
if cargo test --test language_tests; then
    echo "✅ Language tests completed!"
    LANG_PASSED=$(cargo test --test language_tests 2>&1 | grep -o '[0-9]* passed' | head -1 | grep -o '[0-9]*' || echo "0")
    LANG_FAILED=$(cargo test --test language_tests 2>&1 | grep -o '[0-9]* failed' | head -1 | grep -o '[0-9]*' || echo "0")
    echo "📊 Language Results: $LANG_PASSED passed, $LANG_FAILED failed"
else
    echo "⚠️  Some language tests failed (expected for unimplemented features)"
    LANG_PASSED=$(cargo test --test language_tests 2>&1 | grep -o '[0-9]* passed' | head -1 | grep -o '[0-9]*' || echo "0")
    LANG_FAILED=$(cargo test --test language_tests 2>&1 | grep -o '[0-9]* failed' | head -1 | grep -o '[0-9]*' || echo "0")
    echo "📊 Language Results: $LANG_PASSED passed, $LANG_FAILED failed"
fi

# Show comprehensive test summary
echo ""
echo "📊 Comprehensive Test Summary"
echo "============================="
echo "✅ Working CLI functionality:"
echo "   - Version and help commands"
echo "   - Error handling for invalid commands"
echo "   - Environment variable support"
echo "   - Basic CLI argument parsing"
echo ""
echo "⚠️  CLI functionality needing implementation:"
echo "   - Project initialization (ampu init)"
echo "   - Dependency management (ampu list, add)"
echo "   - File generation and project structure"
echo "   - Access control validation"
echo "   - Build and check commands"
echo ""
echo "🎯 Language features being tested:"
echo "   - Syntax parsing (functions, imports, types)"
echo "   - Type system (primitives, slices, pointers)"
echo "   - String operations (concatenation)"
echo "   - FFI and memory model (extern, ownership)"
echo "   - Control flow (if/else, loops)"
echo "   - Error handling (Result types)"
echo "   - Observability (logging primitives)"
echo "   - Access control (import restrictions)"
echo "   - Type inference system"
echo "   - Complete build integration"
echo ""
echo "🔧 Language features needing implementation:"
echo "   - Most language features are likely not implemented yet"
echo "   - Parser and compiler development needed"
echo "   - Semantic analysis and code generation"
echo "   - Standard library integration"
echo ""
echo "🎯 Next steps:"
echo "   1. Implement basic Asthra parser and AST"
echo "   2. Add 'ampu check' command for syntax validation"
echo "   3. Implement 'ampu init' command"
echo "   4. Add basic type checking"
echo "   5. Implement code generation pipeline"
echo ""
echo "📚 For more information, see:"
echo "   - tests/README.md - Comprehensive testing guide"
echo "   - tests/e2e_tests.rs - CLI functionality tests"
echo "   - tests/language_tests.rs - Language feature tests"
echo "   - docs/spec.md - Asthra language specification"
echo "   - TESTING_RECOMMENDATION.md - Testing approach guide"

echo ""
echo "🏁 Comprehensive test run complete!"
echo "Total CLI: $CLI_PASSED passed, $CLI_FAILED failed"
echo "Total Language: $LANG_PASSED passed, $LANG_FAILED failed" 
