/**
 * Asthra Programming Language Compiler - Expression Grammar Productions
 * Expression parsing with precedence levels and operators
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_GRAMMAR_EXPRESSIONS_H
#define ASTHRA_GRAMMAR_EXPRESSIONS_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// EXPRESSION GRAMMAR PRODUCTIONS
// =============================================================================

// Expression precedence hierarchy
ASTNode *parse_expr(Parser *parser);
ASTNode *parse_logic_or(Parser *parser);
ASTNode *parse_logic_and(Parser *parser);
ASTNode *parse_bitwise_or(Parser *parser);
ASTNode *parse_bitwise_xor(Parser *parser);
ASTNode *parse_bitwise_and(Parser *parser);
ASTNode *parse_equality(Parser *parser);
ASTNode *parse_relational(Parser *parser);
ASTNode *parse_shift(Parser *parser);
ASTNode *parse_add(Parser *parser);
ASTNode *parse_mult(Parser *parser);
ASTNode *parse_unary(Parser *parser);
ASTNode *parse_unary_prefix(Parser *parser);
ASTNode *parse_pointer_prefix_or_postfix(Parser *parser);
ASTNode *parse_postfix_expr(Parser *parser);
ASTNode *parse_primary(Parser *parser);

// Special expressions
ASTNode *parse_parenthesized_expr(Parser *parser);
ASTNode *parse_sizeof(Parser *parser);
ASTNode *parse_await_expr(Parser *parser);
ASTNode *parse_recv_expr(Parser *parser);
ASTNode *parse_struct_literal_with_name(Parser *parser, char *struct_name, SourceLocation start_loc);
ASTNode *parse_struct_literal_with_name_and_type_args(Parser *parser, char *struct_name, ASTNodeList *type_args, SourceLocation start_loc);

// Helper functions
bool is_expression_start(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GRAMMAR_EXPRESSIONS_H 
