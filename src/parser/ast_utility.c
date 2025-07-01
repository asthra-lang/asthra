/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Utility Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains the implementation of AST utility functions such as
 * printing, validation, and node finding
 * Split from the original ast.c for better maintainability
 */

#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// AST UTILITY FUNCTIONS
// =============================================================================

void ast_print(ASTNode *node, int indent) {
    if (!node)
        return;

    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    printf("%s", ast_node_type_name(node->type));

    // Print node-specific information
    switch (node->type) {
    case AST_IMPORT_DECL:
        printf(" (path: \"%s\"",
               node->data.import_decl.path ? node->data.import_decl.path : "null");
        if (node->data.import_decl.alias) {
            printf(", alias: \"%s\"", node->data.import_decl.alias);
        }
        printf(")");
        break;

    case AST_VISIBILITY_MODIFIER:
        printf(" (%s)", node->data.visibility_modifier.is_public ? "public" : "private");
        break;
    case AST_IDENTIFIER:
        printf(" (%s)", node->data.identifier.name ? node->data.identifier.name : "null");
        break;
    case AST_INTEGER_LITERAL:
        printf(" (%lld)", (long long)node->data.integer_literal.value);
        break;
    case AST_FLOAT_LITERAL:
        printf(" (%f)", node->data.float_literal.value);
        break;
    case AST_STRING_LITERAL:
        printf(" (\"%s\")",
               node->data.string_literal.value ? node->data.string_literal.value : "null");
        break;
    case AST_BOOL_LITERAL:
        printf(" (%s)", node->data.bool_literal.value ? "true" : "false");
        break;
    case AST_CHAR_LITERAL:
        printf(" (%c)", node->data.char_literal.value);
        break;
    case AST_STRUCT_LITERAL:
        printf(" (%s with %zu fields)",
               node->data.struct_literal.struct_name ? node->data.struct_literal.struct_name
                                                     : "null",
               node->data.struct_literal.field_inits
                   ? ast_node_list_size(node->data.struct_literal.field_inits)
                   : 0);
        break;
    case AST_BINARY_EXPR:
        printf(" (%s)", binary_operator_name(node->data.binary_expr.operator));
        break;
    case AST_UNARY_EXPR:
        printf(" (%s)", unary_operator_name(node->data.unary_expr.operator));
        break;
    case AST_ENUM_VARIANT:
        printf(" (%s.%s)",
               node->data.enum_variant.enum_name ? node->data.enum_variant.enum_name : "null",
               node->data.enum_variant.variant_name ? node->data.enum_variant.variant_name
                                                    : "null");
        break;
    case AST_STRUCT_PATTERN:
        printf(" (%s with %zu field patterns)",
               node->data.struct_pattern.struct_name ? node->data.struct_pattern.struct_name
                                                     : "null",
               node->data.struct_pattern.field_patterns
                   ? ast_node_list_size(node->data.struct_pattern.field_patterns)
                   : 0);
        break;
    case AST_FIELD_PATTERN:
        printf(" (field: %s",
               node->data.field_pattern.field_name ? node->data.field_pattern.field_name : "null");
        if (node->data.field_pattern.binding_name) {
            printf(", binding: %s", node->data.field_pattern.binding_name);
        }
        if (node->data.field_pattern.is_ignored) {
            printf(", ignored");
        }
        printf(")");
        break;
    case AST_CALL_EXPR:
        printf(" (function: %p)", (void *)node->data.call_expr.function);
        break;
    case AST_ASSOCIATED_FUNC_CALL:
        printf(" (struct: %s, func: %s, type_args: %s)",
               node->data.associated_func_call.struct_name
                   ? node->data.associated_func_call.struct_name
                   : "null",
               node->data.associated_func_call.function_name
                   ? node->data.associated_func_call.function_name
                   : "null",
               node->data.associated_func_call.type_args ? "present" : "none");
        break;
    case AST_FIELD_ACCESS:
        printf(" (field: %s)",
               node->data.field_access.field_name ? node->data.field_access.field_name : "null");
        break;
    default:
        break;
    }

    printf("\n");

    // Print children using the new ASTNodeList structure
    switch (node->type) {
    case AST_PROGRAM:
        if (node->data.program.package_decl) {
            ast_print(node->data.program.package_decl, indent + 1);
        }
        if (node->data.program.imports) {
            for (size_t i = 0; i < ast_node_list_size(node->data.program.imports); i++) {
                ASTNode *import_node = ast_node_list_get(node->data.program.imports, i);
                if (import_node) {
                    ast_print(import_node, indent + 1);
                }
            }
        }
        if (node->data.program.declarations) {
            for (size_t i = 0; i < ast_node_list_size(node->data.program.declarations); i++) {
                ASTNode *decl_node = ast_node_list_get(node->data.program.declarations, i);
                if (decl_node) {
                    ast_print(decl_node, indent + 1);
                }
            }
        }
        break;

    case AST_BLOCK:
        if (node->data.block.statements) {
            for (size_t i = 0; i < ast_node_list_size(node->data.block.statements); i++) {
                ASTNode *stmt_node = ast_node_list_get(node->data.block.statements, i);
                if (stmt_node) {
                    ast_print(stmt_node, indent + 1);
                }
            }
        }
        break;

    case AST_STRUCT_PATTERN:
        if (node->data.struct_pattern.field_patterns) {
            for (size_t i = 0; i < ast_node_list_size(node->data.struct_pattern.field_patterns);
                 i++) {
                ASTNode *field_pattern =
                    ast_node_list_get(node->data.struct_pattern.field_patterns, i);
                if (field_pattern) {
                    ast_print(field_pattern, indent + 1);
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
                ast_print(child, indent + 1);
            }
        }
        break;
    }
    }
}

