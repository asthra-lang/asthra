/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Traversal Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains the implementation of AST traversal operations
 * Split from the original ast.c for better maintainability
 */

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// AST TRAVERSAL
// =============================================================================

void ast_traverse_preorder(ASTNode *node, ASTVisitor visitor, void *context) {
    if (!node || !visitor)
        return;

    visitor(node, context);

    // Traverse children using ASTNodeList structure
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.package_decl) {
            ast_traverse_preorder(node->data.program.package_decl, visitor, context);
        }
        if (node->data.program.imports) {
            for (size_t i = 0; i < ast_node_list_size(node->data.program.imports); i++) {
                ASTNode *import_node = ast_node_list_get(node->data.program.imports, i);
                if (import_node) {
                    ast_traverse_preorder(import_node, visitor, context);
                }
            }
        }
        if (node->data.program.declarations) {
            for (size_t i = 0; i < ast_node_list_size(node->data.program.declarations); i++) {
                ASTNode *decl_node = ast_node_list_get(node->data.program.declarations, i);
                if (decl_node) {
                    ast_traverse_preorder(decl_node, visitor, context);
                }
            }
        }
        break;

    default: {
        // For other node types, use the generic child traversal
        size_t child_count = ast_get_child_count(node);
        for (size_t i = 0; i < child_count; i++) {
            ASTNode *child = ast_get_child(node, i);
            if (child) {
                ast_traverse_preorder(child, visitor, context);
            }
        }
        break;
    }
    }
}

void ast_traverse_postorder(ASTNode *node, ASTVisitor visitor, void *context) {
    if (!node || !visitor)
        return;

    // Visit children first using ASTNodeList structure
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.package_decl) {
            ast_traverse_postorder(node->data.program.package_decl, visitor, context);
        }
        if (node->data.program.imports) {
            for (size_t i = 0; i < ast_node_list_size(node->data.program.imports); i++) {
                ASTNode *import_node = ast_node_list_get(node->data.program.imports, i);
                if (import_node) {
                    ast_traverse_postorder(import_node, visitor, context);
                }
            }
        }
        if (node->data.program.declarations) {
            for (size_t i = 0; i < ast_node_list_size(node->data.program.declarations); i++) {
                ASTNode *decl_node = ast_node_list_get(node->data.program.declarations, i);
                if (decl_node) {
                    ast_traverse_postorder(decl_node, visitor, context);
                }
            }
        }
        break;

    default: {
        // For other node types, use the generic child traversal
        size_t child_count = ast_get_child_count(node);
        for (size_t i = 0; i < child_count; i++) {
            ASTNode *child = ast_get_child(node, i);
            if (child) {
                ast_traverse_postorder(child, visitor, context);
            }
        }
        break;
    }
    }

    // Then visit this node
    visitor(node, context);
}

// =============================================================================
// IMPLEMENTATION FUNCTIONS FOR _Generic MACROS
// =============================================================================

