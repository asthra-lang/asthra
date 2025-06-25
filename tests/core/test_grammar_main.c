/**
 * Asthra Programming Language Grammar Tests - Main Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main entry point for grammar and parsing tests.
 * This file coordinates all grammar test modules including:
 * - Grammar disambiguation tests
 * - Pattern matching syntax tests
 * - String operations syntax tests
 * - Slice syntax tests
 */

#include "test_comprehensive.h"
#include "test_grammar_helpers.h"
#include <stdio.h>
#include <string.h>

// =============================================================================
// EXTERNAL TEST FUNCTION DECLARATIONS
// =============================================================================

// Grammar disambiguation tests (from test_grammar_disambiguation.c)
AsthraTestResult test_grammar_postfix_expr_disambiguation(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_precedence_rules(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_zero_parsing_conflicts(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_semantic_tags_parsing(AsthraV12TestContext *ctx);
AsthraTestResult test_grammar_ffi_annotations_parsing(AsthraV12TestContext *ctx);

// Pattern matching syntax tests (from test_grammar_pattern_matching.c)
AsthraTestResult test_pattern_match_statement_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_if_let_constructs(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_destructuring_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_pattern_result_type_syntax(AsthraV12TestContext *ctx);

// String operations syntax tests (from test_grammar_string_operations.c)
AsthraTestResult test_string_concatenation_syntax(AsthraV12TestContext *ctx);
// String interpolation test removed - feature deprecated for AI generation efficiency
AsthraTestResult test_string_edge_case_parsing(AsthraV12TestContext *ctx);

// Slice syntax tests (from test_grammar_slice_syntax.c)
AsthraTestResult test_slice_creation_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_len_access_syntax(AsthraV12TestContext *ctx);
AsthraTestResult test_slice_array_literal_syntax(AsthraV12TestContext *ctx);

// =============================================================================
// GRAMMAR TEST SUITE RUNNER
// =============================================================================

/**
 * Run all grammar disambiguation tests
 */
AsthraTestResult run_grammar_disambiguation_tests(AsthraV12TestContext *ctx) {
    AsthraTestResult results[] = {
        test_grammar_postfix_expr_disambiguation(ctx), test_grammar_precedence_rules(ctx),
        test_grammar_zero_parsing_conflicts(ctx), test_grammar_semantic_tags_parsing(ctx),
        test_grammar_ffi_annotations_parsing(ctx)};

    for (size_t i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        if (results[i] != ASTHRA_TEST_PASS) {
            return results[i];
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Run all pattern matching syntax tests
 */
AsthraTestResult run_pattern_matching_syntax_tests(AsthraV12TestContext *ctx) {
    AsthraTestResult results[] = {
        test_pattern_match_statement_syntax(ctx), test_pattern_if_let_constructs(ctx),
        test_pattern_destructuring_syntax(ctx), test_pattern_result_type_syntax(ctx)};

    for (size_t i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        if (results[i] != ASTHRA_TEST_PASS) {
            return results[i];
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Run all string operations syntax tests
 */
AsthraTestResult run_string_operations_syntax_tests(AsthraV12TestContext *ctx) {
    AsthraTestResult results[] = {test_string_concatenation_syntax(ctx),
                                  test_string_edge_case_parsing(ctx)};

    for (size_t i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        if (results[i] != ASTHRA_TEST_PASS) {
            return results[i];
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Run all slice syntax tests
 */
AsthraTestResult run_slice_syntax_tests(AsthraV12TestContext *ctx) {
    AsthraTestResult results[] = {test_slice_creation_syntax(ctx),
                                  test_slice_len_access_syntax(ctx),
                                  test_slice_array_literal_syntax(ctx)};

    for (size_t i = 0; i < sizeof(results) / sizeof(results[0]); i++) {
        if (results[i] != ASTHRA_TEST_PASS) {
            return results[i];
        }
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Run all grammar tests
 */
AsthraTestResult run_all_grammar_tests(AsthraV12TestContext *ctx) {
    printf("Running Grammar Disambiguation Tests...\n");
    AsthraTestResult disambiguation_result = run_grammar_disambiguation_tests(ctx);
    if (disambiguation_result != ASTHRA_TEST_PASS) {
        printf("Grammar disambiguation tests failed\n");
        return disambiguation_result;
    }

    printf("Running Pattern Matching Syntax Tests...\n");
    AsthraTestResult pattern_result = run_pattern_matching_syntax_tests(ctx);
    if (pattern_result != ASTHRA_TEST_PASS) {
        printf("Pattern matching syntax tests failed\n");
        return pattern_result;
    }

    printf("Running String Operations Syntax Tests...\n");
    AsthraTestResult string_result = run_string_operations_syntax_tests(ctx);
    if (string_result != ASTHRA_TEST_PASS) {
        printf("String operations syntax tests failed\n");
        return string_result;
    }

    printf("Running Slice Syntax Tests...\n");
    AsthraTestResult slice_result = run_slice_syntax_tests(ctx);
    if (slice_result != ASTHRA_TEST_PASS) {
        printf("Slice syntax tests failed\n");
        return slice_result;
    }

    printf("All grammar tests passed!\n");
    return ASTHRA_TEST_PASS;
}
