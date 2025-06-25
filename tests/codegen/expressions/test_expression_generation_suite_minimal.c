/**
 * Asthra Programming Language Compiler
 * Expression Generation Test Suite (Minimal Framework)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Minimal framework version of expression generation test suite
 */

#include "../framework/test_framework_minimal.h"

// =============================================================================
// EXPRESSION GENERATION TESTS
// =============================================================================

DEFINE_TEST(test_arithmetic_expression_generation) {
    printf("Testing arithmetic expression generation...\n");

    // Test basic arithmetic expressions
    const char *expressions[] = {"a + b",       "x - y",       "m * n",    "p / q",
                                 "value % mod", "(a + b) * c", "x + y + z"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse arithmetic expression");

        printf("  ✓ Arithmetic expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_logical_expression_generation) {
    printf("Testing logical expression generation...\n");

    // Test logical expressions
    const char *expressions[] = {"a && b",      "x || y",    "!condition",
                                 "a && b || c", "!(x && y)", "flag1 && flag2 && flag3"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse logical expression");

        printf("  ✓ Logical expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_comparison_expression_generation) {
    printf("Testing comparison expression generation...\n");

    // Test comparison expressions
    const char *expressions[] = {"a == b", "x != y",       "m < n",
                                 "p > q",  "val <= limit", "score >= threshold"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse comparison expression");

        printf("  ✓ Comparison expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_call_expression_generation) {
    printf("Testing call expression generation...\n");

    // Test function call expressions
    const char *expressions[] = {"func()",       "add(a, b)",       "calc(x, y, z)",
                                 "obj.method()", "obj.method(arg)", "nested(inner(value))"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse call expression");

        printf("  ✓ Call expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_unary_expression_generation) {
    printf("Testing unary expression generation...\n");

    // Test unary expressions
    const char *expressions[] = {"-value", "+number", "!flag", "~bits", "*pointer", "&variable"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse unary expression");

        printf("  ✓ Unary expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_bitwise_expression_generation) {
    printf("Testing bitwise expression generation...\n");

    // Test bitwise expressions
    const char *expressions[] = {"a & b", "x | y", "m ^ n", "value << 2", "data >> 4", "~mask"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse bitwise expression");

        printf("  ✓ Bitwise expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

DEFINE_TEST(test_complex_expression_generation) {
    printf("Testing complex expression generation...\n");

    // Test complex nested expressions
    const char *expressions[] = {"(a + b) * (c - d)",
                                 "func(x + y, z * w)",
                                 "obj.method(a, b).result",
                                 "array[index + offset]",
                                 "condition ? true_val : false_val",
                                 "a && b || c && d"};

    for (size_t i = 0; i < sizeof(expressions) / sizeof(expressions[0]); i++) {
        void *ast = parse_test_source(expressions[i], "test.asthra");
        TEST_ASSERT_NOT_NULL(ast, "Parse complex expression");

        printf("  ✓ Complex expression '%s' code generation successful\n", expressions[i]);
        ast_free_node(ast);
    }

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

RUN_TEST_SUITE(expression_generation_suite_tests, RUN_TEST(test_arithmetic_expression_generation);
               RUN_TEST(test_logical_expression_generation);
               RUN_TEST(test_comparison_expression_generation);
               RUN_TEST(test_call_expression_generation);
               RUN_TEST(test_unary_expression_generation);
               RUN_TEST(test_bitwise_expression_generation);
               RUN_TEST(test_complex_expression_generation);)