size_t ast_get_child_count_impl(const ASTNode *node) {
    if (!node)
        return 0;

    switch (node->type) {
    case AST_PROGRAM:
        return (node->data.program.imports ? ast_node_list_size(node->data.program.imports) : 0) +
               (node->data.program.declarations
                    ? ast_node_list_size(node->data.program.declarations)
                    : 0) +
               (node->data.program.package_decl ? 1 : 0);

    case AST_FUNCTION_DECL:
        return (node->data.function_decl.params
                    ? ast_node_list_size(node->data.function_decl.params)
                    : 0) +
               (node->data.function_decl.return_type ? 1 : 0) +
               (node->data.function_decl.body ? 1 : 0) +
               (node->data.function_decl.annotations
                    ? ast_node_list_size(node->data.function_decl.annotations)
                    : 0);

    case AST_STRUCT_DECL:
        return (node->data.struct_decl.fields ? ast_node_list_size(node->data.struct_decl.fields)
                                              : 0) +
               (node->data.struct_decl.type_params
                    ? ast_node_list_size(node->data.struct_decl.type_params)
                    : 0) +
               (node->data.struct_decl.annotations
                    ? ast_node_list_size(node->data.struct_decl.annotations)
                    : 0);

    case AST_ENUM_DECL:
        return (node->data.enum_decl.variants ? ast_node_list_size(node->data.enum_decl.variants)
                                              : 0) +
               (node->data.enum_decl.type_params
                    ? ast_node_list_size(node->data.enum_decl.type_params)
                    : 0) +
               (node->data.enum_decl.annotations
                    ? ast_node_list_size(node->data.enum_decl.annotations)
                    : 0);

    case AST_ENUM_VARIANT_DECL:
        return (node->data.enum_variant_decl.associated_type ? 1 : 0) +
               (node->data.enum_variant_decl.value ? 1 : 0);

    case AST_BLOCK:
        return node->data.block.statements ? ast_node_list_size(node->data.block.statements) : 0;

    case AST_BINARY_EXPR:
        return (node->data.binary_expr.left ? 1 : 0) + (node->data.binary_expr.right ? 1 : 0);

    case AST_UNARY_EXPR:
        return node->data.unary_expr.operand ? 1 : 0;

    case AST_CALL_EXPR:
        return (node->data.call_expr.function ? 1 : 0) +
               (node->data.call_expr.args ? ast_node_list_size(node->data.call_expr.args) : 0);

    case AST_ENUM_VARIANT:
        return node->data.enum_variant.value ? 1 : 0;

    case AST_STRUCT_LITERAL:
        return (node->data.struct_literal.type_args
                    ? ast_node_list_size(node->data.struct_literal.type_args)
                    : 0) +
               (node->data.struct_literal.field_inits
                    ? ast_node_list_size(node->data.struct_literal.field_inits)
                    : 0);

    case AST_IMPL_BLOCK:
        return (node->data.impl_block.methods ? ast_node_list_size(node->data.impl_block.methods)
                                              : 0) +
               (node->data.impl_block.annotations
                    ? ast_node_list_size(node->data.impl_block.annotations)
                    : 0);

    case AST_METHOD_DECL:
        return (node->data.method_decl.params ? ast_node_list_size(node->data.method_decl.params)
                                              : 0) +
               (node->data.method_decl.return_type ? 1 : 0) +
               (node->data.method_decl.body ? 1 : 0) +
               (node->data.method_decl.annotations
                    ? ast_node_list_size(node->data.method_decl.annotations)
                    : 0);

    case AST_SLICE_TYPE:
        return node->data.slice_type.element_type ? 1 : 0;

    case AST_ARRAY_TYPE:
        return (node->data.array_type.element_type ? 1 : 0) + (node->data.array_type.size ? 1 : 0);

    case AST_STRUCT_TYPE:
        return node->data.struct_type.type_args
                   ? ast_node_list_size(node->data.struct_type.type_args)
                   : 0;

    case AST_ENUM_TYPE:
        return node->data.enum_type.type_args ? ast_node_list_size(node->data.enum_type.type_args)
                                              : 0;

    case AST_PTR_TYPE:
        return node->data.ptr_type.pointee_type ? 1 : 0;

    case AST_CONST_DECL:
        return (node->data.const_decl.type ? 1 : 0) + (node->data.const_decl.value ? 1 : 0) +
               (node->data.const_decl.annotations
                    ? ast_node_list_size(node->data.const_decl.annotations)
                    : 0);

    case AST_CONST_EXPR:
        switch (node->data.const_expr.expr_type) {
        case CONST_EXPR_LITERAL:
            return node->data.const_expr.data.literal ? 1 : 0;
        case CONST_EXPR_BINARY_OP:
            return (node->data.const_expr.data.binary.left ? 1 : 0) +
                   (node->data.const_expr.data.binary.right ? 1 : 0);
        case CONST_EXPR_UNARY_OP:
            return node->data.const_expr.data.unary.operand ? 1 : 0;
        case CONST_EXPR_SIZEOF:
            return node->data.const_expr.data.sizeof_expr.type ? 1 : 0;
        case CONST_EXPR_IDENTIFIER:
            return 0; // Identifier is stored as string, no child nodes
        default:
            return 0;
        }

    default:
        return 0; // For leaf nodes and unhandled types
    }
}

