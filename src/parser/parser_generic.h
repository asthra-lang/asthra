/**
 * Asthra Programming Language Compiler
 * C17 generic operations and type-safe parser macros
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_GENERIC_H
#define ASTHRA_PARSER_GENERIC_H

#include "ast.h"
#include "lexer.h"
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// C17 GENERIC TYPE-SAFE PARSER OPERATIONS
// =============================================================================

// C17 _Generic for type-safe parsing operations
#define parse_node(parser, type) _Generic((type), \
    ASTNodeType: parse_node_by_type_impl, \
    const char*: parse_node_by_name_impl \
)(parser, type)

#define expect_token_safe(parser, expected) _Generic((expected), \
    TokenType: expect_token_impl, \
    const char*: expect_token_by_name_impl \
)(parser, expected)

#define consume_token_safe(parser, expected) _Generic((expected), \
    TokenType: consume_token_impl, \
    const char*: consume_token_by_name_impl \
)(parser, expected)

// Type-safe error reporting with _Generic
#define report_parse_error(parser, ...) _Generic((parser), \
    Parser*: report_parse_error_impl \
)(parser, __VA_ARGS__)

#define report_parse_warning(parser, ...) _Generic((parser), \
    Parser*: report_parse_warning_impl \
)(parser, __VA_ARGS__)

// Type-safe AST creation with compound literals
#define CREATE_BINARY_EXPR_SAFE(parser, op, left, right) \
    create_ast_node_safe(parser, AST_BINARY_EXPR, \
        (ASTNodeData){ .binary_expr = { .operator = (op), .left = (left), .right = (right) } })

#define CREATE_UNARY_EXPR_SAFE(parser, op, operand) \
    create_ast_node_safe(parser, AST_UNARY_EXPR, \
        (ASTNodeData){ .unary_expr = { .operator = (op), .operand = (operand) } })

#define CREATE_IDENTIFIER_SAFE(parser, name) \
    create_ast_node_safe(parser, AST_IDENTIFIER, \
        (ASTNodeData){ .identifier = { .name = (name) } })

// =============================================================================
// IMPLEMENTATION FUNCTIONS FOR _Generic MACROS
// =============================================================================

// Implementation functions for type-safe parsing (not for direct use)
ASTNode *parse_node_by_type_impl(Parser *parser, ASTNodeType type);
ASTNode *parse_node_by_name_impl(Parser *parser, const char *type_name);

bool expect_token_impl(Parser *parser, TokenType expected);
bool expect_token_by_name_impl(Parser *parser, const char *token_name);
bool consume_token_impl(Parser *parser, TokenType expected);
bool consume_token_by_name_impl(Parser *parser, const char *token_name);

void report_parse_error_impl(Parser *parser, const char *format, ...);
void report_parse_warning_impl(Parser *parser, const char *format, ...);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_GENERIC_H 
