/**
 * Asthra Programming Language
 * Test Framework - Compiler-Specific Assertions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Specialized assertion functions for compiler structures (tokens, AST, etc.)
 */

#ifndef ASTHRA_TEST_ASSERTIONS_COMPILER_H
#define ASTHRA_TEST_ASSERTIONS_COMPILER_H

#include "ast_node.h"
#include "ast_types.h"
#include "parser_token.h"
#include "test_assertions_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FORWARD DECLARATIONS FOR TEST STRUCTURES
// =============================================================================

/**
 * Structure for expected token values in tests
 */
typedef struct {
    TokenType type;
    const char *value;
    size_t line;
    size_t column;
} TokenExpectation;

/**
 * Structure for expected AST node values in tests
 */
typedef struct {
    ASTNodeType type;
    size_t child_count;
    const char *name;         // For identifier nodes and other named nodes
    const char *string_value; // For string literal nodes
    int int_value;            // For integer literal nodes
} AstExpectation;

// =============================================================================
// COMPILER-SPECIFIC ASSERTION FUNCTIONS
// =============================================================================

/**
 * Compare a token with expected values
 * @param context Test context
 * @param token The actual token
 * @param expected The expected token values
 * @return true if token matches expectations
 */
bool assert_token_equals(AsthraTestContext *context, const Token *token,
                         const TokenExpectation *expected);

/**
 * Validate AST node structure
 * @param context Test context
 * @param node The actual AST node
 * @param expected The expected AST structure
 * @return true if AST matches expectations
 */
bool assert_ast_structure(AsthraTestContext *context, const ASTNode *node,
                          const AstExpectation *expected);

/**
 * Compare token values for equality
 * @param token1 First token
 * @param token2 Second token
 * @return true if tokens are equal
 */
bool tokens_equal(const Token *token1, const Token *token2);

/**
 * Check if AST node has expected structure
 * @param node The AST node to check
 * @param expected_type Expected node type
 * @param expected_children Expected number of children
 * @return true if structure matches
 */
bool ast_has_structure(const ASTNode *node, ASTNodeType expected_type, size_t expected_children);

// =============================================================================
// ENHANCED ASSERTION MACROS
// =============================================================================

/**
 * Assert that a token matches the expected type and value
 */
#define ASSERT_TOKEN_EQUALS(context, token, expected)                                              \
    do {                                                                                           \
        if (!assert_token_equals(context, token, expected)) {                                      \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that a token has the expected type
 */
#define ASSERT_TOKEN_TYPE(context, token, expected_type)                                           \
    do {                                                                                           \
        if (!asthra_test_assert_int_eq(context, (token)->type, expected_type,                      \
                                       "Token type mismatch: expected %d, got %d", expected_type,  \
                                       (token)->type)) {                                           \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that an AST node has the expected type and structure
 */
#define ASSERT_AST_STRUCTURE(context, node, expected)                                              \
    do {                                                                                           \
        if (!assert_ast_structure(context, node, expected)) {                                      \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that an AST node has the expected type
 */
#define ASSERT_AST_NODE_TYPE(context, node, expected_type)                                         \
    do {                                                                                           \
        if (!asthra_test_assert_int_eq(context, (node)->type, expected_type,                       \
                                       "AST node type mismatch: expected %d, got %d",              \
                                       expected_type, (node)->type)) {                             \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that a symbol exists in the semantic analyzer
 */
#define ASSERT_SYMBOL_EXISTS(context, analyzer, name)                                              \
    do {                                                                                           \
        if (!asthra_test_assert_pointer(context, semantic_resolve_identifier(analyzer, name),      \
                                        "Symbol not found: %s", name)) {                           \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that two types are compatible
 */
#define ASSERT_TYPE_COMPATIBLE(context, analyzer, type1, type2)                                    \
    do {                                                                                           \
        if (!asthra_test_assert_bool(context,                                                      \
                                     semantic_check_type_compatibility(analyzer, type1, type2),    \
                                     "Types not compatible")) {                                    \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that the parser has the expected number of errors
 */
#define ASSERT_ERROR_COUNT(context, parser, expected_count)                                        \
    do {                                                                                           \
        if (!asthra_test_assert_int_eq(context, parser_get_error_count(parser), expected_count,    \
                                       "Error count mismatch: expected %d, got %d",                \
                                       expected_count, parser_get_error_count(parser))) {          \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

/**
 * Assert that there are no semantic errors
 */
#define ASSERT_NO_SEMANTIC_ERRORS(context, analyzer)                                               \
    do {                                                                                           \
        if (!asthra_test_assert_int_eq(context, (analyzer)->error_count, 0,                        \
                                       "Unexpected semantic errors: %d",                           \
                                       (analyzer)->error_count)) {                                 \
            return ASTHRA_TEST_FAIL;                                                               \
        }                                                                                          \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_ASSERTIONS_COMPILER_H