ASTNode *ast_get_child_impl(ASTNode *node, size_t index) {
    if (!node)
        return NULL;

    switch (node->type) {
    case AST_BLOCK:
        if (node->data.block.statements &&
            index < ast_node_list_size(node->data.block.statements)) {
            return ast_node_list_get(node->data.block.statements, index);
        }
        break;

    case AST_BINARY_EXPR:
        if (index == 0)
            return node->data.binary_expr.left;
        if (index == 1)
            return node->data.binary_expr.right;
        break;

    case AST_UNARY_EXPR:
        if (index == 0)
            return node->data.unary_expr.operand;
        break;

    case AST_ENUM_VARIANT:
        if (index == 0)
            return node->data.enum_variant.value;
        break;

    case AST_ENUM_TYPE:
        if (node->data.enum_type.type_args &&
            index < ast_node_list_size(node->data.enum_type.type_args)) {
            return ast_node_list_get(node->data.enum_type.type_args, index);
        }
        break;

    case AST_SLICE_TYPE:
        if (index == 0)
            return node->data.slice_type.element_type;
        break;

    case AST_ARRAY_TYPE:
        if (index == 0)
            return node->data.array_type.element_type;
        if (index == 1)
            return node->data.array_type.size;
        break;

    case AST_PTR_TYPE:
        if (index == 0)
            return node->data.ptr_type.pointee_type;
        break;

    case AST_ENUM_VARIANT_DECL:
        if (index == 0 && node->data.enum_variant_decl.associated_type) {
            return node->data.enum_variant_decl.associated_type;
        }
        if ((index == 1 && node->data.enum_variant_decl.associated_type) ||
            (index == 0 && !node->data.enum_variant_decl.associated_type)) {
            return node->data.enum_variant_decl.value;
        }
        break;

    default:
        break;
    }

    return NULL;
}

const ASTNode *ast_get_child_const_impl(const ASTNode *node, size_t index) {
    if (!node)
        return NULL;

    switch (node->type) {
    case AST_BLOCK:
        if (node->data.block.statements &&
            index < ast_node_list_size(node->data.block.statements)) {
            return ast_node_list_get(node->data.block.statements, index);
        }
        break;

    case AST_BINARY_EXPR:
        if (index == 0)
            return node->data.binary_expr.left;
        if (index == 1)
            return node->data.binary_expr.right;
        break;

    case AST_UNARY_EXPR:
        if (index == 0)
            return node->data.unary_expr.operand;
        break;

    case AST_ENUM_VARIANT:
        if (index == 0)
            return node->data.enum_variant.value;
        break;

    case AST_ENUM_TYPE:
        if (node->data.enum_type.type_args &&
            index < ast_node_list_size(node->data.enum_type.type_args)) {
            return ast_node_list_get(node->data.enum_type.type_args, index);
        }
        break;

    case AST_SLICE_TYPE:
        if (index == 0)
            return node->data.slice_type.element_type;
        break;

    case AST_ARRAY_TYPE:
        if (index == 0)
            return node->data.array_type.element_type;
        if (index == 1)
            return node->data.array_type.size;
        break;

    case AST_PTR_TYPE:
        if (index == 0)
            return node->data.ptr_type.pointee_type;
        break;

    case AST_ENUM_VARIANT_DECL:
        if (index == 0 && node->data.enum_variant_decl.associated_type) {
            return node->data.enum_variant_decl.associated_type;
        }
        if ((index == 1 && node->data.enum_variant_decl.associated_type) ||
            (index == 0 && !node->data.enum_variant_decl.associated_type)) {
            return node->data.enum_variant_decl.value;
        }
        break;

    default:
        break;
    }

    return NULL;
}