bool ast_validate(ASTNode *node) {
    if (!node)
        return false;

    // Basic validation - check that required fields are present
    switch (node->type) {
    case AST_PACKAGE_DECL:
        return node->data.package_decl.name != NULL;

    case AST_IMPORT_DECL:
        return node->data.import_decl.path != NULL;

    case AST_IDENTIFIER:
        return node->data.identifier.name != NULL;

    case AST_BINARY_EXPR:
        return node->data.binary_expr.left != NULL && node->data.binary_expr.right != NULL &&
               ast_validate(node->data.binary_expr.left) &&
               ast_validate(node->data.binary_expr.right);

    case AST_UNARY_EXPR:
        return node->data.unary_expr.operand != NULL && ast_validate(node->data.unary_expr.operand);

    case AST_ENUM_DECL:
        return node->data.enum_decl.name != NULL;

    case AST_ENUM_VARIANT_DECL:
        return node->data.enum_variant_decl.name != NULL;

    default:
        return true; // Assume valid for other types
    }
}

ASTNode *ast_find_node(ASTNode *root, ASTNodeType type, const char *name) {
    if (!root)
        return NULL;

    // Check current node
    if (root->type == type) {
        if (!name)
            return root; // Type match is enough

        // Check name for specific node types
        switch (type) {
        case AST_IDENTIFIER:
            if (root->data.identifier.name && strcmp(root->data.identifier.name, name) == 0) {
                return root;
            }
            break;
        case AST_FUNCTION_DECL:
            if (root->data.function_decl.name && strcmp(root->data.function_decl.name, name) == 0) {
                return root;
            }
            break;
        case AST_STRUCT_DECL:
            if (root->data.struct_decl.name && strcmp(root->data.struct_decl.name, name) == 0) {
                return root;
            }
            break;
        case AST_ENUM_DECL:
            if (root->data.enum_decl.name && strcmp(root->data.enum_decl.name, name) == 0) {
                return root;
            }
            break;
        case AST_ENUM_VARIANT_DECL:
            if (root->data.enum_variant_decl.name &&
                strcmp(root->data.enum_variant_decl.name, name) == 0) {
                return root;
            }
            break;
        default:
            return root; // Type match for other types
        }
    }

    // Search children using ASTNodeList structure
    switch (root->type) {
    case AST_PROGRAM:
        if (root->data.program.package_decl) {
            ASTNode *found = ast_find_node(root->data.program.package_decl, type, name);
            if (found)
                return found;
        }
        if (root->data.program.imports) {
            for (size_t i = 0; i < ast_node_list_size(root->data.program.imports); i++) {
                ASTNode *import_node = ast_node_list_get(root->data.program.imports, i);
                if (import_node) {
                    ASTNode *found = ast_find_node(import_node, type, name);
                    if (found)
                        return found;
                }
            }
        }
        if (root->data.program.declarations) {
            for (size_t i = 0; i < ast_node_list_size(root->data.program.declarations); i++) {
                ASTNode *decl_node = ast_node_list_get(root->data.program.declarations, i);
                if (decl_node) {
                    ASTNode *found = ast_find_node(decl_node, type, name);
                    if (found)
                        return found;
                }
            }
        }
        break;

    default: {
        // For other node types, use the generic child traversal
        size_t child_count = ast_get_child_count(root);
        for (size_t i = 0; i < child_count; i++) {
            ASTNode *child = ast_get_child(root, i);
            if (child) {
                ASTNode *found = ast_find_node(child, type, name);
                if (found)
                    return found;
            }
        }
        break;
    }
    }

    return NULL;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

const char *ast_node_type_name(ASTNodeType type) {
    switch (type) {
    case AST_PROGRAM:
        return "PROGRAM";
    case AST_PACKAGE_DECL:
        return "PACKAGE_DECL";
    case AST_IMPORT_DECL:
        return "IMPORT_DECL";
    case AST_VISIBILITY_MODIFIER:
        return "VISIBILITY_MODIFIER";
    case AST_FUNCTION_DECL:
        return "FUNCTION_DECL";
    case AST_STRUCT_DECL:
        return "STRUCT_DECL";
    case AST_STRUCT_FIELD:
        return "STRUCT_FIELD";
    case AST_ENUM_DECL:
        return "ENUM_DECL";
    case AST_ENUM_VARIANT_DECL:
        return "ENUM_VARIANT_DECL";
    case AST_EXTERN_DECL:
        return "EXTERN_DECL";
    case AST_PARAM_DECL:
        return "PARAM_DECL";
    case AST_BLOCK:
        return "BLOCK";
    case AST_EXPR_STMT:
        return "EXPR_STMT";
    case AST_LET_STMT:
        return "LET_STMT";
    case AST_RETURN_STMT:
        return "RETURN_STMT";
    case AST_IF_STMT:
        return "IF_STMT";
    case AST_MATCH_STMT:
        return "MATCH_STMT";
    case AST_IF_LET_STMT:
        return "IF_LET_STMT";
    case AST_SPAWN_STMT:
        return "SPAWN_STMT";
    case AST_SPAWN_WITH_HANDLE_STMT:
        return "SPAWN_WITH_HANDLE_STMT";
    case AST_AWAIT_EXPR:
        return "AWAIT_EXPR";
    // Note: Tier 3 AST types moved to stdlib:
    // AST_CHANNEL_DECL, AST_SEND_STMT, AST_RECV_EXPR, AST_SELECT_STMT,
    // AST_SELECT_CASE, AST_WORKER_POOL_DECL, AST_CLOSE_STMT,
    // AST_CHANNEL_TYPE, AST_TASK_HANDLE_TYPE, AST_WORKER_POOL_TYPE, AST_TIMEOUT_EXPR
    case AST_UNSAFE_BLOCK:
        return "UNSAFE_BLOCK";
    case AST_BINARY_EXPR:
        return "BINARY_EXPR";
    case AST_UNARY_EXPR:
        return "UNARY_EXPR";
    case AST_POSTFIX_EXPR:
        return "POSTFIX_EXPR";
    case AST_CALL_EXPR:
        return "CALL_EXPR";
    case AST_ASSOCIATED_FUNC_CALL:
        return "ASSOCIATED_FUNC_CALL";
    case AST_FIELD_ACCESS:
        return "FIELD_ACCESS";

    case AST_INDEX_ACCESS:
        return "INDEX_ACCESS";
    case AST_SLICE_LENGTH_ACCESS:
        return "SLICE_LENGTH_ACCESS";
    case AST_ASSIGNMENT:
        return "ASSIGNMENT";

    case AST_ARRAY_LITERAL:
        return "ARRAY_LITERAL";
    case AST_INTEGER_LITERAL:
        return "INTEGER_LITERAL";
    case AST_FLOAT_LITERAL:
        return "FLOAT_LITERAL";
    case AST_STRING_LITERAL:
        return "STRING_LITERAL";
    case AST_BOOL_LITERAL:
        return "BOOL_LITERAL";
    case AST_CHAR_LITERAL:
        return "CHAR_LITERAL";
    case AST_UNIT_LITERAL:
        return "UNIT_LITERAL";
    case AST_STRUCT_LITERAL:
        return "STRUCT_LITERAL";
    case AST_IDENTIFIER:
        return "IDENTIFIER";
    case AST_BASE_TYPE:
        return "BASE_TYPE";
    case AST_SLICE_TYPE:
        return "SLICE_TYPE";
    case AST_ARRAY_TYPE:
        return "ARRAY_TYPE";
    case AST_STRUCT_TYPE:
        return "STRUCT_TYPE";
    case AST_ENUM_TYPE:
        return "ENUM_TYPE";
    case AST_PTR_TYPE:
        return "PTR_TYPE";
    case AST_RESULT_TYPE:
        return "RESULT_TYPE";
    case AST_OPTION_TYPE:
        return "OPTION_TYPE";
    case AST_TASKHANDLE_TYPE:
        return "TASKHANDLE_TYPE";
    case AST_MATCH_ARM:
        return "MATCH_ARM";
    case AST_PATTERN:
        return "PATTERN";
    case AST_ENUM_PATTERN:
        return "ENUM_PATTERN";
    case AST_STRUCT_PATTERN:
        return "STRUCT_PATTERN";
    case AST_FIELD_PATTERN:
        return "FIELD_PATTERN";
    case AST_ENUM_VARIANT:
        return "ENUM_VARIANT";
    case AST_SEMANTIC_TAG:
        return "SEMANTIC_TAG";
    case AST_OWNERSHIP_TAG:
        return "OWNERSHIP_TAG";
    case AST_FFI_ANNOTATION:
        return "FFI_ANNOTATION";
    case AST_SECURITY_TAG:
        return "SECURITY_TAG";
    case AST_HUMAN_REVIEW_TAG:
        return "HUMAN_REVIEW_TAG";
    case AST_IMPL_BLOCK:
        return "IMPL_BLOCK";
    case AST_METHOD_DECL:
        return "METHOD_DECL";
    default:
        return "UNKNOWN";
    }
}

const char *binary_operator_name(BinaryOperator op) {
    switch (op) {
    case BINOP_ADD:
        return "ADD";
    case BINOP_SUB:
        return "SUB";
    case BINOP_MUL:
        return "MUL";
    case BINOP_DIV:
        return "DIV";
    case BINOP_MOD:
        return "MOD";
    case BINOP_EQ:
        return "EQ";
    case BINOP_NE:
        return "NE";
    case BINOP_LT:
        return "LT";
    case BINOP_LE:
        return "LE";
    case BINOP_GT:
        return "GT";
    case BINOP_GE:
        return "GE";
    case BINOP_AND:
        return "AND";
    case BINOP_OR:
        return "OR";
    case BINOP_BITWISE_AND:
        return "BITWISE_AND";
    case BINOP_BITWISE_OR:
        return "BITWISE_OR";
    case BINOP_BITWISE_XOR:
        return "BITWISE_XOR";
    case BINOP_LSHIFT:
        return "LSHIFT";
    case BINOP_RSHIFT:
        return "RSHIFT";
    default:
        return "UNKNOWN";
    }
}

const char *unary_operator_name(UnaryOperator op) {
    switch (op) {
    case UNOP_MINUS:
        return "MINUS";
    case UNOP_NOT:
        return "NOT";
    case UNOP_BITWISE_NOT:
        return "BITWISE_NOT";
    case UNOP_DEREF:
        return "DEREF";
    case UNOP_ADDRESS_OF:
        return "ADDRESS_OF";
    case UNOP_SIZEOF:
        return "SIZEOF";
    default:
        return "UNKNOWN";
    }
}
