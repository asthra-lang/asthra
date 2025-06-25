/**
 * Asthra Programming Language Compiler
 * Expression Parsing Helper Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Helper functions for parsing expression strings in tests
 */

#ifndef EXPRESSION_PARSING_HELPER_H
#define EXPRESSION_PARSING_HELPER_H

#include "ast.h"
#include "semantic_analyzer.h"

/**
 * Parse an expression string into an AST node
 * This is a wrapper around parse_expression_string that handles
 * the ParseResult and extracts just the AST node.
 *
 * @param expr_str The expression string to parse
 * @return The parsed AST node, or NULL on failure
 */
ASTNode *parse_test_expression_string(const char *expr_str);

/**
 * Parse and analyze an expression with proper context
 * This creates a minimal program with variable declarations,
 * runs semantic analysis, and returns the analyzed expression.
 *
 * @param expr_str The expression string to parse
 * @param analyzer The semantic analyzer to use (must be initialized)
 * @return The parsed and analyzed AST node, or NULL on failure
 */
ASTNode *parse_and_analyze_expression(const char *expr_str, SemanticAnalyzer *analyzer);

#endif // EXPRESSION_PARSING_HELPER_H