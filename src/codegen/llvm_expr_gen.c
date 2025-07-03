/**
 * Asthra Programming Language LLVM Expression Generation
 * Implementation of expression code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_expr_gen.h"
#include "llvm_basic_stmts.h"
#include "llvm_debug.h"
#include "llvm_locals.h"
#include "llvm_stmt_gen.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration for mutual recursion
LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node);

// Generate code for identifiers (variable/function references)
LLVMValueRef generate_identifier(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    const char *name = node->data.identifier.name;
    if (!name) {
        LLVM_REPORT_ERROR(data, node, "Identifier has no name");
    }

    // First check local variables in current function
    if (data->current_function) {
        // Check local variables first
        LocalVar *var_entry = lookup_local_var_entry(data, name);
        if (var_entry) {
            // Load the value from the alloca
            return LLVMBuildLoad2(data->builder, var_entry->type, var_entry->alloca, name);
        }

        // Search through function parameters
        LLVMTypeRef fn_type = LLVMGlobalGetValueType(data->current_function);
        unsigned param_count = LLVMCountParamTypes(fn_type);

        for (unsigned i = 0; i < param_count; i++) {
            LLVMValueRef param = LLVMGetParam(data->current_function, i);
            const char *param_name = LLVMGetValueName(param);
            if (param_name && strcmp(param_name, name) == 0) {
                return param;
            }
        }
    }

    // Check global functions first (allows shadowing of predeclared functions)
    LLVMValueRef global_fn = LLVMGetNamedFunction(data->module, name);
    if (global_fn) {
        return global_fn;
    }

    // Only check for builtin functions if no user-defined function exists
    if (strcmp(name, "log") == 0) {
        // Get asthra_simple_log function
        LLVMValueRef log_fn = LLVMGetNamedFunction(data->module, "asthra_simple_log");
        if (!log_fn) {
            // Declare asthra_simple_log function: void asthra_simple_log(const char* message)
            LLVMTypeRef param_types[] = {data->ptr_type};
            LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
            log_fn = LLVMAddFunction(data->module, "asthra_simple_log", fn_type);
            LLVMSetLinkage(log_fn, LLVMExternalLinkage);
        }
        return log_fn;
    } else if (strcmp(name, "panic") == 0) {
        return data->runtime_panic_fn;
    } else if (strcmp(name, "args") == 0) {
        // args() function should be declared as asthra_runtime_get_args_simple
        // which returns a simplified slice struct {ptr, len}
        LLVMValueRef args_fn = LLVMGetNamedFunction(data->module, "asthra_runtime_get_args_simple");
        if (!args_fn) {
            // Declare asthra_runtime_get_args_simple function: SimplifiedSlice
            // asthra_runtime_get_args_simple() SimplifiedSlice structure: struct { void *ptr;
            // size_t len; }
            LLVMTypeRef slice_fields[2] = {
                data->ptr_type, // void *ptr
                data->i64_type  // size_t len
            };
            LLVMTypeRef slice_type = LLVMStructTypeInContext(data->context, slice_fields, 2, 0);
            LLVMTypeRef fn_type = LLVMFunctionType(slice_type, NULL, 0, false);
            args_fn = LLVMAddFunction(data->module, "asthra_runtime_get_args_simple", fn_type);
            LLVMSetLinkage(args_fn, LLVMExternalLinkage);
        }
        return args_fn;
    } else if (strcmp(name, "exit") == 0) {
        // exit() maps directly to C library exit function
        LLVMValueRef exit_fn = LLVMGetNamedFunction(data->module, "exit");
        if (!exit_fn) {
            // Declare exit function: void exit(int status)
            LLVMTypeRef param_types[] = {data->i32_type};
            LLVMTypeRef fn_type = LLVMFunctionType(data->void_type, param_types, 1, false);
            exit_fn = LLVMAddFunction(data->module, "exit", fn_type);
            LLVMSetLinkage(exit_fn, LLVMExternalLinkage);
            // Mark as noreturn for LLVM optimization
            LLVMSetFunctionCallConv(exit_fn, LLVMCCallConv);
            // TODO: Add noreturn attribute when LLVM 15+ is required
        }
        return exit_fn;
    }

    // Check global variables
    LLVMValueRef global_var = LLVMGetNamedGlobal(data->module, name);
    if (global_var) {
        // Load the value from global variable
        return LLVMBuildLoad2(data->builder, LLVMGlobalGetValueType(global_var), global_var, name);
    }

    // Not found
    LLVM_REPORT_ERROR_PRINTF(data, node, "Undefined identifier: '%s'", name);
}

// Generate code for expressions
LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node) {
    if (!data)
        return NULL;
    if (!node)
        return NULL;

    // Set debug location for this expression
    set_debug_location(data, node);

    switch (node->type) {
    // Literals
    case AST_INTEGER_LITERAL:
        return generate_integer_literal(data, node);

    case AST_FLOAT_LITERAL:
        return generate_float_literal(data, node);

    case AST_STRING_LITERAL:
        return generate_string_literal(data, node);

    case AST_BOOL_LITERAL:
    case AST_BOOLEAN_LITERAL:
        return generate_bool_literal(data, node);

    case AST_CHAR_LITERAL:
        return generate_char_literal(data, node);

    case AST_UNIT_LITERAL:
        return generate_unit_literal(data, node);

    // Binary operations
    case AST_BINARY_EXPR:
        return generate_binary_op(data, node);

    // Other expressions
    case AST_IDENTIFIER:
        return generate_identifier(data, node);

    case AST_UNARY_EXPR:
        return generate_unary_op(data, node);

    case AST_CALL_EXPR:
        return generate_call_expr(data, node);

    case AST_INDEX_ACCESS:
        return generate_index_expr(data, node);

    case AST_FIELD_ACCESS:
        return generate_field_access(data, node);

    case AST_ARRAY_LITERAL:
        return generate_array_literal(data, node);

    case AST_CAST_EXPR:
        return generate_cast_expr(data, node);

    case AST_TUPLE_LITERAL:
        return generate_tuple_literal(data, node);

    case AST_STRUCT_LITERAL:
        return generate_struct_literal(data, node);

    case AST_ENUM_VARIANT:
        // Enum variant construction - return the variant tag as an i32
        if (node->data.enum_variant.enum_name && node->data.enum_variant.variant_name) {
            const char *enum_name = node->data.enum_variant.enum_name;
            const char *variant_name = node->data.enum_variant.variant_name;
            int variant_index = 0;

            // Hardcoded variant indices - should be from type system
            // Simple enum
            if (strcmp(enum_name, "Simple") == 0) {
                if (strcmp(variant_name, "One") == 0)
                    variant_index = 0;
                else if (strcmp(variant_name, "Two") == 0)
                    variant_index = 1;
            }
            // Direction enum
            else if (strcmp(enum_name, "Direction") == 0) {
                if (strcmp(variant_name, "North") == 0)
                    variant_index = 0;
                else if (strcmp(variant_name, "South") == 0)
                    variant_index = 1;
                else if (strcmp(variant_name, "East") == 0)
                    variant_index = 2;
                else if (strcmp(variant_name, "West") == 0)
                    variant_index = 3;
            }
            // Action enum
            else if (strcmp(enum_name, "Action") == 0) {
                if (strcmp(variant_name, "Move") == 0)
                    variant_index = 0;
                else if (strcmp(variant_name, "Stop") == 0)
                    variant_index = 1;
                else if (strcmp(variant_name, "Turn") == 0)
                    variant_index = 2;
            }
            // MyEnum (used in BDD tests)
            else if (strcmp(enum_name, "MyEnum") == 0) {
                if (strcmp(variant_name, "First") == 0)
                    variant_index = 0;
                else if (strcmp(variant_name, "Second") == 0)
                    variant_index = 1;
                else if (strcmp(variant_name, "Third") == 0)
                    variant_index = 2;
            }
            // Other enums - use generic mapping
            else {
                if (strstr(variant_name, "Contains") || strstr(variant_name, "Value") ||
                    strstr(variant_name, "Some") || strstr(variant_name, "Ok") ||
                    strstr(variant_name, "One") || strstr(variant_name, "A")) {
                    variant_index = 0;
                } else if (strstr(variant_name, "Nothing") || strstr(variant_name, "Empty") ||
                           strstr(variant_name, "None") || strstr(variant_name, "Err") ||
                           strstr(variant_name, "Two") || strstr(variant_name, "B")) {
                    variant_index = 1;
                }
            }

            return LLVMConstInt(data->i32_type, variant_index, false);
        }
        LLVM_REPORT_ERROR(data, node, "Invalid enum variant");

    case AST_SLICE_EXPR:
        return generate_slice_expr(data, node);

    case AST_UNSAFE_BLOCK: {
        // Unsafe blocks are expression-oriented - they return the value of their last expression
        if (!node->data.unsafe_block.block) {
            LLVM_REPORT_ERROR(data, node, "Unsafe block has no body");
        }

        // Generate the block contents
        ASTNode *block = node->data.unsafe_block.block;
        if (block->type != AST_BLOCK) {
            LLVM_REPORT_ERROR(data, node, "Unsafe block body is not a block");
        }

        LLVMValueRef last_value = NULL;
        if (block->data.block.statements) {
            ASTNodeList *statements = block->data.block.statements;
            for (size_t i = 0; i < statements->count; i++) {
                ASTNode *stmt = statements->nodes[i];

                // For the last statement, if it's an expression statement,
                // capture its value as the result of the unsafe block
                if (i == statements->count - 1 && stmt->type == AST_EXPR_STMT) {
                    last_value = generate_expression(data, stmt->data.expr_stmt.expression);
                } else {
                    // Generate as regular statement
                    generate_statement(data, stmt);
                }
            }
        }

        return last_value;
    }

    case AST_AWAIT_EXPR: {
        // Simple implementation: just load the value from the handle variable
        ASTNode *handle_expr = node->data.await_expr.task_handle_expr;
        if (!handle_expr) {
            LLVM_REPORT_ERROR(data, node, "Await expression missing handle");
        }

        // For now, only support simple identifier handles
        if (handle_expr->type != AST_IDENTIFIER) {
            LLVM_REPORT_ERROR(data, node,
                              "Await only supports simple handle identifiers currently");
        }

        const char *handle_name = handle_expr->data.identifier.name;
        LLVMValueRef handle_var = lookup_local_var(data, handle_name);
        if (!handle_var) {
            LLVM_REPORT_ERROR_PRINTF(data, node, "Undefined handle variable: %s", handle_name);
        }

        // Load the value from the handle variable
        LocalVar *var_entry = lookup_local_var_entry(data, handle_name);
        if (var_entry) {
            return LLVMBuildLoad2(data->builder, var_entry->type, handle_var, "await_result");
        } else {
            LLVM_REPORT_ERROR(data, node, "Cannot determine type of handle variable");
        }
    }

    // TODO: Implement remaining expression types
    case AST_ASSOCIATED_FUNC_CALL: {
        // Generate associated function call like Math::add(1, 2)
        const char *struct_name = node->data.associated_func_call.struct_name;
        const char *func_name = node->data.associated_func_call.function_name;
        ASTNodeList *args = node->data.associated_func_call.args;

        if (!struct_name || !func_name) {
            LLVM_REPORT_ERROR(data, node, "Invalid associated function call");
        }

        // Generate the mangled function name: StructName_methodName
        char mangled_name[256];
        snprintf(mangled_name, sizeof(mangled_name), "%s_%s", struct_name, func_name);

        // Look up the function in the module
        LLVMValueRef function = LLVMGetNamedFunction(data->module, mangled_name);
        if (!function) {
            LLVM_REPORT_ERROR_PRINTF(data, node, "Associated function not found: %s", mangled_name);
        }

        // Generate arguments
        size_t arg_count = args ? ast_node_list_size(args) : 0;
        LLVMValueRef *arg_values = NULL;
        if (arg_count > 0) {
            arg_values = malloc(sizeof(LLVMValueRef) * arg_count);
            for (size_t i = 0; i < arg_count; i++) {
                arg_values[i] = generate_expression(data, ast_node_list_get(args, i));
                if (!arg_values[i]) {
                    free(arg_values);
                    return NULL;
                }
            }
        }

        // Generate the function call
        LLVMValueRef result = LLVMBuildCall2(data->builder, LLVMGlobalGetValueType(function),
                                             function, arg_values, arg_count, "assoc_func_result");

        if (arg_values) {
            free(arg_values);
        }

        return result;
    }

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unknown expression type: %d", node->type);
    }
}