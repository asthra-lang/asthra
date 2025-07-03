/**
 * Postfix Expression Ambiguity Tests (v1.20 Grammar Fix)
 *
 * This module tests the v1.20 grammar fix that eliminates postfix :: operator
 * ambiguity by restricting :: usage to type contexts only (AssociatedFuncCall).
 *
 * Test Coverage:
 * - Rejection of invalid postfix :: usage
 * - Validation of correct associated function calls
 * - Generic type associated function support
 * - Primary vs postfix disambiguation
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "test_pub_impl_self_common.h"

// =============================================================================
// POSTFIX EXPRESSION AMBIGUITY FIX TESTS (v1.20)
// =============================================================================

static AsthraTestResult test_reject_postfix_double_colon(AsthraTestContext *context) {
    // Test that invalid postfix :: usage is rejected per v1.20 grammar fix
    const char *invalid_expressions[] = {"42::to_string()",           // Literal with ::
                                         "some_function()::method()", // Function call with ::
                                         "(x + y)::process()",        // Expression with ::
                                         "array[0]::convert()",       // Array access with ::
                                         "obj.field::transform()",    // Field access with ::
                                         "func()::other()::final()",  // Chained :: usage
                                         NULL};

    for (int i = 0; invalid_expressions[i] != NULL; i++) {
        Parser *parser = create_test_parser(invalid_expressions[i]);

        if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created for: %s",
                                invalid_expressions[i])) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_expression(parser);

        // Should fail to parse or produce error due to invalid postfix :: usage
        if (!ASTHRA_TEST_ASSERT(context, result == NULL,
                                "Should reject invalid postfix :: usage: %s",
                                invalid_expressions[i])) {
            if (result)
                ast_free_node(result);
            cleanup_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        cleanup_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_valid_associated_function_calls(AsthraTestContext *context) {
    // Ensure valid :: usage still works per v1.20 grammar
    const char *valid_expressions[] = {"Vec::new()",       // Simple associated function
                                       "Point::default()", // Another simple case
                                       "MyStruct::create(42, \"test\")", // With arguments
                                       "Result.Ok(value)",               // Enum constructor style
                                       NULL};

    for (int i = 0; valid_expressions[i] != NULL; i++) {
        Parser *parser = create_test_parser(valid_expressions[i]);

        if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created for: %s",
                                valid_expressions[i])) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_expression(parser);

        if (!ASTHRA_TEST_ASSERT(context, result != NULL, "Should parse valid :: usage: %s",
                                valid_expressions[i])) {
            cleanup_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(context, result->type == AST_ASSOCIATED_FUNC_CALL,
                                "Should create AST_ASSOCIATED_FUNC_CALL for: %s",
                                valid_expressions[i])) {
            ast_free_node(result);
            cleanup_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        ast_free_node(result);
        cleanup_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_generic_type_associated_functions(AsthraTestContext *context) {
    // Test enhanced generic type support added in v1.20
    const char *generic_expressions[] = {
        "Vec<i32>::new()",                  // Single type parameter
        "Result<String, Error>::Ok(value)", // Multiple type parameters
        "HashMap<String, i32>::new()",      // Complex generic types
        "Option<Point>::Some(point)",       // Nested type usage
        NULL};

    for (int i = 0; generic_expressions[i] != NULL; i++) {
        Parser *parser = create_test_parser(generic_expressions[i]);

        if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created for: %s",
                                generic_expressions[i])) {
            return ASTHRA_TEST_FAIL;
        }

        ASTNode *result = parser_parse_expression(parser);

        if (!ASTHRA_TEST_ASSERT(context, result != NULL, "Should parse generic type :: usage: %s",
                                generic_expressions[i])) {
            cleanup_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        if (!ASTHRA_TEST_ASSERT(context, result->type == AST_ASSOCIATED_FUNC_CALL,
                                "Should create AST_ASSOCIATED_FUNC_CALL for generic: %s",
                                generic_expressions[i])) {
            ast_free_node(result);
            cleanup_parser(parser);
            return ASTHRA_TEST_FAIL;
        }

        // Verify type_args field is populated for generic types
        if (strstr(generic_expressions[i], "<") != NULL) {
            if (!ASTHRA_TEST_ASSERT(context, result->data.associated_func_call.type_args != NULL,
                                    "Generic type should have type_args populated: %s",
                                    generic_expressions[i])) {
                ast_free_node(result);
                cleanup_parser(parser);
                return ASTHRA_TEST_FAIL;
            }
        }

        ast_free_node(result);
        cleanup_parser(parser);
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_postfix_vs_primary_disambiguation(AsthraTestContext *context) {
    // Test that the parser correctly distinguishes between valid primary :: usage
    // and invalid postfix :: usage

    // This should parse as: (Point::new)(3.0, 4.0) - function call on associated function
    const char *valid_source = "Point::new(3.0, 4.0)";
    Parser *valid_parser = create_test_parser(valid_source);

    if (!ASTHRA_TEST_ASSERT(context, valid_parser != NULL, "Valid parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *valid_result = parser_parse_expression(valid_parser);

    if (!ASTHRA_TEST_ASSERT(context, valid_result != NULL, "Valid expression should parse")) {
        cleanup_parser(valid_parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, valid_result->type == AST_ASSOCIATED_FUNC_CALL,
                            "Should be AST_ASSOCIATED_FUNC_CALL")) {
        ast_free_node(valid_result);
        cleanup_parser(valid_parser);
        return ASTHRA_TEST_FAIL;
    }

    ast_free_node(valid_result);
    cleanup_parser(valid_parser);

    // This should fail: trying to use :: as postfix on function call result
    const char *invalid_source = "get_point()::distance()";
    Parser *invalid_parser = create_test_parser(invalid_source);

    if (!ASTHRA_TEST_ASSERT(context, invalid_parser != NULL, "Invalid parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *invalid_result = parser_parse_expression(invalid_parser);

    if (!ASTHRA_TEST_ASSERT(context, invalid_result == NULL,
                            "Invalid postfix :: should be rejected")) {
        if (invalid_result)
            ast_free_node(invalid_result);
        cleanup_parser(invalid_parser);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_parser(invalid_parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST EXECUTION
// =============================================================================

int main(void) {
    printf("Postfix Expression Ambiguity Tests (v1.20)\n");
    printf("==========================================\n\n");

    AsthraTestStatistics *stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    struct {
        const char *name;
        AsthraTestResult (*test_func)(AsthraTestContext *);
    } tests[] = {
        {"Test reject postfix double colon", test_reject_postfix_double_colon},
        {"Test valid associated function calls", test_valid_associated_function_calls},
        {"Test generic type associated functions", test_generic_type_associated_functions},
        {"Test postfix vs primary disambiguation", test_postfix_vs_primary_disambiguation},
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < test_count; i++) {
        AsthraTestMetadata metadata = pub_impl_self_base_metadata;
        metadata.name = tests[i].name;
        metadata.line = __LINE__;

        AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
        if (!context) {
            printf("❌ Failed to create test context for '%s'\n", tests[i].name);
            overall_result = ASTHRA_TEST_FAIL;
            continue;
        }

        asthra_test_context_start(context);
        AsthraTestResult result = tests[i].test_func(context);
        asthra_test_context_end(context, result);

        if (result == ASTHRA_TEST_PASS) {
            printf("✅ %s: PASS\n", tests[i].name);
        } else {
            printf("❌ %s: FAIL", tests[i].name);
            if (context->error_message) {
                printf(" - %s", context->error_message);
            }
            printf("\n");
            overall_result = ASTHRA_TEST_FAIL;
        }

        asthra_test_context_destroy(context);
    }

    printf("\n=== Postfix Expression Test Summary ===\n");
    printf("Total tests: %zu\n", test_count);
    printf("Assertions checked: %llu\n", stats->assertions_checked);
    printf("Assertions failed: %llu\n", stats->assertions_failed);

    asthra_test_statistics_destroy(stats);

    if (overall_result == ASTHRA_TEST_PASS) {
        printf("✅ All postfix expression tests passed!\n");
        return 0;
    } else {
        printf("❌ Some postfix expression tests failed!\n");
        return 1;
    }
}
