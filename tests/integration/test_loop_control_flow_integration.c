/**
 * Asthra Programming Language
 * Integration Tests for Loop Control Flow (break/continue)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests complex scenarios involving break and continue statements
 */

#include "../../src/analysis/semantic_analyzer.h"
#include "../../src/parser/lexer.h"
#include "../../src/parser/parser.h"
#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool compile_and_analyze(const char *source, const char *test_name) {
    // Create lexer first
    Lexer *lexer = lexer_create(source, strlen(source), test_name);
    if (!lexer) {
        fprintf(stderr, "[%s] Failed to create lexer\n", test_name);
        return false;
    }

    // Create parser with lexer
    Parser *parser = parser_create(lexer);
    if (!parser) {
        fprintf(stderr, "[%s] Failed to create parser\n", test_name);
        lexer_destroy(lexer);
        return false;
    }

    // Parse the source
    ASTNode *ast = parser_parse_program(parser);

    // Check for parser errors
    if (parser->error_count > 0) {
        fprintf(stderr, "[%s] Parser errors: %zu\n", test_name, parser->error_count);
        for (size_t i = 0; i < parser->error_count; i++) {
            fprintf(stderr, "  Error at %s:%d:%d: %s\n",
                    parser->errors[i].location.filename ? parser->errors[i].location.filename
                                                        : test_name,
                    parser->errors[i].location.line, parser->errors[i].location.column,
                    parser->errors[i].message);
        }
    }

    // Clean up parser (also cleans up lexer)
    parser_destroy(parser);

    if (!ast) {
        fprintf(stderr, "[%s] Failed to parse source\n", test_name);
        return false;
    }

    // Run semantic analysis
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        fprintf(stderr, "[%s] Failed to create semantic analyzer\n", test_name);
        ast_free_node(ast);
        return false;
    }

    bool success = semantic_analyze_program(analyzer, ast);
    if (!success) {
        fprintf(stderr, "[%s] Semantic analysis failed\n", test_name);
        // Print semantic errors
        if (analyzer->error_count > 0) {
            fprintf(stderr, "[%s] Semantic errors: %zu\n", test_name, analyzer->error_count);
            for (size_t i = 0; i < analyzer->error_count; i++) {
                const SemanticError *error = &analyzer->errors[i];
                fprintf(stderr, "  Error at %s:%d:%d: %s\n",
                        error->location.filename ? error->location.filename : test_name,
                        error->location.line, error->location.column, error->message);
            }
        }
    }

    // Clean up
    semantic_analyzer_destroy(analyzer);
    ast_free_node(ast);

    return success;
}

// =============================================================================
// INTEGRATION TESTS
// =============================================================================

