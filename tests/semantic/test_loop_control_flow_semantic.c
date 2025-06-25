/**
 * Asthra Programming Language
 * Loop Control Flow Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for break and continue statement semantic validation
 */

#include "../framework/parser_test_utils.h"
#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework.h"
#include "ast_operations.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "test_mutability_common.h"
#include "test_type_system_common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool test_semantic_success(const char *source, const char *test_name) {
    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        fprintf(stderr, "[%s] Failed to create semantic analyzer\n", test_name);
        return false;
    }

    ASTNode *ast = parse_test_source_code(source, test_name);
    if (!ast) {
        fprintf(stderr, "[%s] Failed to parse source\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast_node(analyzer, ast);
    if (!success) {
        fprintf(stderr, "[%s] Semantic analysis failed unexpectedly\n", test_name);
        SemanticError *error = analyzer->errors;
        while (error) {
            fprintf(stderr, "  Error: %s at line %d, column %d\n", error->message,
                    error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_semantic_error(const char *source, SemanticErrorCode expected_error,
                                const char *test_name) {
    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        fprintf(stderr, "[%s] Failed to create semantic analyzer\n", test_name);
        return false;
    }

    ASTNode *ast = parse_test_source_code(source, test_name);
    if (!ast) {
        fprintf(stderr, "[%s] Failed to parse source\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool analysis_result = analyze_test_ast_node(analyzer, ast);
    bool has_expected_error = false;

    SemanticError *error = analyzer->errors;
    while (error) {
        if (error->code == expected_error) {
            has_expected_error = true;
            break;
        }
        error = error->next;
    }

    if (analysis_result && !has_expected_error) {
        fprintf(stderr, "[%s] Expected error %d but analysis succeeded\n", test_name,
                expected_error);
    } else if (!has_expected_error) {
        fprintf(stderr, "[%s] Expected error %d but got different errors:\n", test_name,
                expected_error);
        error = analyzer->errors;
        while (error) {
            fprintf(stderr, "  Got error %d: %s\n", error->code, error->message);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return has_expected_error;
}

// =============================================================================
// BREAK STATEMENT TESTS
// =============================================================================

static AsthraTestResult test_break_in_for_loop(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let mut sum: i32 = 0;\n"
                         "    for i in range(10) {\n"
                         "        if i > 5 {\n"
                         "            break;\n"
                         "        }\n"
                         "        sum = sum + i;\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_semantic_success(source, "break_in_for_loop"),
                            "Break statement should be valid within for loop");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_break_outside_loop(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: i32 = 5;\n"
                         "    if x > 0 {\n"
                         "        break;  // ERROR: break outside loop\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(
        context,
        test_semantic_error(source, SEMANTIC_ERROR_INVALID_OPERATION, "break_outside_loop"),
        "Break statement outside loop should fail");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_break_in_nested_loops(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    for i in range(5) {\n"
                         "        for j in range(5) {\n"
                         "            if j > 2 {\n"
                         "                break;  // Breaks inner loop only\n"
                         "            }\n"
                         "        }\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_semantic_success(source, "break_in_nested_loops"),
                            "Break statement should work in nested loops");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// CONTINUE STATEMENT TESTS
// =============================================================================

static AsthraTestResult test_continue_in_for_loop(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let mut sum: i32 = 0;\n"
                         "    for i in range(10) {\n"
                         "        if i % 2 == 0 {\n"
                         "            continue;  // Skip even numbers\n"
                         "        }\n"
                         "        sum = sum + i;\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_semantic_success(source, "continue_in_for_loop"),
                            "Continue statement should be valid within for loop");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_continue_outside_loop(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: i32 = 5;\n"
                         "    if x > 0 {\n"
                         "        continue;  // ERROR: continue outside loop\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(
        context,
        test_semantic_error(source, SEMANTIC_ERROR_INVALID_OPERATION, "continue_outside_loop"),
        "Continue statement outside loop should fail");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_continue_in_nested_loops(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let mut count: i32 = 0;\n"
                         "    for i in range(3) {\n"
                         "        for j in range(3) {\n"
                         "            if i == j {\n"
                         "                continue;  // Skip when i equals j\n"
                         "            }\n"
                         "            count = count + 1;\n"
                         "        }\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_semantic_success(source, "continue_in_nested_loops"),
                            "Continue statement should work in nested loops");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MIXED CONTROL FLOW TESTS
// =============================================================================

static AsthraTestResult test_break_and_continue_together(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let mut sum: i32 = 0;\n"
                         "    for i in range(20) {\n"
                         "        if i > 10 {\n"
                         "            break;  // Exit loop\n"
                         "        }\n"
                         "        if i % 2 == 0 {\n"
                         "            continue;  // Skip even numbers\n"
                         "        }\n"
                         "        sum = sum + i;\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_semantic_success(source, "break_and_continue_together"),
                            "Break and continue should work together in same loop");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_control_flow_in_match_error(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: i32 = 5;\n"
                         "    match x {\n"
                         "        1 => { break; }  // ERROR: break in match\n"
                         "        _ => { return (); }\n"
                         "    }\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(
        context,
        test_semantic_error(source, SEMANTIC_ERROR_INVALID_OPERATION, "control_flow_in_match"),
        "Break statement in match should fail");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

AsthraTestSuite *create_loop_control_flow_semantic_test_suite(void) {
    AsthraTestSuite *suite =
        asthra_test_suite_create("Loop Control Flow Semantic Tests",
                                 "Semantic validation for break and continue statements");

    if (!suite)
        return NULL;

    // Break statement tests
    asthra_test_suite_add_test(suite, "test_break_in_for_loop", "Break statement in for loop",
                               test_break_in_for_loop);

    asthra_test_suite_add_test(suite, "test_break_outside_loop",
                               "Break statement outside loop should fail", test_break_outside_loop);

    asthra_test_suite_add_test(suite, "test_break_in_nested_loops",
                               "Break statement in nested loops", test_break_in_nested_loops);

    // Continue statement tests
    asthra_test_suite_add_test(suite, "test_continue_in_for_loop", "Continue statement in for loop",
                               test_continue_in_for_loop);

    asthra_test_suite_add_test(suite, "test_continue_outside_loop",
                               "Continue statement outside loop should fail",
                               test_continue_outside_loop);

    asthra_test_suite_add_test(suite, "test_continue_in_nested_loops",
                               "Continue statement in nested loops", test_continue_in_nested_loops);

    // Mixed control flow tests
    asthra_test_suite_add_test(suite, "test_break_and_continue_together",
                               "Break and continue in same loop", test_break_and_continue_together);

    asthra_test_suite_add_test(suite, "test_control_flow_in_match_error",
                               "Control flow statements in match should fail",
                               test_control_flow_in_match_error);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Loop Control Flow Semantic Tests ===\n\n");

    AsthraTestSuite *suite = create_loop_control_flow_semantic_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED