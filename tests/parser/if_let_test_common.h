/**
 * Asthra Programming Language
 * If-Let Statement Testing - Common Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared test framework and utilities for if-let statement validation
 */

#ifndef IF_LET_TEST_COMMON_H
#define IF_LET_TEST_COMMON_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Core parser and AST includes
#include "ast_node.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include "grammar_statements.h"
#include "lexer.h"
#include "parser_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST FRAMEWORK MACROS
// =============================================================================

extern size_t tests_passed;
extern size_t tests_total;
extern size_t tests_failed;

#define TEST_ASSERT(condition, message)                                                            \
    do {                                                                                           \
        tests_total++;                                                                             \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("✓ %s\n", message);                                                             \
        } else {                                                                                   \
            tests_failed++;                                                                        \
            printf("✗ %s\n", message);                                                             \
        }                                                                                          \
    } while (0)

#define TEST_SECTION(name)                                                                         \
    do {                                                                                           \
        printf("\n=== %s ===\n", name);                                                            \
    } while (0)

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create a simple parser from source code
 * @param source Source code string to parse
 * @return Parser instance or NULL on failure
 */
Parser *create_test_parser(const char *source);

/**
 * Cleanup parser and associated resources
 * @param parser Parser instance to destroy
 */
void destroy_test_parser(Parser *parser);

/**
 * Validate AST node structure for if-let statements
 * @param node AST node to validate
 * @return true if node is a valid if-let statement, false otherwise
 */
bool validate_if_let_ast(ASTNode *node);

/**
 * Initialize test framework counters
 */
void init_if_let_test_framework(void);

/**
 * Print test summary and return exit code
 * @return 0 if all tests passed, 1 otherwise
 */
int print_if_let_test_summary(void);

#ifdef __cplusplus
}
#endif

#endif // IF_LET_TEST_COMMON_H
