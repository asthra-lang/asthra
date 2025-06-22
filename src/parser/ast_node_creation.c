/**
 * Asthra Programming Language Compiler
 * Abstract Syntax Tree (AST) Node Creation and Memory Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file contains the implementation of AST node creation, destruction,
 * and reference counting operations
 * Split from the original ast.c for better maintainability
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdatomic.h>
#include "ast.h"
#include "ast_node_creation.h"

// =============================================================================
// AST NODE CREATION
// =============================================================================

ASTNode *ast_create_node(ASTNodeType type, SourceLocation location) {
    ASTNode *node = calloc(1, sizeof(ASTNode));
    if (!node) {
        return NULL;
    }
    
    node->type = type;
    node->location = location;
    node->type_info = NULL;
    
    // Initialize atomic reference count
    atomic_init(&node->ref_count, 1);
    
    // Initialize flags
    node->flags.is_validated = false;
    node->flags.is_type_checked = false;
    node->flags.is_constant_expr = false;
    node->flags.has_side_effects = false;
    node->flags.is_lvalue = false;
    node->flags.is_mutable = false;
    node->flags.reserved = 0;
    
    return node;
}

ASTNode *ast_create_node_with_data(ASTNodeType type, SourceLocation location, ASTNodeData data) {
    ASTNode *node = ast_create_node(type, location);
    if (!node) return NULL;
    
    // Copy the appropriate data based on node type
    switch (type) {
        case AST_BINARY_EXPR:
            node->data.binary_expr.operator = data.binary_expr.operator;
            node->data.binary_expr.left = data.binary_expr.left;
            node->data.binary_expr.right = data.binary_expr.right;
            break;
        case AST_UNARY_EXPR:
            node->data.unary_expr.operator = data.unary_expr.operator;
            node->data.unary_expr.operand = data.unary_expr.operand;
            break;
        case AST_IDENTIFIER:
            node->data.identifier.name = data.identifier.name ? strdup(data.identifier.name) : NULL;
            break;
        case AST_INTEGER_LITERAL:
            node->data.integer_literal.value = data.integer_literal.value;
            break;
        case AST_FLOAT_LITERAL:
            node->data.float_literal.value = data.float_literal.value;
            break;
        case AST_STRING_LITERAL:
            node->data.string_literal.value = data.string_literal.value ? strdup(data.string_literal.value) : NULL;
            break;
        case AST_BOOL_LITERAL:
            node->data.bool_literal.value = data.bool_literal.value;
            break;
        case AST_UNIT_LITERAL:
            // Unit literal has no data to set
            break;
        default:
            // For other types, the caller should set the data manually
            break;
    }
    
    return node;
}

// =============================================================================
// REFERENCE COUNTING
// =============================================================================

ASTNode *ast_retain_node(ASTNode *node) {
    if (!node) return NULL;
    
    atomic_fetch_add_explicit(&node->ref_count, 1, memory_order_relaxed);
    return node;
}

void ast_release_node(ASTNode *node) {
    if (!node) return;
    
    uint32_t old_count = atomic_fetch_sub_explicit(&node->ref_count, 1, memory_order_acq_rel);
    if (old_count == 1) {
        // Last reference, free the node
        ast_free_node(node);
    }
}

uint32_t ast_get_ref_count(const ASTNode *node) {
    if (!node) return 0;
    return atomic_load_explicit(&node->ref_count, memory_order_acquire);
}

// Compatibility wrappers for existing code
void ast_node_ref(ASTNode *node) {
    ast_retain_node(node);
}

void ast_node_unref(ASTNode *node) {
    ast_release_node(node);
}

// =============================================================================
// NODE CLONING
// =============================================================================

ASTNode *ast_clone_node(ASTNode *node) {
    if (!node) return NULL;
    
    ASTNode *clone = ast_create_node(node->type, node->location);
    if (!clone) return NULL;
    
    // Copy type info
    clone->type_info = node->type_info;
    
    switch (node->type) {
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
            
        case AST_STRUCT_LITERAL:
            if (node->data.struct_literal.struct_name) {
                clone->data.struct_literal.struct_name = strdup(node->data.struct_literal.struct_name);
            }
            // Deep clone type_args and field_inits
            clone->data.struct_literal.type_args = ast_node_list_clone_deep(node->data.struct_literal.type_args);
            clone->data.struct_literal.field_inits = ast_node_list_clone_deep(node->data.struct_literal.field_inits);
            break;
            
        case AST_IDENTIFIER:
            if (node->data.identifier.name) {
                clone->data.identifier.name = strdup(node->data.identifier.name);
            }
            break;
            
        case AST_BINARY_EXPR:
            clone->data.binary_expr.operator = node->data.binary_expr.operator;
            clone->data.binary_expr.left = ast_clone_node(node->data.binary_expr.left);
            clone->data.binary_expr.right = ast_clone_node(node->data.binary_expr.right);
            break;
            
        case AST_UNARY_EXPR:
            clone->data.unary_expr.operator = node->data.unary_expr.operator;
            clone->data.unary_expr.operand = ast_clone_node(node->data.unary_expr.operand);
            break;
            
        case AST_ENUM_DECL:
            if (node->data.enum_decl.name) {
                clone->data.enum_decl.name = strdup(node->data.enum_decl.name);
            }
            clone->data.enum_decl.visibility = node->data.enum_decl.visibility;
            // Deep clone variants, type_params, and annotations
            clone->data.enum_decl.variants = ast_node_list_clone_deep(node->data.enum_decl.variants);
            clone->data.enum_decl.type_params = ast_node_list_clone_deep(node->data.enum_decl.type_params);
            clone->data.enum_decl.annotations = ast_node_list_clone_deep(node->data.enum_decl.annotations);
            break;
            
        case AST_ENUM_VARIANT_DECL:
            if (node->data.enum_variant_decl.name) {
                clone->data.enum_variant_decl.name = strdup(node->data.enum_variant_decl.name);
            }
            clone->data.enum_variant_decl.visibility = node->data.enum_variant_decl.visibility;
            clone->data.enum_variant_decl.associated_type = ast_clone_node(node->data.enum_variant_decl.associated_type);
            clone->data.enum_variant_decl.value = ast_clone_node(node->data.enum_variant_decl.value);
            break;
            
        case AST_PROGRAM:
            clone->data.program.package_decl = ast_clone_node(node->data.program.package_decl);
            clone->data.program.imports = ast_node_list_clone_deep(node->data.program.imports);
            clone->data.program.declarations = ast_node_list_clone_deep(node->data.program.declarations);
            break;
            
        case AST_FUNCTION_DECL:
            if (node->data.function_decl.name) {
                clone->data.function_decl.name = strdup(node->data.function_decl.name);
            }
            clone->data.function_decl.params = ast_node_list_clone_deep(node->data.function_decl.params);
            clone->data.function_decl.return_type = ast_clone_node(node->data.function_decl.return_type);
            clone->data.function_decl.body = ast_clone_node(node->data.function_decl.body);
            clone->data.function_decl.visibility = node->data.function_decl.visibility;
            clone->data.function_decl.annotations = ast_node_list_clone_deep(node->data.function_decl.annotations);
            break;
            
        case AST_STRUCT_DECL:
            if (node->data.struct_decl.name) {
                clone->data.struct_decl.name = strdup(node->data.struct_decl.name);
            }
            clone->data.struct_decl.fields = ast_node_list_clone_deep(node->data.struct_decl.fields);
            clone->data.struct_decl.type_params = ast_node_list_clone_deep(node->data.struct_decl.type_params);
            clone->data.struct_decl.visibility = node->data.struct_decl.visibility;
            clone->data.struct_decl.annotations = ast_node_list_clone_deep(node->data.struct_decl.annotations);
            break;
            
        case AST_CALL_EXPR:
            clone->data.call_expr.function = ast_clone_node(node->data.call_expr.function);
            clone->data.call_expr.args = ast_node_list_clone_deep(node->data.call_expr.args);
            break;
            
        case AST_ARRAY_LITERAL:
            clone->data.array_literal.elements = ast_node_list_clone_deep(node->data.array_literal.elements);
            break;
            
        case AST_BLOCK:
            clone->data.block.statements = ast_node_list_clone_deep(node->data.block.statements);
            break;
            
        case AST_ASSOCIATED_FUNC_CALL:
            if (node->data.associated_func_call.struct_name) {
                clone->data.associated_func_call.struct_name = strdup(node->data.associated_func_call.struct_name);
            }
            if (node->data.associated_func_call.function_name) {
                clone->data.associated_func_call.function_name = strdup(node->data.associated_func_call.function_name);
            }
            clone->data.associated_func_call.type_args = ast_node_list_clone_deep(node->data.associated_func_call.type_args);
            clone->data.associated_func_call.args = ast_node_list_clone_deep(node->data.associated_func_call.args);
            break;
            
        case AST_MATCH_STMT:
            clone->data.match_stmt.expression = ast_clone_node(node->data.match_stmt.expression);
            clone->data.match_stmt.arms = ast_node_list_clone_deep(node->data.match_stmt.arms);
            break;
            
        case AST_POSTFIX_EXPR:
            clone->data.postfix_expr.base = ast_clone_node(node->data.postfix_expr.base);
            clone->data.postfix_expr.suffixes = ast_node_list_clone_deep(node->data.postfix_expr.suffixes);
            break;
            
        case AST_TUPLE_LITERAL:
            clone->data.tuple_literal.elements = ast_node_list_clone_deep(node->data.tuple_literal.elements);
            break;
            
        case AST_TUPLE_TYPE:
            clone->data.tuple_type.element_types = ast_node_list_clone_deep(node->data.tuple_type.element_types);
            break;
            
        case AST_IMPL_BLOCK:
            if (node->data.impl_block.struct_name) {
                clone->data.impl_block.struct_name = strdup(node->data.impl_block.struct_name);
            }
            clone->data.impl_block.methods = ast_node_list_clone_deep(node->data.impl_block.methods);
            clone->data.impl_block.annotations = ast_node_list_clone_deep(node->data.impl_block.annotations);
            break;
            
        case AST_METHOD_DECL:
            if (node->data.method_decl.name) {
                clone->data.method_decl.name = strdup(node->data.method_decl.name);
            }
            clone->data.method_decl.params = ast_node_list_clone_deep(node->data.method_decl.params);
            clone->data.method_decl.return_type = ast_clone_node(node->data.method_decl.return_type);
            clone->data.method_decl.body = ast_clone_node(node->data.method_decl.body);
            clone->data.method_decl.is_instance_method = node->data.method_decl.is_instance_method;
            clone->data.method_decl.visibility = node->data.method_decl.visibility;
            clone->data.method_decl.annotations = ast_node_list_clone_deep(node->data.method_decl.annotations);
            break;
            
        case AST_SPAWN_STMT:
            if (node->data.spawn_stmt.function_name) {
                clone->data.spawn_stmt.function_name = strdup(node->data.spawn_stmt.function_name);
            }
            clone->data.spawn_stmt.args = ast_node_list_clone_deep(node->data.spawn_stmt.args);
            break;
            
        case AST_SPAWN_WITH_HANDLE_STMT:
            if (node->data.spawn_with_handle_stmt.function_name) {
                clone->data.spawn_with_handle_stmt.function_name = strdup(node->data.spawn_with_handle_stmt.function_name);
            }
            if (node->data.spawn_with_handle_stmt.handle_var_name) {
                clone->data.spawn_with_handle_stmt.handle_var_name = strdup(node->data.spawn_with_handle_stmt.handle_var_name);
            }
            clone->data.spawn_with_handle_stmt.args = ast_node_list_clone_deep(node->data.spawn_with_handle_stmt.args);
            break;
            
        case AST_STRUCT_PATTERN:
            if (node->data.struct_pattern.struct_name) {
                clone->data.struct_pattern.struct_name = strdup(node->data.struct_pattern.struct_name);
            }
            clone->data.struct_pattern.type_args = ast_node_list_clone_deep(node->data.struct_pattern.type_args);
            clone->data.struct_pattern.field_patterns = ast_node_list_clone_deep(node->data.struct_pattern.field_patterns);
            clone->data.struct_pattern.fields = ast_node_list_clone_deep(node->data.struct_pattern.fields);
            clone->data.struct_pattern.is_partial = node->data.struct_pattern.is_partial;
            break;
            
        case AST_TUPLE_PATTERN:
            clone->data.tuple_pattern.patterns = ast_node_list_clone_deep(node->data.tuple_pattern.patterns);
            break;
            
        case AST_EXTERN_DECL:
            if (node->data.extern_decl.name) {
                clone->data.extern_decl.name = strdup(node->data.extern_decl.name);
            }
            if (node->data.extern_decl.extern_name) {
                clone->data.extern_decl.extern_name = strdup(node->data.extern_decl.extern_name);
            }
            clone->data.extern_decl.params = ast_node_list_clone_deep(node->data.extern_decl.params);
            clone->data.extern_decl.return_type = ast_clone_node(node->data.extern_decl.return_type);
            clone->data.extern_decl.annotations = ast_node_list_clone_deep(node->data.extern_decl.annotations);
            break;
            
        case AST_PARAM_DECL:
            if (node->data.param_decl.name) {
                clone->data.param_decl.name = strdup(node->data.param_decl.name);
            }
            clone->data.param_decl.type = ast_clone_node(node->data.param_decl.type);
            clone->data.param_decl.annotations = ast_node_list_clone_deep(node->data.param_decl.annotations);
            break;
            
        case AST_CONST_DECL:
            if (node->data.const_decl.name) {
                clone->data.const_decl.name = strdup(node->data.const_decl.name);
            }
            clone->data.const_decl.type = ast_clone_node(node->data.const_decl.type);
            clone->data.const_decl.value = ast_clone_node(node->data.const_decl.value);
            clone->data.const_decl.visibility = node->data.const_decl.visibility;
            clone->data.const_decl.annotations = ast_node_list_clone_deep(node->data.const_decl.annotations);
            break;
            
        case AST_SEMANTIC_TAG:
            if (node->data.semantic_tag.name) {
                clone->data.semantic_tag.name = strdup(node->data.semantic_tag.name);
            }
            clone->data.semantic_tag.params = ast_node_list_clone_deep(node->data.semantic_tag.params);
            break;
            
        case AST_STRUCT_TYPE:
            if (node->data.struct_type.name) {
                clone->data.struct_type.name = strdup(node->data.struct_type.name);
            }
            clone->data.struct_type.type_args = ast_node_list_clone_deep(node->data.struct_type.type_args);
            break;
            
        case AST_ENUM_TYPE:
            if (node->data.enum_type.name) {
                clone->data.enum_type.name = strdup(node->data.enum_type.name);
            }
            clone->data.enum_type.type_args = ast_node_list_clone_deep(node->data.enum_type.type_args);
            break;
            
        // Statement node types with child nodes
        case AST_EXPR_STMT:
            clone->data.expr_stmt.expression = ast_clone_node(node->data.expr_stmt.expression);
            break;
            
        case AST_LET_STMT:
            if (node->data.let_stmt.name) {
                clone->data.let_stmt.name = strdup(node->data.let_stmt.name);
            }
            clone->data.let_stmt.type = ast_clone_node(node->data.let_stmt.type);
            clone->data.let_stmt.initializer = ast_clone_node(node->data.let_stmt.initializer);
            clone->data.let_stmt.is_mutable = node->data.let_stmt.is_mutable;
            break;
            
        case AST_RETURN_STMT:
            clone->data.return_stmt.expression = ast_clone_node(node->data.return_stmt.expression);
            break;
            
        case AST_IF_STMT:
            clone->data.if_stmt.condition = ast_clone_node(node->data.if_stmt.condition);
            clone->data.if_stmt.then_block = ast_clone_node(node->data.if_stmt.then_block);
            clone->data.if_stmt.else_block = ast_clone_node(node->data.if_stmt.else_block);
            break;
            
        case AST_FOR_STMT:
            if (node->data.for_stmt.variable) {
                clone->data.for_stmt.variable = strdup(node->data.for_stmt.variable);
            }
            clone->data.for_stmt.iterable = ast_clone_node(node->data.for_stmt.iterable);
            clone->data.for_stmt.body = ast_clone_node(node->data.for_stmt.body);
            break;
            
        case AST_IF_LET_STMT:
            clone->data.if_let_stmt.pattern = ast_clone_node(node->data.if_let_stmt.pattern);
            clone->data.if_let_stmt.expression = ast_clone_node(node->data.if_let_stmt.expression);
            clone->data.if_let_stmt.then_block = ast_clone_node(node->data.if_let_stmt.then_block);
            clone->data.if_let_stmt.else_block = ast_clone_node(node->data.if_let_stmt.else_block);
            break;
            
        case AST_UNSAFE_BLOCK:
            clone->data.unsafe_block.block = ast_clone_node(node->data.unsafe_block.block);
            break;
            
        // Expression node types with child nodes
        case AST_FIELD_ACCESS:
            clone->data.field_access.object = ast_clone_node(node->data.field_access.object);
            if (node->data.field_access.field_name) {
                clone->data.field_access.field_name = strdup(node->data.field_access.field_name);
            }
            break;
            
        case AST_INDEX_ACCESS:
            clone->data.index_access.array = ast_clone_node(node->data.index_access.array);
            clone->data.index_access.index = ast_clone_node(node->data.index_access.index);
            break;
            
        case AST_SLICE_EXPR:
            clone->data.slice_expr.array = ast_clone_node(node->data.slice_expr.array);
            clone->data.slice_expr.start = ast_clone_node(node->data.slice_expr.start);
            clone->data.slice_expr.end = ast_clone_node(node->data.slice_expr.end);
            break;
            
        case AST_SLICE_LENGTH_ACCESS:
            clone->data.slice_length_access.slice = ast_clone_node(node->data.slice_length_access.slice);
            break;
            
        case AST_ASSIGNMENT:
            clone->data.assignment.target = ast_clone_node(node->data.assignment.target);
            clone->data.assignment.value = ast_clone_node(node->data.assignment.value);
            break;
            
        case AST_AWAIT_EXPR:
            clone->data.await_expr.task_handle_expr = ast_clone_node(node->data.await_expr.task_handle_expr);
            clone->data.await_expr.timeout_expr = ast_clone_node(node->data.await_expr.timeout_expr);
            break;
            
        // Type node types with child nodes
        case AST_BASE_TYPE:
            if (node->data.base_type.name) {
                clone->data.base_type.name = strdup(node->data.base_type.name);
            }
            break;
            
        case AST_SLICE_TYPE:
            clone->data.slice_type.element_type = ast_clone_node(node->data.slice_type.element_type);
            break;
            
        case AST_ARRAY_TYPE:
            clone->data.array_type.element_type = ast_clone_node(node->data.array_type.element_type);
            clone->data.array_type.size = ast_clone_node(node->data.array_type.size);
            break;
            
        case AST_PTR_TYPE:
            clone->data.ptr_type.is_mutable = node->data.ptr_type.is_mutable;
            clone->data.ptr_type.pointee_type = ast_clone_node(node->data.ptr_type.pointee_type);
            break;
            
        case AST_RESULT_TYPE:
            clone->data.result_type.ok_type = ast_clone_node(node->data.result_type.ok_type);
            clone->data.result_type.err_type = ast_clone_node(node->data.result_type.err_type);
            break;
            
        case AST_OPTION_TYPE:
            clone->data.option_type.value_type = ast_clone_node(node->data.option_type.value_type);
            break;
            
        // Pattern node types
        case AST_MATCH_ARM:
            clone->data.match_arm.pattern = ast_clone_node(node->data.match_arm.pattern);
            clone->data.match_arm.body = ast_clone_node(node->data.match_arm.body);
            clone->data.match_arm.guard = ast_clone_node(node->data.match_arm.guard);
            break;
            
        case AST_ENUM_PATTERN:
            if (node->data.enum_pattern.enum_name) {
                clone->data.enum_pattern.enum_name = strdup(node->data.enum_pattern.enum_name);
            }
            if (node->data.enum_pattern.variant_name) {
                clone->data.enum_pattern.variant_name = strdup(node->data.enum_pattern.variant_name);
            }
            if (node->data.enum_pattern.binding) {
                clone->data.enum_pattern.binding = strdup(node->data.enum_pattern.binding);
            }
            break;
            
        case AST_FIELD_PATTERN:
            if (node->data.field_pattern.field_name) {
                clone->data.field_pattern.field_name = strdup(node->data.field_pattern.field_name);
            }
            if (node->data.field_pattern.binding_name) {
                clone->data.field_pattern.binding_name = strdup(node->data.field_pattern.binding_name);
            }
            clone->data.field_pattern.is_ignored = node->data.field_pattern.is_ignored;
            clone->data.field_pattern.pattern = ast_clone_node(node->data.field_pattern.pattern);
            break;
            
        case AST_ENUM_VARIANT:
            if (node->data.enum_variant.enum_name) {
                clone->data.enum_variant.enum_name = strdup(node->data.enum_variant.enum_name);
            }
            if (node->data.enum_variant.variant_name) {
                clone->data.enum_variant.variant_name = strdup(node->data.enum_variant.variant_name);
            }
            clone->data.enum_variant.value = ast_clone_node(node->data.enum_variant.value);
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
            
        case AST_STRUCT_FIELD:
            if (node->data.struct_field.name) {
                clone->data.struct_field.name = strdup(node->data.struct_field.name);
            }
            clone->data.struct_field.type = ast_clone_node(node->data.struct_field.type);
            clone->data.struct_field.visibility = node->data.struct_field.visibility;
            break;
            
        case AST_CONST_EXPR:
            clone->data.const_expr.expr_type = node->data.const_expr.expr_type;
            switch (node->data.const_expr.expr_type) {
                case CONST_EXPR_LITERAL:
                    clone->data.const_expr.data.literal = ast_clone_node(node->data.const_expr.data.literal);
                    break;
                case CONST_EXPR_BINARY_OP:
                    clone->data.const_expr.data.binary.left = ast_clone_node(node->data.const_expr.data.binary.left);
                    clone->data.const_expr.data.binary.op = node->data.const_expr.data.binary.op;
                    clone->data.const_expr.data.binary.right = ast_clone_node(node->data.const_expr.data.binary.right);
                    break;
                case CONST_EXPR_UNARY_OP:
                    clone->data.const_expr.data.unary.op = node->data.const_expr.data.unary.op;
                    clone->data.const_expr.data.unary.operand = ast_clone_node(node->data.const_expr.data.unary.operand);
                    break;
                case CONST_EXPR_SIZEOF:
                    clone->data.const_expr.data.sizeof_expr.type = ast_clone_node(node->data.const_expr.data.sizeof_expr.type);
                    break;
                case CONST_EXPR_IDENTIFIER:
                    if (node->data.const_expr.data.identifier) {
                        clone->data.const_expr.data.identifier = strdup(node->data.const_expr.data.identifier);
                    }
                    break;
            }
            break;
            
        case AST_BREAK_STMT:
        case AST_CONTINUE_STMT:
        case AST_WILDCARD_PATTERN:
            // These have no data to clone
            break;
            
        // Add more cases as needed for other node types
        default:
            // For now, just copy the data union directly for simple cases
            clone->data = node->data;
            break;
    }
    
    return clone;
} 
