/**
 * Asthra Programming Language
 * Pattern Matching Tests - Main Test Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for pattern matching semantic analysis including:
 * - Match statement exhaustiveness checking
 * - Enum variant pattern validation
 * - Struct destructuring patterns
 * - Guard conditions
 * - If-let statements
 * - Pattern variable bindings
 * - Wildcard and literal patterns
 * - Type compatibility in patterns
 */

#include "test_pattern_matching_common.h"

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(int argc, char *argv[]) {
    printf("Running Pattern Matching Tests\n");
    printf("==============================\n\n");

    AsthraTestContext *context = asthra_test_create_context(NULL);
    if (!context) {
        printf("Failed to create test context\n");
        return 1;
    }

    int total_tests = 0;
    int passed_tests = 0;

    // Basic enum pattern matching tests
    printf("Running basic enum pattern matching tests...\n");
    if (test_basic_enum_pattern_matching(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Basic enum pattern matching tests passed\n");
    } else {
        printf("✗ Basic enum pattern matching tests failed\n");
    }
    total_tests++;

    // Struct pattern destructuring tests
    printf("\nRunning struct pattern destructuring tests...\n");
    if (test_struct_pattern_destructuring(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Struct pattern destructuring tests passed\n");
    } else {
        printf("✗ Struct pattern destructuring tests failed\n");
    }
    total_tests++;

    // Nested pattern matching tests
    printf("\nRunning nested pattern matching tests...\n");
    if (test_nested_pattern_matching(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Nested pattern matching tests passed\n");
    } else {
        printf("✗ Nested pattern matching tests failed\n");
    }
    total_tests++;

    // Guard conditions tests
    printf("\nRunning guard conditions tests...\n");
    if (test_guard_conditions(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Guard conditions tests passed\n");
    } else {
        printf("✗ Guard conditions tests failed\n");
    }
    total_tests++;

    // If-let statement tests
    printf("\nRunning if-let statement tests...\n");
    if (test_if_let_statements(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ If-let statement tests passed\n");
    } else {
        printf("✗ If-let statement tests failed\n");
    }
    total_tests++;

    // Pattern variable binding tests
    printf("\nRunning pattern variable binding tests...\n");
    if (test_pattern_variable_binding(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Pattern variable binding tests passed\n");
    } else {
        printf("✗ Pattern variable binding tests failed\n");
    }
    total_tests++;

    // Wildcard and literal pattern tests
    printf("\nRunning wildcard and literal pattern tests...\n");
    if (test_wildcard_and_literal_patterns(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Wildcard and literal pattern tests passed\n");
    } else {
        printf("✗ Wildcard and literal pattern tests failed\n");
    }
    total_tests++;

    // Exhaustiveness complex case tests
    printf("\nRunning exhaustiveness complex case tests...\n");
    if (test_exhaustiveness_complex_cases(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Exhaustiveness complex case tests passed\n");
    } else {
        printf("✗ Exhaustiveness complex case tests failed\n");
    }
    total_tests++;

    // Type compatibility in patterns tests
    printf("\nRunning type compatibility in patterns tests...\n");
    if (test_type_compatibility_in_patterns(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Type compatibility in patterns tests passed\n");
    } else {
        printf("✗ Type compatibility in patterns tests failed\n");
    }
    total_tests++;

    // Match expression vs statement tests
    printf("\nRunning match expression vs statement tests...\n");
    if (test_match_expression_vs_statement(context) == ASTHRA_TEST_PASS) {
        passed_tests++;
        printf("✓ Match expression vs statement tests passed\n");
    } else {
        printf("✗ Match expression vs statement tests failed\n");
    }
    total_tests++;

    printf("\n==============================\n");
    printf("Test Results: %d/%d passed\n", passed_tests, total_tests);

    asthra_test_destroy_context(context);

    return (passed_tests == total_tests) ? 0 : 1;
}