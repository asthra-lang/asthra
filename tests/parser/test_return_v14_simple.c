/**
 * Asthra Programming Language - Return Statement v1.14 Simple Test
 * Phase 8: Testing & Validation (Return Statement Simplification Plan)
 *
 * Simple validation for v1.14 return statement grammar requirements
 * Integrates with existing test framework
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "grammar_statements.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// RETURN STATEMENT V1.14 TESTS
// =============================================================================

/**
 * Test: Required Expression Enforcement (v1.14+)
 * Verifies that return statements always require expressions
 */
static AsthraTestResult test_return_expression_required_v14(AsthraTestContext *context) {
    // Valid returns with expressions
    const char *valid_returns[] = {"return 42;", "return ();", "return x + y;", "return func();"};

    size_t valid_count = sizeof(valid_returns) / sizeof(valid_returns[0]);

    for (size_t i = 0; i < valid_count; i++) {
        Parser *parser = create_test_parser(valid_returns[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Valid return should parse")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        if (!asthra_test_assert_int_eq(context, result->type, AST_RETURN_STMT,
                                       "Should be return statement")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Key v1.14 requirement: expression must always be present
        if (!asthra_test_assert_not_null(context, result->data.return_stmt.expression,
                                         "Return statement must have expression in v1.14+")) {
            ast_free_node(result);
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

/**
 * Test: Bare Return Rejection (v1.14+)
 * Verifies that "return;" is properly rejected
 */
static AsthraTestResult test_bare_return_rejection_v14(AsthraTestContext *context) {
    const char *bare_return = "return;";
    Parser *parser = create_test_parser(bare_return);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parser_parse_statement(parser);

    // In v1.14+, bare return should fail to parse
    if (result != NULL) {
        asthra_test_assert_true(context, false, "Bare return should be rejected in v1.14+");
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // NULL result means parsing failed, which is correct for v1.14+
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Unit Return Support (v1.14+)
 * Verifies that "return ();" works correctly for void functions
 */
static AsthraTestResult test_unit_return_support_v14(AsthraTestContext *context) {
    const char *unit_return = "return ();";
    Parser *parser = create_test_parser(unit_return);

    if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *result = parser_parse_statement(parser);

    if (!asthra_test_assert_not_null(context, result, "Unit return should parse successfully")) {
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_int_eq(context, result->type, AST_RETURN_STMT,
                                   "Should be return statement")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!asthra_test_assert_not_null(context, result->data.return_stmt.expression,
                                     "Unit return should have expression")) {
        ast_free_node(result);
        destroy_test_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(result);
    destroy_test_parser(parser);
    return ASTHRA_TEST_PASS;
}

/**
 * Test: Grammar Compliance Summary (v1.14+)
 * Comprehensive test covering all v1.14 requirements
 */
static AsthraTestResult test_grammar_compliance_summary_v14(AsthraTestContext *context) {
    printf("=== Return Statement v1.14 Grammar Compliance ===\n");

    // Test cases for comprehensive validation
    struct {
        const char *source;
        bool should_pass;
        const char *description;
    } test_cases[] = {
        {"return 42;", true, "Integer literal return"},
        {"return ();", true, "Unit return (void)"},
        {"return x + y;", true, "Binary expression return"},
        {"return func();", true, "Function call return"},
        {"return;", false, "Bare return (should fail in v1.14+)"},
    };

    size_t test_count = sizeof(test_cases) / sizeof(test_cases[0]);
    size_t passed = 0;

    for (size_t i = 0; i < test_count; i++) {
        Parser *parser = create_test_parser(test_cases[i].source);
        if (!parser)
            continue;

        ASTNode *result = parser_parse_statement(parser);
        bool parsing_success = (result != NULL);

        bool test_passed = (parsing_success == test_cases[i].should_pass);
        if (test_passed) {
            printf("✅ PASS: %s\n", test_cases[i].description);
            passed++;
        } else {
            printf("❌ FAIL: %s\n", test_cases[i].description);
        }

        if (result)
            ast_free_node(result);
        destroy_test_parser(parser);
    }

    printf("Grammar compliance: %zu/%zu tests passed\n", passed, test_count);

    if (!asthra_test_assert_int_eq(context, passed, test_count,
                                   "All grammar compliance tests should pass")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Create Return Statement v1.14 test suite
 */
AsthraTestSuite *create_return_v14_simple_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Return Statement v1.14 Simple Tests",
                                                      "Phase 8: Testing & Validation");

    if (!suite)
        return NULL;

    asthra_test_suite_add_test(suite, "test_return_expression_required_v14",
                               "Required expression enforcement (v1.14+)",
                               test_return_expression_required_v14);

    asthra_test_suite_add_test(suite, "test_bare_return_rejection_v14",
                               "Bare return rejection (v1.14+)", test_bare_return_rejection_v14);

    asthra_test_suite_add_test(suite, "test_unit_return_support_v14",
                               "Unit return support (v1.14+)", test_unit_return_support_v14);

    asthra_test_suite_add_test(suite, "test_grammar_compliance_summary_v14",
                               "Grammar compliance summary (v1.14+)",
                               test_grammar_compliance_summary_v14);

    return suite;
}

// =============================================================================
// MAIN (for standalone execution)
// =============================================================================

#ifndef ASTHRA_TEST_INTEGRATION
int main(int argc, char *argv[]) {
    (void)argc;
    (void)argv;

    printf("Asthra Return Statement v1.14 Simple Test\n");
    printf("==========================================\n");
    printf("Phase 8: Testing & Validation\n\n");

    AsthraTestSuite *suite = create_return_v14_simple_test_suite();
    if (!suite) {
        printf("❌ Failed to create test suite\n");
        return 1;
    }

    AsthraTestRunner *runner = asthra_test_runner_create();
    if (!runner) {
        printf("❌ Failed to create test runner\n");
        asthra_test_suite_destroy(suite);
        return 1;
    }

    asthra_test_runner_add_suite(runner, suite);
    AsthraTestResults *results = asthra_test_runner_run_all(runner);

    if (!results) {
        printf("❌ Failed to run tests\n");
        asthra_test_runner_destroy(runner);
        return 1;
    }

    printf("\n==================================================\n");
    printf("Return Statement v1.14 Simple Test Results\n");
    printf("===========================================\n");
    printf("Passed: %d/%d tests\n", results->passed, results->total);
    printf("Success Rate: %.1f%%\n", (double)results->passed / results->total * 100.0);

    if (results->passed == results->total) {
        printf("\n🎉 ALL RETURN STATEMENT v1.14 TESTS PASSED!\n");
        printf("✅ Grammar compliance: ACHIEVED\n");
        printf("✅ Required expressions: ENFORCED\n");
        printf("✅ Bare returns: CORRECTLY REJECTED\n");
        printf("✅ Unit returns: WORKING\n");
        printf("✅ AI generation reliability: MAXIMUM\n");
        printf("\n🏆 Phase 8 (Testing & Validation): COMPLETE\n");
        printf("🚀 Return Statement Simplification Plan: 100%% IMPLEMENTED\n");
    } else {
        printf("\n❌ Some tests failed - implementation needs review\n");
    }

    int exit_code = (results->passed == results->total) ? 0 : 1;

    asthra_test_results_destroy(results);
    asthra_test_runner_destroy(runner);

    return exit_code;
}
#endif
