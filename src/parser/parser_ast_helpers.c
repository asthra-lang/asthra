/**
 * Asthra Programming Language Compiler
 * AST node creation helper functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// AST NODE CREATION HELPERS
// =============================================================================

ASTNode *create_binary_expr(Parser *parser, BinaryOperator op, ASTNode *left, ASTNode *right) {
    if (!parser || !left || !right) return NULL;
    
    ASTNode *node = ast_create_node(AST_BINARY_EXPR, left->location);
    if (!node) return NULL;
    
    node->data.binary_expr.operator = op;
    node->data.binary_expr.left = left;
    node->data.binary_expr.right = right;
    
    return node;
}

ASTNode *create_unary_expr(Parser *parser, UnaryOperator op, ASTNode *operand) {
    if (!parser || !operand) return NULL;
    
    ASTNode *node = ast_create_node(AST_UNARY_EXPR, operand->location);
    if (!node) return NULL;
    
    node->data.unary_expr.operator = op;
    node->data.unary_expr.operand = operand;
    
    return node;
}

ASTNode *create_identifier(Parser *parser, const char *name) {
    if (!parser || !name) return NULL;
    
    ASTNode *node = ast_create_node(AST_IDENTIFIER, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.identifier.name = strdup(name);
    
    return node;
}

ASTNode *create_literal_int(Parser *parser, int64_t value) {
    if (!parser) return NULL;
    
    ASTNode *node = ast_create_node(AST_INTEGER_LITERAL, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.integer_literal.value = value;
    node->flags.is_constant_expr = true;  // Integer literals are compile-time constants
    
    return node;
}

ASTNode *create_literal_float(Parser *parser, double value) {
    if (!parser) return NULL;
    
    ASTNode *node = ast_create_node(AST_FLOAT_LITERAL, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.float_literal.value = value;
    
    return node;
}

ASTNode *create_literal_string(Parser *parser, const char *value) {
    if (!parser || !value) return NULL;
    
    ASTNode *node = ast_create_node(AST_STRING_LITERAL, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.string_literal.value = strdup(value);
    
    return node;
}

ASTNode *create_literal_bool(Parser *parser, bool value) {
    if (!parser) return NULL;
    
    ASTNode *node = ast_create_node(AST_BOOL_LITERAL, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.bool_literal.value = value;
    
    return node;
}

ASTNode *create_literal_char(Parser *parser, uint32_t value) {
    if (!parser) return NULL;
    
    ASTNode *node = ast_create_node(AST_CHAR_LITERAL, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.char_literal.value = value;
    
    return node;
}

ASTNode *create_enum_variant(Parser *parser, const char *enum_name, const char *variant_name, ASTNode *value) {
    if (!parser || !enum_name || !variant_name) return NULL;
    
    ASTNode *node = ast_create_node(AST_ENUM_VARIANT, parser->current_token.location);
    if (!node) return NULL;
    
    node->data.enum_variant.enum_name = strdup(enum_name);
    node->data.enum_variant.variant_name = strdup(variant_name);
    node->data.enum_variant.value = value;
    
    return node;
} 
