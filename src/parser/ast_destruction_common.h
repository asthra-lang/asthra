/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Common Patterns
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains common macros and utilities for AST node destruction
 * to eliminate code duplication across destruction modules.
 */

#ifndef AST_DESTRUCTION_COMMON_H
#define AST_DESTRUCTION_COMMON_H

#include "ast.h"
#include "ast_node_refcount.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * Common null check macro for all destruction functions
 */
#define AST_DESTRUCTION_NULL_CHECK(node)                                                           \
    do {                                                                                           \
        if (!node)                                                                                 \
            return;                                                                                \
    } while (0)

/**
 * Safe string free macro - frees string if not null
 */
#define AST_FREE_STRING(str_ptr)                                                                   \
    do {                                                                                           \
        if (str_ptr) {                                                                             \
            free(str_ptr);                                                                         \
            str_ptr = NULL;                                                                        \
        }                                                                                          \
    } while (0)

/**
 * Safe node release macro - releases node if not null
 */
#define AST_RELEASE_NODE(node_ptr)                                                                 \
    do {                                                                                           \
        if (node_ptr) {                                                                            \
            ast_release_node(node_ptr);                                                            \
            node_ptr = NULL;                                                                       \
        }                                                                                          \
    } while (0)

/**
 * Safe node list destroy macro - destroys list if not null
 */
#define AST_DESTROY_NODE_LIST(list_ptr)                                                            \
    do {                                                                                           \
        if (list_ptr) {                                                                            \
            ast_node_list_destroy(list_ptr);                                                       \
            list_ptr = NULL;                                                                       \
        }                                                                                          \
    } while (0)

/**
 * Macro for handling binary expression destruction
 */
#define AST_DESTROY_BINARY_EXPR(node)                                                              \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.binary_expr.left);                                             \
        AST_RELEASE_NODE(node->data.binary_expr.right);                                            \
    } while (0)

/**
 * Macro for handling unary expression destruction
 */
#define AST_DESTROY_UNARY_EXPR(node)                                                               \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.unary_expr.operand);                                           \
    } while (0)

/**
 * Macro for handling call expression destruction
 */
#define AST_DESTROY_CALL_EXPR(node)                                                                \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.call_expr.function);                                           \
        AST_DESTROY_NODE_LIST(node->data.call_expr.args);                                          \
    } while (0)

/**
 * Macro for handling field access destruction
 */
#define AST_DESTROY_FIELD_ACCESS(node)                                                             \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.field_access.object);                                          \
        AST_FREE_STRING(node->data.field_access.field_name);                                       \
    } while (0)

/**
 * Macro for handling index access destruction
 */
#define AST_DESTROY_INDEX_ACCESS(node)                                                             \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.index_access.array);                                           \
        AST_RELEASE_NODE(node->data.index_access.index);                                           \
    } while (0)

/**
 * Macro for handling struct literal destruction
 */
#define AST_DESTROY_STRUCT_LITERAL(node)                                                           \
    do {                                                                                           \
        AST_FREE_STRING(node->data.struct_literal.struct_name);                                    \
        AST_DESTROY_NODE_LIST(node->data.struct_literal.type_args);                                \
        AST_DESTROY_NODE_LIST(node->data.struct_literal.field_inits);                              \
    } while (0)

/**
 * Macro for handling function declaration destruction
 */
#define AST_DESTROY_FUNCTION_DECL(node)                                                            \
    do {                                                                                           \
        AST_FREE_STRING(node->data.function_decl.name);                                            \
        AST_DESTROY_NODE_LIST(node->data.function_decl.params);                                    \
        AST_RELEASE_NODE(node->data.function_decl.return_type);                                    \
        AST_RELEASE_NODE(node->data.function_decl.body);                                           \
        AST_DESTROY_NODE_LIST(node->data.function_decl.annotations);                               \
    } while (0)

/**
 * Macro for handling struct declaration destruction
 */
#define AST_DESTROY_STRUCT_DECL(node)                                                              \
    do {                                                                                           \
        AST_FREE_STRING(node->data.struct_decl.name);                                              \
        AST_DESTROY_NODE_LIST(node->data.struct_decl.fields);                                      \
        AST_DESTROY_NODE_LIST(node->data.struct_decl.type_params);                                 \
        AST_DESTROY_NODE_LIST(node->data.struct_decl.annotations);                                 \
    } while (0)

/**
 * Macro for handling enum declaration destruction
 */
#define AST_DESTROY_ENUM_DECL(node)                                                                \
    do {                                                                                           \
        AST_FREE_STRING(node->data.enum_decl.name);                                                \
        AST_DESTROY_NODE_LIST(node->data.enum_decl.variants);                                      \
        AST_DESTROY_NODE_LIST(node->data.enum_decl.type_params);                                   \
        AST_DESTROY_NODE_LIST(node->data.enum_decl.annotations);                                   \
    } while (0)

/**
 * Macro for handling if statement destruction
 */
#define AST_DESTROY_IF_STMT(node)                                                                  \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.if_stmt.condition);                                            \
        AST_RELEASE_NODE(node->data.if_stmt.then_block);                                           \
        AST_RELEASE_NODE(node->data.if_stmt.else_block);                                           \
    } while (0)

/**
 * Macro for handling match statement destruction
 */
#define AST_DESTROY_MATCH_STMT(node)                                                               \
    do {                                                                                           \
        AST_RELEASE_NODE(node->data.match_stmt.expression);                                        \
        AST_DESTROY_NODE_LIST(node->data.match_stmt.arms);                                         \
    } while (0)

/**
 * Common header comment for all destruction files
 */
#define AST_DESTRUCTION_FILE_HEADER(category)                                                      \
    "/**\n"                                                                                        \
    " * Asthra Programming Language Compiler\n"                                                    \
    " * AST Node Destruction - " category "\n"                                                     \
    " * \n"                                                                                        \
    " * Copyright (c) 2024 Asthra Project\n"                                                       \
    " * Licensed under the terms specified in LICENSE\n"                                           \
    " * \n"                                                                                        \
    " * This file contains destruction operations for " category " AST nodes\n"                    \
    " * Uses common patterns from ast_destruction_common.h\n"                                      \
    " */"

/**
 * Statistics tracking for destruction operations (optional)
 */
typedef struct {
    size_t nodes_destroyed;
    size_t strings_freed;
    size_t lists_destroyed;
} ASTDestructionStats;

extern ASTDestructionStats ast_destruction_stats;

/**
 * Initialize destruction statistics
 */
void ast_destruction_stats_init(void);

/**
 * Get current destruction statistics
 */
ASTDestructionStats ast_destruction_stats_get(void);

/**
 * Reset destruction statistics
 */
void ast_destruction_stats_reset(void);

#endif /* AST_DESTRUCTION_COMMON_H */
