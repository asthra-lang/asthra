/**
 * Asthra Programming Language Compiler
 * Parser operator handling and precedence functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_OPERATORS_H
#define ASTHRA_PARSER_OPERATORS_H

#include "ast.h"
#include "lexer.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// OPERATOR AND PRECEDENCE FUNCTIONS
// =============================================================================

// Precedence and operator utilities with validation
int get_binary_precedence(TokenType token_type);
BinaryOperator token_to_binary_op(TokenType token_type);
UnaryOperator token_to_unary_op(TokenType token_type);
bool is_assignment_operator(TokenType token_type);
bool is_comparison_operator(TokenType token_type);
bool is_arithmetic_operator(TokenType token_type);
bool is_logical_operator(TokenType token_type);
bool is_bitwise_operator(TokenType token_type);
bool is_shift_operator(TokenType token_type);
bool is_equality_operator(TokenType token_type);
bool is_relational_operator(TokenType token_type);

// Enhanced operator validation
bool is_valid_binary_operator_combination(BinaryOperator left_op, BinaryOperator right_op);
bool is_valid_unary_operator_context(UnaryOperator op, ASTNodeType operand_type);

// =============================================================================
// COMPILE-TIME VALIDATION MACROS
// =============================================================================

// C17 static assertions for parser compile-time validation
#define PARSER_VALIDATE_TOKEN_TYPE(type)                                                           \
    _Static_assert((type) >= TOKEN_EOF && (type) < TOKEN_ERROR, "Token type must be valid")

#define PARSER_VALIDATE_AST_NODE_TYPE(type)                                                        \
    _Static_assert((type) >= AST_PROGRAM && (type) < AST_NODE_TYPE_COUNT,                          \
                   "AST node type must be valid")

#define PARSER_VALIDATE_BINARY_OPERATOR(op)                                                        \
    _Static_assert((op) >= BINOP_ADD && (op) < BINOP_COUNT, "Binary operator must be valid")

#define PARSER_VALIDATE_UNARY_OPERATOR(op)                                                         \
    _Static_assert((op) >= UNOP_MINUS && (op) < UNOP_COUNT, "Unary operator must be valid")

// Compile-time grammar validation
#define PARSER_VALIDATE_PRECEDENCE_TABLE()                                                         \
    _Static_assert(sizeof(precedence_table) / sizeof(precedence_table[0]) == BINOP_COUNT,          \
                   "Precedence table must cover all binary operators")

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_OPERATORS_H
