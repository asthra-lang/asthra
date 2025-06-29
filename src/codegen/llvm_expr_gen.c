/**
 * Asthra Programming Language LLVM Expression Generation
 * Implementation of expression code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_expr_gen.h"
#include "llvm_debug.h"
#include "llvm_locals.h"
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

    // Check for builtin functions and map to runtime functions
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
        // args() function should be declared as asthra_runtime_get_args
        LLVMValueRef args_fn = LLVMGetNamedFunction(data->module, "asthra_runtime_get_args");
        if (!args_fn) {
            // Declare asthra_runtime_get_args function: []string asthra_runtime_get_args()
            LLVMTypeRef slice_fields[2] = {data->ptr_type, data->i64_type};
            LLVMTypeRef slice_type = LLVMStructTypeInContext(data->context, slice_fields, 2, 0);
            LLVMTypeRef fn_type = LLVMFunctionType(slice_type, NULL, 0, false);
            args_fn = LLVMAddFunction(data->module, "asthra_runtime_get_args", fn_type);
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

    // Check global functions
    LLVMValueRef global_fn = LLVMGetNamedFunction(data->module, name);
    if (global_fn) {
        return global_fn;
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

    // TODO: Implement remaining expression types
    case AST_STRUCT_LITERAL:
    case AST_SLICE_EXPR:
    case AST_ASSOCIATED_FUNC_CALL:
        // Not yet implemented
        LLVM_REPORT_ERROR_PRINTF(data, node, "Expression type not yet implemented: %d", node->type);

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unknown expression type: %d", node->type);
    }
}