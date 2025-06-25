/**
 * Asthra Programming Language Compiler
 * Operator utilities and precedence handling
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "parser.h"

// =============================================================================
// OPERATOR UTILITIES
// =============================================================================

int get_binary_precedence(TokenType token_type) {
    switch (token_type) {
    case TOKEN_LOGICAL_OR:
        return 1;
    case TOKEN_LOGICAL_AND:
        return 2;
    case TOKEN_BITWISE_OR:
        return 3;
    case TOKEN_BITWISE_XOR:
        return 4;
    case TOKEN_BITWISE_AND:
        return 5;
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        return 6;
    case TOKEN_LESS_THAN:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER_THAN:
    case TOKEN_GREATER_EQUAL:
        return 7;
    case TOKEN_LEFT_SHIFT:
    case TOKEN_RIGHT_SHIFT:
        return 8;
    case TOKEN_PLUS:
    case TOKEN_MINUS:
        return 9;
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
    case TOKEN_MODULO:
        return 10;
    default:
        return 0; // Not a binary operator
    }
}

BinaryOperator token_to_binary_op(TokenType token_type) {
    switch (token_type) {
    case TOKEN_PLUS:
        return BINOP_ADD;
    case TOKEN_MINUS:
        return BINOP_SUB;
    case TOKEN_MULTIPLY:
        return BINOP_MUL;
    case TOKEN_DIVIDE:
        return BINOP_DIV;
    case TOKEN_MODULO:
        return BINOP_MOD;
    case TOKEN_EQUAL:
        return BINOP_EQ;
    case TOKEN_NOT_EQUAL:
        return BINOP_NE;
    case TOKEN_LESS_THAN:
        return BINOP_LT;
    case TOKEN_LESS_EQUAL:
        return BINOP_LE;
    case TOKEN_GREATER_THAN:
        return BINOP_GT;
    case TOKEN_GREATER_EQUAL:
        return BINOP_GE;
    case TOKEN_LOGICAL_AND:
        return BINOP_AND;
    case TOKEN_LOGICAL_OR:
        return BINOP_OR;
    case TOKEN_BITWISE_AND:
        return BINOP_BITWISE_AND;
    case TOKEN_BITWISE_OR:
        return BINOP_BITWISE_OR;
    case TOKEN_BITWISE_XOR:
        return BINOP_BITWISE_XOR;
    case TOKEN_LEFT_SHIFT:
        return BINOP_LSHIFT;
    case TOKEN_RIGHT_SHIFT:
        return BINOP_RSHIFT;
    default:
        return BINOP_ADD; // Default, should not happen
    }
}

UnaryOperator token_to_unary_op(TokenType token_type) {
    switch (token_type) {
    case TOKEN_MINUS:
        return UNOP_MINUS;
    case TOKEN_LOGICAL_NOT:
        return UNOP_NOT;
    case TOKEN_BITWISE_NOT:
        return UNOP_BITWISE_NOT;
    case TOKEN_MULTIPLY:
        return UNOP_DEREF;
    case TOKEN_BITWISE_AND:
        return UNOP_ADDRESS_OF;
    case TOKEN_SIZEOF:
        return UNOP_SIZEOF;
    default:
        return UNOP_MINUS; // Default, should not happen
    }
}

bool is_assignment_operator(TokenType token_type) {
    return token_type == TOKEN_ASSIGN;
}

bool is_comparison_operator(TokenType token_type) {
    // Combined function for both equality and relational operators
    return is_equality_operator(token_type) || is_relational_operator(token_type);
}

bool is_arithmetic_operator(TokenType token_type) {
    switch (token_type) {
    case TOKEN_PLUS:
    case TOKEN_MINUS:
    case TOKEN_MULTIPLY:
    case TOKEN_DIVIDE:
    case TOKEN_MODULO:
        return true;
    default:
        return false;
    }
}

bool is_logical_operator(TokenType token_type) {
    switch (token_type) {
    case TOKEN_LOGICAL_AND:
    case TOKEN_LOGICAL_OR:
    case TOKEN_LOGICAL_NOT:
        return true;
    default:
        return false;
    }
}

bool is_bitwise_operator(TokenType token_type) {
    switch (token_type) {
    case TOKEN_BITWISE_AND:
    case TOKEN_BITWISE_OR:
    case TOKEN_BITWISE_XOR:
    case TOKEN_BITWISE_NOT:
        return true;
    default:
        return false;
    }
}

bool is_shift_operator(TokenType token_type) {
    switch (token_type) {
    case TOKEN_LEFT_SHIFT:
    case TOKEN_RIGHT_SHIFT:
        return true;
    default:
        return false;
    }
}

bool is_equality_operator(TokenType token_type) {
    switch (token_type) {
    case TOKEN_EQUAL:
    case TOKEN_NOT_EQUAL:
        return true;
    default:
        return false;
    }
}

bool is_relational_operator(TokenType token_type) {
    switch (token_type) {
    case TOKEN_LESS_THAN:
    case TOKEN_LESS_EQUAL:
    case TOKEN_GREATER_THAN:
    case TOKEN_GREATER_EQUAL:
        return true;
    default:
        return false;
    }
}
