/**
 * Asthra Programming Language Compiler
 * Parser AST creation helper functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_AST_HELPERS_H
#define ASTHRA_PARSER_AST_HELPERS_H

#include "ast.h"
#include "lexer.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// AST NODE CREATION HELPERS
// =============================================================================

// AST node creation helpers
ASTNode *create_binary_expr(Parser *parser, BinaryOperator op, ASTNode *left, ASTNode *right);
ASTNode *create_unary_expr(Parser *parser, UnaryOperator op, ASTNode *operand);
ASTNode *create_identifier(Parser *parser, const char *name);
ASTNode *create_literal_int(Parser *parser, int64_t value);
ASTNode *create_literal_float(Parser *parser, double value);
ASTNode *create_literal_string(Parser *parser, const char *value);
ASTNode *create_literal_bool(Parser *parser, bool value);
ASTNode *create_literal_char(Parser *parser, uint32_t value);
ASTNode *create_enum_variant(Parser *parser, const char *enum_name, const char *variant_name,
                             ASTNode *value);

// Enhanced AST creation with statistics tracking
ASTNode *create_ast_node_safe(Parser *parser, ASTNodeType type, ASTNodeData data);
ASTNode *create_ast_node_with_location(Parser *parser, ASTNodeType type, SourceLocation location);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_AST_HELPERS_H
