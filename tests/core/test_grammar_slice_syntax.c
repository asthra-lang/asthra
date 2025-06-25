/**
 * Asthra Programming Language Slice Syntax Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for slice syntax parsing, including slice creation,
 * length access, and array literal syntax.
 */

#include "../framework/test_framework.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "test_grammar_helpers.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// SLICE SYNTAX TESTS
// =============================================================================

AsthraTestResult test_slice_creation_syntax(AsthraTestContext *ctx) {
    // Test slice creation syntax parsing

    // Mock test cases for slice creation
    const char *slice_creation_tests[] = {
        "let slice = &array[..];",                    // Full slice
        "let slice = &array[start..end];",            // Range slice
        "let slice = &array[..end];",                 // Start-to-index slice
        "let slice = &array[start..];",               // Index-to-end slice
        "let dynamic = &array[calculate_start()..];", // Dynamic indices
        NULL};

    for (int i = 0; slice_creation_tests[i] != NULL; i++) {
        // Mock slice creation parsing
        bool syntax_valid = grammar_test_parse_expression(slice_creation_tests[i]);
        bool indices_valid = grammar_test_validate_disambiguation(slice_creation_tests[i]);

        if (!ASTHRA_TEST_ASSERT(ctx, syntax_valid, "Invalid slice creation syntax: %s",
                                slice_creation_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(ctx, indices_valid, "Invalid indices in slice creation: %s",
                                slice_creation_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_len_access_syntax(AsthraTestContext *ctx) {
    // Test slice length access syntax

    // Mock test cases for slice length access
    const char *slice_len_tests[] = {
        "let length = slice.len();",                                    // Method-style length
        "let length = len(slice);",                                     // Function-style length
        "if slice.len() > 0 { process(slice); }",                       // Length in condition
        "for i in 0..slice.len(void) { total += slice[i]; }",           // Length in loop
        "return if slice.len() == 0 { None } else { Some(slice[0]) };", // Length in expression
        NULL};

    for (int i = 0; slice_len_tests[i] != NULL; i++) {
        // Mock length access parsing
        bool syntax_valid = grammar_test_parse_expression(slice_len_tests[i]);
        bool semantics_valid = grammar_test_validate_disambiguation(slice_len_tests[i]);

        if (!ASTHRA_TEST_ASSERT(ctx, syntax_valid, "Invalid slice length syntax: %s",
                                slice_len_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(ctx, semantics_valid, "Invalid slice length semantics: %s",
                                slice_len_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_slice_array_literal_syntax(AsthraTestContext *ctx) {
    // Test slice/array literal syntax

    // Mock test cases for array/slice literals
    const char *array_literal_tests[] = {
        "let array = [1, 2, 3, 4, 5];",              // Basic array literal
        "let nested = [[1, 2], [3, 4]];",            // Nested array literal
        "let mixed = [1, \"string\", true];",        // Heterogeneous array (if supported)
        "let computed = [calculate(), generate()];", // Computed elements
        "let repeated = [default_value; size];",     // Repeated element syntax
        NULL};

    for (int i = 0; array_literal_tests[i] != NULL; i++) {
        // Mock array literal parsing
        bool syntax_valid = grammar_test_parse_expression(array_literal_tests[i]);
        bool elements_valid = grammar_test_validate_disambiguation(array_literal_tests[i]);

        if (!ASTHRA_TEST_ASSERT(ctx, syntax_valid, "Invalid array literal syntax: %s",
                                array_literal_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(ctx, elements_valid, "Invalid elements in array literal: %s",
                                array_literal_tests[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("=== Asthra Grammar Slice Syntax Test Suite ===\n");

    // Create standard test context
    AsthraTestMetadata metadata = {.name = "Grammar Slice Syntax Tests",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "main",
                                   .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
                                   .timeout_ns = 30000000000ULL, // 30 seconds
                                   .skip = false,
                                   .skip_reason = NULL};

    AsthraTestContext ctx = {.metadata = metadata,
                             .result = ASTHRA_TEST_PASS,
                             .start_time_ns = 0,
                             .end_time_ns = 0,
                             .duration_ns = 0,
                             .error_message = NULL,
                             .error_message_allocated = false,
                             .assertions_in_test = 0,
                             .global_stats = NULL,
                             .user_data = NULL};

    int passed = 0;
    int total = 3;

    printf("\n[1/3] Running Slice Creation Syntax Test...\n");
    if (test_slice_creation_syntax(&ctx) == ASTHRA_TEST_PASS) {
        printf("  ✓ PASSED\n");
        passed++;
    } else {
        printf("  ✗ FAILED\n");
    }

    printf("\n[2/3] Running Slice Length Access Syntax Test...\n");
    if (test_slice_len_access_syntax(&ctx) == ASTHRA_TEST_PASS) {
        printf("  ✓ PASSED\n");
        passed++;
    } else {
        printf("  ✗ FAILED\n");
    }

    printf("\n[3/3] Running Slice Array Literal Syntax Test...\n");
    if (test_slice_array_literal_syntax(&ctx) == ASTHRA_TEST_PASS) {
        printf("  ✓ PASSED\n");
        passed++;
    } else {
        printf("  ✗ FAILED\n");
    }

    // No need to destroy stack-allocated context

    printf("\n=== Test Summary ===\n");
    printf("Passed: %d/%d\n", passed, total);
    printf("Success Rate: %.1f%%\n", (double)passed / total * 100.0);

    if (passed == total) {
        printf("🎉 All tests passed!\n");
        return 0;
    } else {
        printf("❌ Some tests failed.\n");
        return 1;
    }
}
