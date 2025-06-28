/**
 * Asthra Programming Language LLVM Function Calls
 * Implementation of function call expression code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_call_expr.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generate code for function calls
LLVMValueRef generate_call_expr(LLVMBackendData *data, const ASTNode *node) {
    // Get the function to call
    if (node->data.call_expr.function) {
        if (node->data.call_expr.function->type == AST_FIELD_ACCESS) {
            // Check if this is an instance method call (object.method())
            // We need to check this BEFORE calling generate_expression
            ASTNode *field_access = node->data.call_expr.function;

            // Try to generate the object
            LLVMValueRef self_obj =
                generate_expression(data, field_access->data.field_access.object);
            if (self_obj) {
                // This is likely an instance method call
                const char *method_name = field_access->data.field_access.field_name;

                // Look up the method - for now, we'll use a simple naming convention
                char mangled_name[256];

                // Get the type name from the object's type info
                const char *type_name = "Point"; // TODO: Get from type info
                snprintf(mangled_name, sizeof(mangled_name), "%s_instance_%s", type_name,
                         method_name);

                LLVMValueRef function = LLVMGetNamedFunction(data->module, mangled_name);

                if (!function) {
                    // Try without instance prefix
                    snprintf(mangled_name, sizeof(mangled_name), "%s_%s", type_name, method_name);
                    function = LLVMGetNamedFunction(data->module, mangled_name);
                }

                if (!function) {
                    // Try just the method name (in case semantic analyzer used that)
                    function = LLVMGetNamedFunction(data->module, method_name);
                }

                if (!function) {
                    return NULL;
                }

                // For instance methods, we need to add self as the first parameter
                // Prepare arguments with self as first
                size_t arg_count = node->data.call_expr.args ? node->data.call_expr.args->count : 0;
                size_t total_args = arg_count + 1; // +1 for self
                LLVMValueRef *args = malloc(total_args * sizeof(LLVMValueRef));

                // First argument is self
                args[0] = self_obj;

                // Generate remaining arguments
                for (size_t i = 0; i < arg_count; i++) {
                    args[i + 1] = generate_expression(data, node->data.call_expr.args->nodes[i]);
                    if (!args[i + 1]) {
                        free(args);
                        return NULL;
                    }
                }

                // Build the call
                LLVMTypeRef fn_type = LLVMGlobalGetValueType(function);
                if (!fn_type) {
                    free(args);
                    return NULL;
                }

                // Only name the result if the function returns a non-void value
                const char *method_result_name = "";
                if (fn_type && LLVMGetReturnType(fn_type) != data->void_type) {
                    method_result_name = "method_call";
                }

                LLVMValueRef result = LLVMBuildCall2(data->builder, fn_type, function, args,
                                                     (unsigned)total_args, method_result_name);
                free(args);
                return result;
            }
        }
    }

    LLVMValueRef function = generate_expression(data, node->data.call_expr.function);
    if (!function) {
        // Check if this is an associated function call like Option.Some
        if (node->data.call_expr.function->type == AST_FIELD_ACCESS) {
            ASTNode *field_access = node->data.call_expr.function;
            if (field_access->data.field_access.object->type == AST_IDENTIFIER) {
                const char *type_name =
                    field_access->data.field_access.object->data.identifier.name;
                const char *func_name = field_access->data.field_access.field_name;

                // Handle Option.Some and Option.None
                if (strcmp(type_name, "Option") == 0) {
                    return generate_option_function_call(data, node, func_name);
                }
                // Handle Result.Ok and Result.Err
                if (strcmp(type_name, "Result") == 0) {
                    return generate_result_function_call(data, node, func_name);
                }
            }
        }

        return NULL;
    }

    // Generate arguments
    size_t arg_count = node->data.call_expr.args ? node->data.call_expr.args->count : 0;
    LLVMValueRef *args = NULL;
    size_t actual_arg_count = arg_count;

    // Regular function call - no special handling needed
    if (arg_count > 0) {
        args = malloc(arg_count * sizeof(LLVMValueRef));
        for (size_t i = 0; i < arg_count; i++) {
            args[i] = generate_expression(data, node->data.call_expr.args->nodes[i]);
            if (!args[i]) {
                free(args);
                return NULL;
            }
        }
    }

    // Build the call
    // Get function type from the global function
    LLVMTypeRef fn_type = LLVMGlobalGetValueType(function);
    if (!fn_type) {
        // Fallback: try to get it from the function pointer type
        LLVMTypeRef fn_ptr_type = LLVMTypeOf(function);
        if (LLVMGetTypeKind(fn_ptr_type) == LLVMPointerTypeKind) {
            fn_type = LLVMGetElementType(fn_ptr_type);
        }
    }

    if (!fn_type) {
        if (args)
            free(args);
        return NULL;
    }

    // Only name the result if the function returns a non-void value
    const char *result_name = "";
    if (fn_type && LLVMGetReturnType(fn_type) != data->void_type) {
        result_name = "call";
    }

    LLVMValueRef result = LLVMBuildCall2(data->builder, fn_type, function, args,
                                         (unsigned)actual_arg_count, result_name);

    if (args)
        free(args);
    return result;
}

// Generate code for Option.Some and Option.None function calls
LLVMValueRef generate_option_function_call(LLVMBackendData *data, const ASTNode *node,
                                           const char *func_name) {
    if (strcmp(func_name, "Some") == 0) {
        // Option.Some(value) - create an Option struct
        if (node->data.call_expr.args && node->data.call_expr.args->count == 1) {
            // Generate the value
            LLVMValueRef value = generate_expression(data, node->data.call_expr.args->nodes[0]);
            if (!value)
                return NULL;

            // Create Option struct { bool present; T value; }
            LLVMTypeRef value_type = LLVMTypeOf(value);
            LLVMTypeRef fields[2] = {
                data->bool_type, // present flag
                value_type       // value
            };
            LLVMTypeRef option_type = LLVMStructTypeInContext(data->context, fields, 2, 0);

            // Create the struct value
            LLVMValueRef option_alloca = LLVMBuildAlloca(data->builder, option_type, "option");

            // Set present = true
            LLVMValueRef present_ptr =
                LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 0, "present_ptr");
            LLVMBuildStore(data->builder, LLVMConstInt(data->bool_type, 1, false), present_ptr);

            // Set value
            LLVMValueRef value_ptr =
                LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 1, "value_ptr");
            LLVMBuildStore(data->builder, value, value_ptr);

            // Load and return the struct
            return LLVMBuildLoad2(data->builder, option_type, option_alloca, "option_value");
        }
    } else if (strcmp(func_name, "None") == 0) {
        // Option.None - this is not a function call, just a constant
        // But since we're in generate_call_expr, this means it's used as
        // Option.None() We should return an Option struct with present = false

        // Get the expected type from node->type_info if available
        // For now, create a generic Option<i32>
        LLVMTypeRef fields[2] = {
            data->bool_type, // present flag
            data->i32_type   // value (placeholder)
        };
        LLVMTypeRef option_type = LLVMStructTypeInContext(data->context, fields, 2, 0);

        // Create the struct value
        LLVMValueRef option_alloca = LLVMBuildAlloca(data->builder, option_type, "option_none");

        // Set present = false
        LLVMValueRef present_ptr =
            LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 0, "present_ptr");
        LLVMBuildStore(data->builder, LLVMConstInt(data->bool_type, 0, false), present_ptr);

        // Value doesn't matter for None, but initialize it
        LLVMValueRef value_ptr =
            LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 1, "value_ptr");
        LLVMBuildStore(data->builder, LLVMConstInt(data->i32_type, 0, false), value_ptr);

        // Load and return the struct
        return LLVMBuildLoad2(data->builder, option_type, option_alloca, "option_none_value");
    }

    return NULL;
}

// Generate code for Result.Ok and Result.Err function calls
LLVMValueRef generate_result_function_call(LLVMBackendData *data, const ASTNode *node,
                                           const char *func_name) {
    if (strcmp(func_name, "Ok") == 0) {
        // Result.Ok(value) - create a Result struct with discriminant = 0
        if (node->data.call_expr.args && node->data.call_expr.args->count == 1) {
            // Generate the value
            LLVMValueRef ok_value = generate_expression(data, node->data.call_expr.args->nodes[0]);
            if (!ok_value)
                return NULL;

            // Create Result struct { i8 discriminant; T ok; E err; }
            LLVMTypeRef ok_type = LLVMTypeOf(ok_value);
            // For now, use i32 as default error type - this should come from type info
            LLVMTypeRef err_type = data->i32_type;
            
            LLVMTypeRef fields[3] = {
                LLVMInt8TypeInContext(data->context), // discriminant
                ok_type,                               // ok value
                err_type                               // err value
            };
            LLVMTypeRef result_type = LLVMStructTypeInContext(data->context, fields, 3, 0);

            // Create the struct value
            LLVMValueRef result_alloca = LLVMBuildAlloca(data->builder, result_type, "result");

            // Set discriminant = 0 (Ok)
            LLVMValueRef disc_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 0, "disc_ptr");
            LLVMBuildStore(data->builder, LLVMConstInt(LLVMInt8TypeInContext(data->context), 0, false), disc_ptr);

            // Set ok value
            LLVMValueRef ok_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 1, "ok_ptr");
            LLVMBuildStore(data->builder, ok_value, ok_ptr);

            // Initialize err value (not used but should be initialized)
            LLVMValueRef err_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 2, "err_ptr");
            LLVMBuildStore(data->builder, LLVMConstInt(err_type, 0, false), err_ptr);

            // Load and return the struct
            return LLVMBuildLoad2(data->builder, result_type, result_alloca, "result_value");
        }
    } else if (strcmp(func_name, "Err") == 0) {
        // Result.Err(error) - create a Result struct with discriminant = 1
        if (node->data.call_expr.args && node->data.call_expr.args->count == 1) {
            // Generate the error value
            LLVMValueRef err_value = generate_expression(data, node->data.call_expr.args->nodes[0]);
            if (!err_value)
                return NULL;

            // Create Result struct { i8 discriminant; T ok; E err; }
            // For now, use i32 as default ok type - this should come from type info
            LLVMTypeRef ok_type = data->i32_type;
            LLVMTypeRef err_type = LLVMTypeOf(err_value);
            
            LLVMTypeRef fields[3] = {
                LLVMInt8TypeInContext(data->context), // discriminant
                ok_type,                               // ok value
                err_type                               // err value
            };
            LLVMTypeRef result_type = LLVMStructTypeInContext(data->context, fields, 3, 0);

            // Create the struct value
            LLVMValueRef result_alloca = LLVMBuildAlloca(data->builder, result_type, "result");

            // Set discriminant = 1 (Err)
            LLVMValueRef disc_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 0, "disc_ptr");
            LLVMBuildStore(data->builder, LLVMConstInt(LLVMInt8TypeInContext(data->context), 1, false), disc_ptr);

            // Initialize ok value (not used but should be initialized)
            LLVMValueRef ok_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 1, "ok_ptr");
            LLVMBuildStore(data->builder, LLVMConstInt(ok_type, 0, false), ok_ptr);

            // Set err value
            LLVMValueRef err_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 2, "err_ptr");
            LLVMBuildStore(data->builder, err_value, err_ptr);

            // Load and return the struct
            return LLVMBuildLoad2(data->builder, result_type, result_alloca, "result_value");
        }
    }

    return NULL;
}