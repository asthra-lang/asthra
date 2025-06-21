/**
 * Asthra Programming Language
 * Test Framework - Assertions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Type-safe assertion macros and functions using _Generic
 */

#ifndef ASTHRA_TEST_ASSERTIONS_H
#define ASTHRA_TEST_ASSERTIONS_H

#include "test_framework.h"
#include "lexer_test_utils.h"
#include "parser_test_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE-SPECIFIC ASSERTION IMPLEMENTATIONS
// =============================================================================

// Basic type assertions
bool asthra_test_assert_bool(AsthraTestContext *context, bool condition, const char *message, ...);
bool asthra_test_assert_int(AsthraTestContext *context, int value, const char *message, ...);
bool asthra_test_assert_long(AsthraTestContext *context, long value, const char *message, ...);
bool asthra_test_assert_size(AsthraTestContext *context, size_t value, const char *message, ...);
bool asthra_test_assert_string(AsthraTestContext *context, const char *value, const char *message, ...);
bool asthra_test_assert_pointer(AsthraTestContext *context, void *value, const char *message, ...);
bool asthra_test_assert_not_null(AsthraTestContext *context, void *value, const char *message, ...);
bool asthra_test_assert_null(AsthraTestContext *context, void *value, const char *message, ...);
bool asthra_test_assert_str_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...);
bool asthra_test_assert_generic(AsthraTestContext *context, void *value, const char *message, ...);

// Equality assertions
bool asthra_test_assert_bool_eq(AsthraTestContext *context, bool actual, bool expected, const char *message, ...);
bool asthra_test_assert_int_eq(AsthraTestContext *context, int actual, int expected, const char *message, ...);
bool asthra_test_assert_long_eq(AsthraTestContext *context, long actual, long expected, const char *message, ...);
bool asthra_test_assert_size_eq(AsthraTestContext *context, size_t actual, size_t expected, const char *message, ...);
bool asthra_test_assert_string_eq(AsthraTestContext *context, const char *actual, const char *expected, const char *message, ...);
bool asthra_test_assert_pointer_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);
bool asthra_test_assert_generic_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);

// Range assertions
bool asthra_test_assert_int_range(AsthraTestContext *context, int value, int min, int max, const char *message, ...);
bool asthra_test_assert_long_range(AsthraTestContext *context, long value, long min, long max, const char *message, ...);
bool asthra_test_assert_size_range(AsthraTestContext *context, size_t value, size_t min, size_t max, const char *message, ...);
bool asthra_test_assert_double_range(AsthraTestContext *context, double value, double min, double max, const char *message, ...);
bool asthra_test_assert_generic_range(AsthraTestContext *context, void *value, void *min, void *max, const char *message, ...);

// Additional missing functions that tests are using
bool asthra_test_assert_ptr_eq(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);
bool asthra_test_assert_ptr_ne(AsthraTestContext *context, void *actual, void *expected, const char *message, ...);
bool asthra_test_assert_int_gt(AsthraTestContext *context, int actual, int min, const char *message, ...);
void asthra_test_log(AsthraTestContext *context, const char *message, ...);

// =============================================================================
// COMPILER-SPECIFIC ASSERTION HELPER FUNCTIONS
// =============================================================================

/**
 * Compare a token with expected values
 * @param context Test context
 * @param token The actual token
 * @param expected The expected token values
 * @return true if token matches expectations
 */
bool assert_token_equals(AsthraTestContext* context, const Token* token, const TokenExpectation* expected);

/**
 * Validate AST node structure
 * @param context Test context
 * @param node The actual AST node
 * @param expected The expected AST structure
 * @return true if AST matches expectations
 */
bool assert_ast_structure(AsthraTestContext* context, const ASTNode* node, const AstExpectation* expected);

/**
 * Compare token values for equality
 * @param token1 First token
 * @param token2 Second token
 * @return true if tokens are equal
 */
bool tokens_equal(const Token* token1, const Token* token2);

/**
 * Check if AST node has expected structure
 * @param node The AST node to check
 * @param expected_type Expected node type
 * @param expected_children Expected number of children
 * @return true if structure matches
 */
bool ast_has_structure(const ASTNode* node, ASTNodeType expected_type, size_t expected_children);

// =============================================================================
// ENHANCED ASSERTION MACROS
// =============================================================================

/**
 * Assert that a token matches the expected type and value
 */
#define ASSERT_TOKEN_EQUALS(context, token, expected) \
    do { \
        if (!assert_token_equals(context, token, expected)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that a token has the expected type
 */
#define ASSERT_TOKEN_TYPE(context, token, expected_type) \
    do { \
        if (!asthra_test_assert_int_eq(context, (token)->type, expected_type, \
                                      "Token type mismatch: expected %d, got %d", \
                                      expected_type, (token)->type)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that an AST node has the expected type and structure
 */
#define ASSERT_AST_STRUCTURE(context, node, expected) \
    do { \
        if (!assert_ast_structure(context, node, expected)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that an AST node has the expected type
 */
#define ASSERT_AST_NODE_TYPE(context, node, expected_type) \
    do { \
        if (!asthra_test_assert_int_eq(context, (node)->type, expected_type, \
                                      "AST node type mismatch: expected %d, got %d", \
                                      expected_type, (node)->type)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that a symbol exists in the semantic analyzer
 */
#define ASSERT_SYMBOL_EXISTS(context, analyzer, name) \
    do { \
        if (!asthra_test_assert_pointer(context, semantic_resolve_identifier(analyzer, name), \
                                       "Symbol not found: %s", name)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that two types are compatible
 */
#define ASSERT_TYPE_COMPATIBLE(context, analyzer, type1, type2) \
    do { \
        if (!asthra_test_assert_bool(context, semantic_check_type_compatibility(analyzer, type1, type2), \
                                    "Types not compatible")) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that the parser has the expected number of errors
 */
#define ASSERT_ERROR_COUNT(context, parser, expected_count) \
    do { \
        if (!asthra_test_assert_int_eq(context, parser_get_error_count(parser), expected_count, \
                                      "Error count mismatch: expected %d, got %d", \
                                      expected_count, parser_get_error_count(parser))) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

/**
 * Assert that there are no semantic errors
 */
#define ASSERT_NO_SEMANTIC_ERRORS(context, analyzer) \
    do { \
        if (!asthra_test_assert_int_eq(context, (analyzer)->error_count, 0, \
                                      "Unexpected semantic errors: %d", (analyzer)->error_count)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

// =============================================================================
// BASIC ASSERTION MACROS
// =============================================================================

/**
 * Assert that two integers are equal
 */
#define ASTHRA_TEST_ASSERT_INT_EQ(context, actual, expected, message) \
    do { \
        if (!asthra_test_assert_int_eq(context, actual, expected, message)) { \
            return ASTHRA_TEST_FAIL; \
        } \
    } while (0)

// Macros ASTHRA_TEST_ASSERT_NOT_NULL and ASTHRA_TEST_ASSERT_NULL 
// are defined in test_framework.h to avoid redefinition conflicts

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_H
