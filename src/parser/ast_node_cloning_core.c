/**
 * Asthra Programming Language Compiler
 * AST Node Cloning Core Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains the core AST node cloning function and
 * handles cloning of basic/literal nodes and annotations.
 */

#include "ast_node_cloning_internal.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CORE NODE CLONING FUNCTION
// =============================================================================

ASTNode *ast_clone_node(ASTNode *node) {
    if (!node)
        return NULL;

    ASTNode *clone = ast_create_node(node->type, node->location);
    if (!clone)
        return NULL;

    // Copy type info
    clone->type_info = node->type_info;

    switch (node->type) {
    // Basic nodes and literals
    case AST_PACKAGE_DECL:
        if (node->data.package_decl.name) {
            clone->data.package_decl.name = strdup(node->data.package_decl.name);
        }
        break;

    case AST_IMPORT_DECL:
        if (node->data.import_decl.path) {
            clone->data.import_decl.path = strdup(node->data.import_decl.path);
        }
        if (node->data.import_decl.alias) {
            clone->data.import_decl.alias = strdup(node->data.import_decl.alias);
        }
        break;

    case AST_INTEGER_LITERAL:
        clone->data.integer_literal.value = node->data.integer_literal.value;
        break;

    case AST_FLOAT_LITERAL:
        clone->data.float_literal.value = node->data.float_literal.value;
        break;

    case AST_STRING_LITERAL:
        if (node->data.string_literal.value) {
            clone->data.string_literal.value = strdup(node->data.string_literal.value);
        }
        break;

    case AST_BOOL_LITERAL:
        clone->data.bool_literal.value = node->data.bool_literal.value;
        break;

    case AST_CHAR_LITERAL:
        clone->data.char_literal.value = node->data.char_literal.value;
        break;

    case AST_UNIT_LITERAL:
        // Unit literal has no data to copy
        break;

    case AST_IDENTIFIER:
        if (node->data.identifier.name) {
            clone->data.identifier.name = strdup(node->data.identifier.name);
        }
        break;

    case AST_PROGRAM:
        clone->data.program.package_decl = ast_clone_node(node->data.program.package_decl);
        clone->data.program.imports = ast_node_list_clone_deep(node->data.program.imports);
        clone->data.program.declarations =
            ast_node_list_clone_deep(node->data.program.declarations);
        break;

    case AST_BLOCK:
        clone->data.block.statements = ast_node_list_clone_deep(node->data.block.statements);
        break;

    // Annotation node types
    case AST_OWNERSHIP_TAG:
        clone->data.ownership_tag.ownership = node->data.ownership_tag.ownership;
        break;

    case AST_FFI_ANNOTATION:
        clone->data.ffi_annotation.transfer_type = node->data.ffi_annotation.transfer_type;
        break;

    case AST_SECURITY_TAG:
        clone->data.security_tag.security_type = node->data.security_tag.security_type;
        break;

    case AST_HUMAN_REVIEW_TAG:
        clone->data.human_review_tag.priority = node->data.human_review_tag.priority;
        break;

    case AST_VISIBILITY_MODIFIER:
        clone->data.visibility_modifier.is_public = node->data.visibility_modifier.is_public;
        break;

    case AST_SEMANTIC_TAG:
        if (node->data.semantic_tag.name) {
            clone->data.semantic_tag.name = strdup(node->data.semantic_tag.name);
        }
        clone->data.semantic_tag.params = ast_node_list_clone_deep(node->data.semantic_tag.params);
        break;

    // Simple statement nodes
    case AST_BREAK_STMT:
    case AST_CONTINUE_STMT:
    case AST_WILDCARD_PATTERN:
        // These have no data to clone
        break;

    // Delegate to specialized cloning functions
    case AST_FUNCTION_DECL:
    case AST_STRUCT_DECL:
    case AST_ENUM_DECL:
    case AST_ENUM_VARIANT_DECL:
    case AST_IMPL_BLOCK:
    case AST_METHOD_DECL:
    case AST_EXTERN_DECL:
    case AST_PARAM_DECL:
    case AST_CONST_DECL:
    case AST_STRUCT_FIELD:
        return clone_declaration_node(node, clone);

    case AST_STRUCT_LITERAL:
    case AST_BINARY_EXPR:
    case AST_UNARY_EXPR:
    case AST_CALL_EXPR:
    case AST_ARRAY_LITERAL:
    case AST_TUPLE_LITERAL:
    case AST_POSTFIX_EXPR:
    case AST_ASSOCIATED_FUNC_CALL:
    case AST_FIELD_ACCESS:
    case AST_INDEX_ACCESS:
    case AST_SLICE_EXPR:
    case AST_SLICE_LENGTH_ACCESS:
    case AST_ASSIGNMENT:
    case AST_AWAIT_EXPR:
    case AST_ENUM_VARIANT:
    case AST_CONST_EXPR:
        return clone_expression_node(node, clone);

    case AST_EXPR_STMT:
    case AST_LET_STMT:
    case AST_RETURN_STMT:
    case AST_IF_STMT:
    case AST_FOR_STMT:
    case AST_IF_LET_STMT:
    case AST_MATCH_STMT:
    case AST_SPAWN_STMT:
    case AST_SPAWN_WITH_HANDLE_STMT:
    case AST_UNSAFE_BLOCK:
        return clone_statement_node(node, clone);

    case AST_BASE_TYPE:
    case AST_SLICE_TYPE:
    case AST_ARRAY_TYPE:
    case AST_PTR_TYPE:
    case AST_RESULT_TYPE:
    case AST_OPTION_TYPE:
    case AST_TASKHANDLE_TYPE:
    case AST_STRUCT_TYPE:
    case AST_ENUM_TYPE:
    case AST_TUPLE_TYPE:
        return clone_type_node(node, clone);

    case AST_MATCH_ARM:
    case AST_TUPLE_PATTERN:
    case AST_ENUM_PATTERN:
        return clone_pattern_node(node, clone);

    // Default case
    default:
        // For now, just copy the data union directly for simple cases
        clone->data = node->data;
        break;
    }

    return clone;
}