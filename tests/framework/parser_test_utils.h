/**
 * Asthra Programming Language
 * Parser Test Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test utility functions for parser testing
 */

#ifndef ASTHRA_PARSER_TEST_UTILS_H
#define ASTHRA_PARSER_TEST_UTILS_H

#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST HELPER STRUCTURES
// =============================================================================

/**
 * Test parser configuration
 */
typedef struct {
    const char* source_code;
    const char* filename;
    bool enable_error_recovery;
    bool enable_ast_validation;
    size_t max_errors;
} TestParserConfig;

/**
 * AST node expectation for testing
 */
typedef struct {
    ASTNodeType type;
    const char* name;
    size_t child_count;
    bool has_annotations;
} AstExpectation;

// =============================================================================
// PARSER TEST UTILITIES
// =============================================================================

/**
 * Create a test parser with the given source code
 * @param source The source code to parse
 * @return A new parser state for testing
 */
Parser* create_test_parser(const char* source);

/**
 * Create a test parser with custom configuration
 * @param config Test parser configuration
 * @return A new parser state for testing
 */
Parser* create_test_parser_with_config(const TestParserConfig* config);

/**
 * Destroy a test parser and clean up resources
 * @param parser The parser to destroy
 */
void destroy_test_parser(Parser* parser);

/**
 * Parse source and return the AST
 * @param source The source code to parse
 * @param filename The filename for error reporting
 * @return The root AST node (caller must free)
 */
ASTNode* parse_test_source(const char* source, const char* filename);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_TEST_UTILS_H
