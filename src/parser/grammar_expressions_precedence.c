/**
 * Asthra Programming Language Compiler - Expression Precedence Hierarchy
 * Binary expression parsing with operator precedence
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_expressions.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// EXPRESSION PRECEDENCE HIERARCHY
// =============================================================================

ASTNode *parse_expr(Parser *parser) {
    if (!parser) return NULL;
    return parse_logic_or(parser);
}

ASTNode *parse_logic_or(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_logic_and(parser);
    if (!left) return NULL;
    
    while (match_token(parser, TOKEN_LOGICAL_OR)) {
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_logic_and(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = BINOP_OR;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_logic_and(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_bitwise_or(parser);
    if (!left) return NULL;
    
    while (match_token(parser, TOKEN_LOGICAL_AND)) {
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_bitwise_or(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = BINOP_AND;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_bitwise_or(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_bitwise_xor(parser);
    if (!left) return NULL;
    
    while (match_token(parser, TOKEN_BITWISE_OR)) {
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_bitwise_xor(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = BINOP_BITWISE_OR;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_bitwise_xor(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_bitwise_and(parser);
    if (!left) return NULL;
    
    while (match_token(parser, TOKEN_BITWISE_XOR)) {
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_bitwise_and(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = BINOP_BITWISE_XOR;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_bitwise_and(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_equality(parser);
    if (!left) return NULL;
    
    while (match_token(parser, TOKEN_BITWISE_AND)) {
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_equality(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = BINOP_BITWISE_AND;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_equality(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_relational(parser);
    if (!left) return NULL;
    
    while (true) {
        BinaryOperator op;
        if (match_token(parser, TOKEN_EQUAL)) {
            op = BINOP_EQ;
        } else if (match_token(parser, TOKEN_NOT_EQUAL)) {
            op = BINOP_NE;
        } else {
            break;
        }
        
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_relational(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = op;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_relational(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_shift(parser);
    if (!left) return NULL;
    
    while (true) {
        BinaryOperator op;
        if (match_token(parser, TOKEN_LESS_THAN)) {
            op = BINOP_LT;
        } else if (match_token(parser, TOKEN_LESS_EQUAL)) {
            op = BINOP_LE;
        } else if (match_token(parser, TOKEN_GREATER_THAN)) {
            op = BINOP_GT;
        } else if (match_token(parser, TOKEN_GREATER_EQUAL)) {
            op = BINOP_GE;
        } else {
            break;
        }
        
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_shift(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = op;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_shift(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_add(parser);
    if (!left) return NULL;
    
    while (true) {
        BinaryOperator op;
        if (match_token(parser, TOKEN_LEFT_SHIFT)) {
            op = BINOP_LSHIFT;
        } else if (match_token(parser, TOKEN_RIGHT_SHIFT)) {
            op = BINOP_RSHIFT;
        } else {
            break;
        }
        
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_add(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = op;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}



ASTNode *parse_add(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_mult(parser);
    if (!left) return NULL;
    
    while (true) {
        BinaryOperator op;
        if (match_token(parser, TOKEN_PLUS)) {
            op = BINOP_ADD;
        } else if (match_token(parser, TOKEN_MINUS)) {
            op = BINOP_SUB;
        } else {
            break;
        }
        
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_mult(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = op;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
}

ASTNode *parse_mult(Parser *parser) {
    if (!parser) return NULL;
    
    ASTNode *left = parse_unary(parser);
    if (!left) return NULL;
    
    while (true) {
        BinaryOperator op;
        if (match_token(parser, TOKEN_MULTIPLY)) {
            op = BINOP_MUL;
        } else if (match_token(parser, TOKEN_DIVIDE)) {
            op = BINOP_DIV;
        } else if (match_token(parser, TOKEN_MODULO)) {
            op = BINOP_MOD;
        } else {
            break;
        }
        
        SourceLocation op_loc = parser->current_token.location;
        advance_token(parser);
        
        ASTNode *right = parse_unary(parser);
        if (!right) {
            ast_free_node(left);
            return NULL;
        }
        
        ASTNode *binary = ast_create_node(AST_BINARY_EXPR, op_loc);
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        binary->data.binary_expr.operator = op;
        binary->data.binary_expr.left = left;
        binary->data.binary_expr.right = right;
        
        left = binary;
    }
    
    return left;
} 
