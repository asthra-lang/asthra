/**
 * Asthra Programming Language Compiler
 * AST Generic Type-Safe Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * C17 _Generic macros for type-safe AST operations
 */

#ifndef ASTHRA_AST_GENERIC_H
#define ASTHRA_AST_GENERIC_H

#include "ast_types.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 GENERIC TYPE-SAFE AST OPERATIONS - PHASE 2.1
// =============================================================================

// Phase 2.1: C17 _Generic for type-safe AST node operations
#define ast_get_child_count(node)                                                                  \
    _Generic((node),                                                                               \
        ASTNode *: ast_get_child_count_impl,                                                       \
        const ASTNode *: ast_get_child_count_impl)(node)

#define ast_get_child(node, index)                                                                 \
    _Generic((node), ASTNode *: ast_get_child_impl, const ASTNode *: ast_get_child_const_impl)(    \
        node, index)

#define ast_set_child(node, index, child)                                                          \
    _Generic((node), ASTNode *: ast_set_child_impl)(node, index, child)

#define ast_add_child(node, child) _Generic((node), ASTNode *: ast_add_child_impl)(node, child)

#define ast_remove_child(node, index)                                                              \
    _Generic((node), ASTNode *: ast_remove_child_impl)(node, index)

// Phase 2.1: Enhanced node creation with compound literals
typedef union {
    struct {
        BinaryOperator operator;
        ASTNode *left;
        ASTNode *right;
    } binary_expr;
    struct {
        UnaryOperator operator;
        ASTNode *operand;
    } unary_expr;
    struct {
        char *name;
    } identifier;
    struct {
        int64_t value;
    } integer_literal;
    struct {
        double value;
    } float_literal;
    struct {
        char *value;
    } string_literal;
    struct {
        bool value;
    } bool_literal;
    struct {
        uint32_t value;
    } char_literal; // Unicode code point
    // Add other node data types as needed
} ASTNodeData;

// Create node with pre-initialized data using compound literals
ASTNode *ast_create_node_with_data(ASTNodeType type, SourceLocation location, ASTNodeData data);

// Phase 2.1: Type-safe node creation with C17 compound literals
#define AST_CREATE_BINARY_EXPR(op, left, right, loc)                                               \
    ast_create_node_with_data(                                                                     \
        AST_BINARY_EXPR, loc,                                                                      \
        (ASTNodeData){.binary_expr = {.operator=(op), .left = (left), .right = (right)}})

#define AST_CREATE_UNARY_EXPR(op, operand, loc)                                                    \
    ast_create_node_with_data(AST_UNARY_EXPR, loc,                                                 \
                              (ASTNodeData){.unary_expr = {.operator=(op), .operand = (operand)}})

#define AST_CREATE_IDENTIFIER(name, loc)                                                           \
    ast_create_node_with_data(AST_IDENTIFIER, loc, (ASTNodeData){.identifier = {.name = (name)}})

#define AST_CREATE_INTEGER_LITERAL(value, loc)                                                     \
    ast_create_node_with_data(AST_INTEGER_LITERAL, loc,                                            \
                              (ASTNodeData){.integer_literal = {.value = (value)}})

#define AST_CREATE_STRING_LITERAL(value, loc)                                                      \
    ast_create_node_with_data(AST_STRING_LITERAL, loc,                                             \
                              (ASTNodeData){.string_literal = {.value = (value)}})

// Phase 2.1: Type-safe node validation with _Generic
#define ast_validate_node(node)                                                                    \
    _Generic((node),                                                                               \
        ASTNode *: ast_validate_node_impl,                                                         \
        const ASTNode *: ast_validate_node_const_impl)(node)

#define ast_is_expression(node)                                                                    \
    _Generic((node), ASTNode *: ast_is_expression_impl, const ASTNode *: ast_is_expression_impl)(  \
        node)

#define ast_is_statement(node)                                                                     \
    _Generic((node), ASTNode *: ast_is_statement_impl, const ASTNode *: ast_is_statement_impl)(node)

#define ast_is_declaration(node)                                                                   \
    _Generic((node),                                                                               \
        ASTNode *: ast_is_declaration_impl,                                                        \
        const ASTNode *: ast_is_declaration_impl)(node)

// Phase 2.1: Atomic reference counting operations
// These are now in ast_node_refcount.h

// =============================================================================
// IMPLEMENTATION FUNCTIONS FOR _Generic MACROS - PHASE 2.1
// =============================================================================

// Implementation functions (not for direct use)
size_t ast_get_child_count_impl(const ASTNode *node);
ASTNode *ast_get_child_impl(ASTNode *node, size_t index);
const ASTNode *ast_get_child_const_impl(const ASTNode *node, size_t index);
bool ast_set_child_impl(ASTNode *node, size_t index, ASTNode *child);
bool ast_add_child_impl(ASTNode *node, ASTNode *child);
bool ast_remove_child_impl(ASTNode *node, size_t index);

bool ast_validate_node_impl(ASTNode *node);
bool ast_validate_node_const_impl(const ASTNode *node);
bool ast_is_expression_impl(const ASTNode *node);
bool ast_is_statement_impl(const ASTNode *node);
bool ast_is_declaration_impl(const ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AST_GENERIC_H