static AsthraTestResult test_nested_loops_with_break(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Test breaking out of nested loops\n"
                         "pub fn find_pair(none) -> i32 {\n"
                         "    let mut result: i32 = -1;\n"
                         "    \n"
                         "    for i in range(10) {\n"
                         "        for j in range(10) {\n"
                         "            if i + j == 15 {\n"
                         "                result = i * 10 + j;\n"
                         "                break;  // Breaks inner loop only\n"
                         "            }\n"
                         "        }\n"
                         "        let neg_one: i32 = -1;\n"
                         "        if result != neg_one {\n"
                         "            break;  // Breaks outer loop\n"
                         "        }\n"
                         "    }\n"
                         "    \n"
                         "    return result;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, compile_and_analyze(source, "nested_loops_with_break"),
                            "Nested loops with break should compile successfully");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_continue_with_side_effects(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Test continue statement with side effects\n"
                         "pub fn sum_odd_numbers(none) -> i32 {\n"
                         "    let mut sum: i32 = 0;\n"
                         "    let mut count: i32 = 0;\n"
                         "    \n"
                         "    for i in range(20) {\n"
                         "        count = count + 1;  // Always incremented\n"
                         "        \n"
                         "        if i % 2 == 0 {\n"
                         "            continue;  // Skip even numbers\n"
                         "        }\n"
                         "        \n"
                         "        sum = sum + i;  // Only for odd numbers\n"
                         "    }\n"
                         "    \n"
                         "    return sum;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, compile_and_analyze(source, "continue_with_side_effects"),
                            "Continue with side effects should compile successfully");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_break_continue_complex_conditions(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Complex control flow with multiple break/continue conditions\n"
                         "pub fn process_data(none) -> i32 {\n"
                         "    let mut result: i32 = 0;\n"
                         "    let mut processed: i32 = 0;\n"
                         "    \n"
                         "    for i in range(100) {\n"
                         "        // Skip negative values (simulated)\n"
                         "        if i < 10 {\n"
                         "            continue;\n"
                         "        }\n"
                         "        \n"
                         "        // Stop if we find a sentinel value\n"
                         "        if i == 42 {\n"
                         "            break;\n"
                         "        }\n"
                         "        \n"
                         "        // Skip multiples of 5\n"
                         "        if i % 5 == 0 {\n"
                         "            continue;\n"
                         "        }\n"
                         "        \n"
                         "        // Process the value\n"
                         "        result = result + i;\n"
                         "        processed = processed + 1;\n"
                         "        \n"
                         "        // Stop after processing 10 values\n"
                         "        if processed >= 10 {\n"
                         "            break;\n"
                         "        }\n"
                         "    }\n"
                         "    \n"
                         "    return result;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context,
                            compile_and_analyze(source, "break_continue_complex_conditions"),
                            "Complex break/continue conditions should compile successfully");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_loop_control_with_pattern_matching(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Test loop control with pattern matching\n"
                         "pub fn find_first_match(none) -> i32 {\n"
                         "    let mut result: i32 = -1;\n"
                         "    \n"
                         "    for i in range(10) {\n"
                         "        let value: i32 = i * 2;\n"
                         "        \n"
                         "        if value > 10 {\n"
                         "            result = value;\n"
                         "            break;  // Found first match, exit loop\n"
                         "        }\n"
                         "        \n"
                         "        if value % 3 == 0 {\n"
                         "            continue;  // Skip multiples of 3\n"
                         "        }\n"
                         "    }\n"
                         "    \n"
                         "    return result;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context,
                            compile_and_analyze(source, "loop_control_with_pattern_matching"),
                            "Loop control with pattern matching should compile successfully");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_multiple_nested_loops(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Test with three levels of nested loops\n"
                         "pub fn find_triplet(none) -> i32 {\n"
                         "    let mut found: bool = false;\n"
                         "    let mut result: i32 = 0;\n"
                         "    \n"
                         "    for i in range(10) {\n"
                         "        if found {\n"
                         "            break;\n"
                         "        }\n"
                         "        \n"
                         "        for j in range(10) {\n"
                         "            if found {\n"
                         "                break;\n"
                         "            }\n"
                         "            \n"
                         "            for k in range(10) {\n"
                         "                if i * i + j * j == k * k {\n"
                         "                    result = i * 100 + j * 10 + k;\n"
                         "                    found = true;\n"
                         "                    break;  // Exit innermost loop\n"
                         "                }\n"
                         "                \n"
                         "                if k > 5 {\n"
                         "                    continue;  // Skip rest of iteration\n"
                         "                }\n"
                         "            }\n"
                         "        }\n"
                         "    }\n"
                         "    \n"
                         "    return result;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(
        context, compile_and_analyze(source, "multiple_nested_loops"),
        "Multiple nested loops with break/continue should compile successfully");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_loop_control_in_unsafe_block(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Test break/continue with unsafe blocks\n"
                         "pub fn search_with_unsafe(none) -> i32 {\n"
                         "    let mut result: i32 = 0;\n"
                         "    \n"
                         "    for i in range(100) {\n"
                         "        let mut value: i32 = 0;\n"
                         "        unsafe {\n"
                         "            // Simulate unsafe operation\n"
                         "            value = i * 3;\n"
                         "        }\n"
                         "        \n"
                         "        if value < 0 {\n"
                         "            continue;  // Skip negative values\n"
                         "        }\n"
                         "        \n"
                         "        if value % 42 == 0 {\n"
                         "            result = value;\n"
                         "            break;  // Found our special value\n"
                         "        }\n"
                         "    }\n"
                         "    \n"
                         "    return result;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, compile_and_analyze(source, "loop_control_in_unsafe_block"),
                            "Loop control in unsafe blocks should compile successfully");

    return ASTHRA_TEST_PASS;
}

