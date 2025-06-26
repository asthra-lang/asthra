/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Type Inference
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Expression type inference and analysis
 */

#include "type_inference.h"
#include "semantic_errors.h"
#include "semantic_macros.h"
#include "semantic_symbols.h"
#include "semantic_type_creation.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "symbol_utilities.h"
#include "type_checking.h"
#include "type_info.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TYPE INFERENCE
// =============================================================================

TypeDescriptor *semantic_get_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr) {
    if (!analyzer || !expr) {
        return NULL;
    }

    // First check if the expression already has type information attached
    if (expr->type_info && expr->type_info->type_descriptor) {
        TypeDescriptor *type = (TypeDescriptor *)expr->type_info->type_descriptor;
        type_descriptor_retain(type);
        return type;
    }

    switch (expr->type) {
    case AST_IDENTIFIER: {
        // Try to resolve the identifier in the symbol table
        const char *identifier_name = expr->data.identifier.name;
        if (identifier_name) {
            SymbolEntry *symbol = semantic_resolve_identifier(analyzer, identifier_name);
            if (symbol && symbol->type) {
                type_descriptor_retain(symbol->type);
                return symbol->type;
            }
        }
        return NULL;
    }

    case AST_INTEGER_LITERAL: {
        // TODO: More sophisticated type inference based on value and context
        // For now, default to i32
        TypeDescriptor *i32_type = semantic_get_builtin_type(analyzer, "i32");
        if (i32_type) {
            type_descriptor_retain(i32_type);
        }
        return i32_type;
    }

    case AST_FLOAT_LITERAL: {
        // Default to f64 for float literals
        TypeDescriptor *f64_type = semantic_get_builtin_type(analyzer, "f64");
        if (f64_type) {
            type_descriptor_retain(f64_type);
        }
        return f64_type;
    }

    case AST_STRING_LITERAL: {
        TypeDescriptor *string_type = semantic_get_builtin_type(analyzer, "string");
        if (string_type) {
            type_descriptor_retain(string_type);
        }
        return string_type;
    }

    case AST_BOOL_LITERAL: {
        TypeDescriptor *bool_type = semantic_get_builtin_type(analyzer, "bool");
        if (bool_type) {
            type_descriptor_retain(bool_type);
        }
        return bool_type;
    }

    case AST_UNIT_LITERAL: {
        TypeDescriptor *void_type = semantic_get_builtin_type(analyzer, "void");
        if (void_type) {
            type_descriptor_retain(void_type);
        }
        return void_type;
    }

    case AST_CALL_EXPR: {
        // For function calls, return the return type of the function
        ASTNode *function = expr->data.call_expr.function;
        if (!function)
            return NULL;

        TypeDescriptor *func_type = semantic_get_expression_type(analyzer, function);
        if (!func_type || func_type->category != TYPE_FUNCTION) {
            if (func_type)
                type_descriptor_release(func_type);
            return NULL;
        }

        TypeDescriptor *return_type = func_type->data.function.return_type;
        if (return_type) {
            type_descriptor_retain(return_type);
        }
        type_descriptor_release(func_type);
        return return_type;
    }

    case AST_FIELD_ACCESS: {
        // For field access, get the type of the accessed field
        ASTNode *object = expr->data.field_access.object;
        const char *field_name = expr->data.field_access.field_name;

        if (!object || !field_name)
            return NULL;

        // Get the type of the object
        TypeDescriptor *object_type = semantic_get_expression_type(analyzer, object);
        if (!object_type)
            return NULL;

        // Check if the object is a struct
        if (object_type->category != TYPE_STRUCT) {
            type_descriptor_release(object_type);
            return NULL;
        }

        // Look up the field in the struct's field table
        if (object_type->data.struct_type.fields) {
            SymbolEntry *field_symbol =
                symbol_table_lookup_safe(object_type->data.struct_type.fields, field_name);

            if (field_symbol && field_symbol->type) {
                // Return the field's type
                TypeDescriptor *field_type = field_symbol->type;
                type_descriptor_retain(field_type);
                type_descriptor_release(object_type);
                return field_type;
            }
        }

        type_descriptor_release(object_type);
        return NULL;
    }

    case AST_ARRAY_LITERAL: {
        // For array literals, infer type based on elements
        if (!expr->data.array_literal.elements) {
            return NULL; // Empty array - cannot infer type
        }

        size_t element_count = ast_node_list_size(expr->data.array_literal.elements);
        if (element_count == 0) {
            return NULL; // Empty array - cannot infer type
        }

        // Get type of first element
        ASTNode *first_element = ast_node_list_get(expr->data.array_literal.elements, 0);
        TypeDescriptor *element_type = semantic_get_expression_type(analyzer, first_element);
        if (!element_type) {
            return NULL;
        }

        // Create fixed-size array type for the array literal
        TypeDescriptor *array_type = type_descriptor_create_array(element_type, element_count);
        type_descriptor_release(element_type);

        return array_type;
    }

    case AST_BINARY_EXPR: {
        // For binary expressions, analyze the operands and determine result type
        ASTNode *left = expr->data.binary_expr.left;
        ASTNode *right = expr->data.binary_expr.right;

        if (!left || !right)
            return NULL;

        TypeDescriptor *left_type = semantic_get_expression_type(analyzer, left);
        TypeDescriptor *right_type = semantic_get_expression_type(analyzer, right);

        if (!left_type || !right_type) {
            if (left_type)
                type_descriptor_release(left_type);
            if (right_type)
                type_descriptor_release(right_type);
            return NULL;
        }

        // For now, return the "wider" type between left and right
        // This is a simplified implementation
        TypeDescriptor *result_type = NULL;
        if (semantic_check_type_compatibility(analyzer, left_type, right_type)) {
            // Left type is compatible with right type, use right type
            result_type = right_type;
        } else {
            // Either right is compatible with left, or types are incompatible
            // In both cases, we default to left type
            // TODO: Report error when types are truly incompatible
            result_type = left_type;
        }

        type_descriptor_retain(result_type);
        type_descriptor_release(left_type);
        type_descriptor_release(right_type);
        return result_type;
    }

    case AST_ASSIGNMENT: {
        // Assignment expressions have the type of the assigned value
        ASTNode *value = expr->data.assignment.value;
        if (value) {
            return semantic_get_expression_type(analyzer, value);
        }
        return NULL;
    }

    case AST_STRUCT_LITERAL: {
        // For struct literals, resolve the struct type (with generic instantiation if needed)
        const char *struct_name = expr->data.struct_literal.struct_name;
        ASTNodeList *type_args = expr->data.struct_literal.type_args;

        if (!struct_name)
            return NULL;

        // Look up the struct type
        SymbolEntry *struct_symbol = symbol_table_lookup_safe(analyzer->current_scope, struct_name);
        if (!struct_symbol || struct_symbol->kind != SYMBOL_TYPE || !struct_symbol->type) {
            return NULL;
        }

        TypeDescriptor *struct_type = struct_symbol->type;
        if (struct_type->category != TYPE_STRUCT) {
            return NULL;
        }

        // If no type arguments, return the base struct type
        if (!type_args) {
            type_descriptor_retain(struct_type);
            return struct_type;
        }

        // Handle generic instantiation
        size_t arg_count = ast_node_list_size(type_args);
        TypeDescriptor **arg_types = malloc(arg_count * sizeof(TypeDescriptor *));
        if (!arg_types) {
            return NULL;
        }

        // Analyze each type argument
        for (size_t i = 0; i < arg_count; i++) {
            ASTNode *type_arg = ast_node_list_get(type_args, i);
            if (type_arg) {
                TypeDescriptor *arg_type = analyze_type_node(analyzer, type_arg);
                if (!arg_type) {
                    // Clean up previously analyzed arguments
                    for (size_t j = 0; j < i; j++) {
                        if (arg_types[j]) {
                            type_descriptor_release(arg_types[j]);
                        }
                    }
                    free(arg_types);
                    return NULL;
                }
                arg_types[i] = arg_type;
            } else {
                arg_types[i] = NULL;
            }
        }

        // Create the generic instance type
        TypeDescriptor *instance_type =
            type_descriptor_create_generic_instance(struct_type, arg_types, arg_count);

        // Release the type arguments (instance retains them)
        for (size_t i = 0; i < arg_count; i++) {
            if (arg_types[i]) {
                type_descriptor_release(arg_types[i]);
            }
        }
        free(arg_types);

        return instance_type;
    }

    case AST_IF_STMT: {
        // If-else expressions must have both branches with the same type
        ASTNode *then_block = expr->data.if_stmt.then_block;
        ASTNode *else_block = expr->data.if_stmt.else_block;

        if (!else_block) {
            // If without else has unit type (void)
            TypeDescriptor *void_type = semantic_get_builtin_type(analyzer, "void");
            if (void_type) {
                type_descriptor_retain(void_type);
            }
            return void_type;
        }

        // Get the type of the then branch
        TypeDescriptor *then_type = NULL;
        if (then_block && then_block->type == AST_BLOCK) {
            // For blocks, get the type of the last statement/expression
            ASTNodeList *statements = then_block->data.block.statements;
            if (statements && ast_node_list_size(statements) > 0) {
                // Check if block ends with an expression
                size_t last_idx = ast_node_list_size(statements) - 1;
                ASTNode *last_stmt = ast_node_list_get(statements, last_idx);
                if (last_stmt) {
                    then_type = semantic_get_expression_type(analyzer, last_stmt);
                }
            }

            // If no expression type found, it's void
            if (!then_type) {
                then_type = semantic_get_builtin_type(analyzer, "void");
                if (then_type) {
                    type_descriptor_retain(then_type);
                }
            }
        } else {
            then_type = semantic_get_expression_type(analyzer, then_block);
        }

        // Get the type of the else branch
        TypeDescriptor *else_type = NULL;
        if (else_block && else_block->type == AST_BLOCK) {
            // For blocks, get the type of the last statement/expression
            ASTNodeList *statements = else_block->data.block.statements;
            if (statements && ast_node_list_size(statements) > 0) {
                // Check if block ends with an expression
                size_t last_idx = ast_node_list_size(statements) - 1;
                ASTNode *last_stmt = ast_node_list_get(statements, last_idx);
                if (last_stmt) {
                    else_type = semantic_get_expression_type(analyzer, last_stmt);
                }
            }

            // If no expression type found, it's void
            if (!else_type) {
                else_type = semantic_get_builtin_type(analyzer, "void");
                if (else_type) {
                    type_descriptor_retain(else_type);
                }
            }
        } else {
            else_type = semantic_get_expression_type(analyzer, else_block);
        }

        // Check if both types are available
        if (!then_type || !else_type) {
            if (then_type)
                type_descriptor_release(then_type);
            if (else_type)
                type_descriptor_release(else_type);
            return NULL;
        }

        // Check if types match
        bool types_match = type_descriptor_equals(then_type, else_type);
        if (!types_match) {
            // For now, just return NULL on type mismatch
            // A proper implementation would add an error
            type_descriptor_release(then_type);
            type_descriptor_release(else_type);
            return NULL;
        }

        // Release the else_type since we're returning then_type
        type_descriptor_release(else_type);
        return then_type;
    }

    default:
        // For unknown expression types, return NULL
        return NULL;
    }
}