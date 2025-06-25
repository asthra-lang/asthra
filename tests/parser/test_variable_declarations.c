/**
 * Asthra Programming Language
 * Variable Declaration Tests
 *
 * Tests for parsing variable declarations with type annotations.
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_data.h"
#include "../framework/test_framework.h"
#include "grammar_statements.h"
#include "parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST SETUP AND TEARDOWN
// =============================================================================

static AsthraTestResult setup_variable_declaration_tests(AsthraTestContext *context) {
    // Initialize any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult teardown_variable_declaration_tests(AsthraTestContext *context) {
    // Clean up any global state if needed
    (void)context; // Suppress unused parameter warning
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// VARIABLE DECLARATION TESTS
// =============================================================================

/**
 * Test: Parse Variable Declarations
 * Verifies that variable declarations are parsed correctly
 */
static AsthraTestResult test_parse_variable_declarations(AsthraTestContext *context) {
    const char *var_declarations[] = {
        "let x: i32 = 42;",             // v1.15+ requires type annotation
        "let y: f64 = 3.14;",           // v1.15+ requires type annotation
        "let name: string = \"hello\";" // v1.15+ requires type annotation
    };

    size_t decl_count = sizeof(var_declarations) / sizeof(var_declarations[0]);

    for (size_t i = 0; i < decl_count; i++) {
        Parser *parser = create_test_parser(var_declarations[i]);

        if (!asthra_test_assert_not_null(context, parser, "Failed to create test parser")) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_statement(parser);

        if (!asthra_test_assert_not_null(context, result, "Failed to parse variable declaration")) {
            destroy_test_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        destroy_test_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE REGISTRATION
// =============================================================================

/**
 * Register all variable declaration tests
 */
AsthraTestSuite *create_variable_declaration_test_suite(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("Variable Declaration Tests",
                                                      "Variable declaration parsing testing");

    if (!suite)
        return NULL;

    // Register setup and teardown
    asthra_test_suite_set_setup(suite, setup_variable_declaration_tests);
    asthra_test_suite_set_teardown(suite, teardown_variable_declaration_tests);

    // Variable declaration tests
    asthra_test_suite_add_test(suite, "test_parse_variable_declarations",
                               "Parse variable declarations", test_parse_variable_declarations);

    return suite;
}

// =============================================================================
// MAIN FUNCTION FOR STANDALONE TESTING
// =============================================================================

#ifndef ASTHRA_TEST_COMBINED
int main(void) {
    printf("=== Asthra Variable Declaration Tests ===\n\n");

    AsthraTestSuite *suite = create_variable_declaration_test_suite();
    if (!suite) {
        fprintf(stderr, "Failed to create test suite\n");
        return 1;
    }

    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);

    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
#endif // ASTHRA_TEST_COMBINED