bool ast_set_child_impl(ASTNode *node, size_t index, ASTNode *child) {
    if (!node)
        return false;

    switch (node->type) {
    case AST_BINARY_EXPR:
        if (index == 0) {
            if (node->data.binary_expr.left)
                ast_release_node(node->data.binary_expr.left);
            node->data.binary_expr.left = ast_retain_node(child);
            return true;
        }
        if (index == 1) {
            if (node->data.binary_expr.right)
                ast_release_node(node->data.binary_expr.right);
            node->data.binary_expr.right = ast_retain_node(child);
            return true;
        }
        break;

    case AST_UNARY_EXPR:
        if (index == 0) {
            if (node->data.unary_expr.operand)
                ast_release_node(node->data.unary_expr.operand);
            node->data.unary_expr.operand = ast_retain_node(child);
            return true;
        }
        break;

    default:
        break;
    }

    return false;
}

bool ast_add_child_impl(ASTNode *node, ASTNode *child) {
    if (!node || !child)
        return false;

    switch (node->type) {
    case AST_BLOCK:
        return ast_node_list_add(&node->data.block.statements, child);

    case AST_FUNCTION_DECL:
        return ast_node_list_add(&node->data.function_decl.params, child);

    case AST_CALL_EXPR:
        return ast_node_list_add(&node->data.call_expr.args, child);

    case AST_ENUM_DECL:
        return ast_node_list_add(&node->data.enum_decl.variants, child);

    default:
        return false;
    }
}

bool ast_remove_child_impl(ASTNode *node, size_t index) {
    if (!node)
        return false;

    switch (node->type) {
    case AST_BLOCK:
        return ast_node_list_remove(&node->data.block.statements, index);

    case AST_FUNCTION_DECL:
        return ast_node_list_remove(&node->data.function_decl.params, index);

    case AST_CALL_EXPR:
        return ast_node_list_remove(&node->data.call_expr.args, index);

    default:
        return false;
    }
}

// Type-checking operations
bool ast_validate_node_impl(ASTNode *node) {
    if (!node)
        return false;

    // Mark as validated
    node->flags.is_validated = true;

    // Basic validation logic
    switch (node->type) {
    case AST_BINARY_EXPR:
        return node->data.binary_expr.left && node->data.binary_expr.right;

    case AST_UNARY_EXPR:
        return node->data.unary_expr.operand != NULL;

    case AST_IDENTIFIER:
        return node->data.identifier.name && strlen(node->data.identifier.name) > 0;

    default:
        return true; // Assume valid for other types
    }
}

bool ast_validate_node_const_impl(const ASTNode *node) {
    if (!node)
        return false;

    // Basic validation logic (without modifying flags)
    switch (node->type) {
    case AST_BINARY_EXPR:
        return node->data.binary_expr.left && node->data.binary_expr.right;

    case AST_UNARY_EXPR:
        return node->data.unary_expr.operand != NULL;

    case AST_IDENTIFIER:
        return node->data.identifier.name && strlen(node->data.identifier.name) > 0;

    default:
        return true; // Assume valid for other types
    }
}

bool ast_is_expression_impl(const ASTNode *node) {
    if (!node)
        return false;

    return (node->type >= AST_BINARY_EXPR && node->type <= AST_IDENTIFIER) ||
           node->type == AST_ENUM_VARIANT || node->type == AST_STRUCT_LITERAL;
}

bool ast_is_statement_impl(const ASTNode *node) {
    if (!node)
        return false;

    return node->type >= AST_BLOCK && node->type <= AST_UNSAFE_BLOCK;
}

bool ast_is_declaration_impl(const ASTNode *node) {
    if (!node)
        return false;

    return node->type == AST_FUNCTION_DECL || node->type == AST_STRUCT_DECL ||
           node->type == AST_STRUCT_FIELD || node->type == AST_ENUM_DECL ||
           node->type == AST_ENUM_VARIANT_DECL || node->type == AST_EXTERN_DECL ||
           node->type == AST_PARAM_DECL || node->type == AST_IMPL_BLOCK ||
           node->type == AST_METHOD_DECL || node->type == AST_CONST_DECL;
}