// Test continue statement in for loops (should not cause infinite loop)
static AsthraTestResult test_for_loop_continue_increment(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "// Test continue in for loop properly increments\n"
                         "pub fn test_continue_increment(none) -> i32 {\n"
                         "    let mut count: i32 = 0;\n"
                         "    \n"
                         "    // Test 1: Continue in range-based for loop\n"
                         "    for i in range(5) {\n"
                         "        if i == 2 {\n"
                         "            continue;  // Should skip to i=3, not stay at i=2\n"
                         "        }\n"
                         "        count = count + 1;\n"
                         "    }\n"
                         "    \n"
                         "    // count should be 4 (0,1,3,4 - skipped 2)\n"
                         "    if count != 4 {\n"
                         "        return 1;\n"
                         "    }\n"
                         "    \n"
                         "    // Test 2: Multiple continues\n"
                         "    count = 0;\n"
                         "    for j in range(10) {\n"
                         "        if j % 2 == 0 {\n"
                         "            continue;  // Skip even numbers\n"
                         "        }\n"
                         "        count = count + 1;\n"
                         "    }\n"
                         "    \n"
                         "    // count should be 5 (1,3,5,7,9)\n"
                         "    if count != 5 {\n"
                         "        return 2;\n"
                         "    }\n"
                         "    \n"
                         "    // Test 3: Continue in nested loops\n"
                         "    count = 0;\n"
                         "    for x in range(3) {\n"
                         "        for y in range(3) {\n"
                         "            if x == y {\n"
                         "                continue;  // Skip diagonal\n"
                         "            }\n"
                         "            count = count + 1;\n"
                         "        }\n"
                         "    }\n"
                         "    \n"
                         "    // count should be 6 (skipped (0,0), (1,1), (2,2))\n"
                         "    if count != 6 {\n"
                         "        return 3;\n"
                         "    }\n"
                         "    \n"
                         "    return 0;  // Success\n"
                         "}\n";

    bool success = compile_and_analyze(source, "test_for_loop_continue_increment");
    ASTHRA_TEST_ASSERT_TRUE(context, success,
                            "For loop with continue statements should compile successfully");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

AsthraTestSuite *create_loop_control_flow_integration_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Loop Control Flow Integration Tests",
                                 "Complex scenarios for break and continue statements");

    if (!suite)
        return NULL;

    asthra_test_suite_add_test(suite, "test_nested_loops_with_break",
                               "Nested loops with break statements", test_nested_loops_with_break);

    asthra_test_suite_add_test(suite, "test_continue_with_side_effects",
                               "Continue statement with side effects",
                               test_continue_with_side_effects);

    asthra_test_suite_add_test(suite, "test_break_continue_complex_conditions",
                               "Complex conditions with multiple break/continue",
                               test_break_continue_complex_conditions);

    asthra_test_suite_add_test(suite, "test_loop_control_with_pattern_matching",
                               "Loop control flow with pattern matching",
                               test_loop_control_with_pattern_matching);

    asthra_test_suite_add_test(suite, "test_multiple_nested_loops", "Three levels of nested loops",
                               test_multiple_nested_loops);

    asthra_test_suite_add_test(suite, "test_loop_control_in_unsafe_block",
                               "Loop control with unsafe blocks",
                               test_loop_control_in_unsafe_block);

    asthra_test_suite_add_test(suite, "test_for_loop_continue_increment",
                               "Continue in for loops increments properly",
                               test_for_loop_continue_increment);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Loop Control Flow Integration Tests ===\n\n");

    AsthraTestSuite *suite = create_loop_control_flow_integration_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED