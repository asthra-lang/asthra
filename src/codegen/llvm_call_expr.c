/**
 * Asthra Programming Language LLVM Function Calls
 * Implementation of function call expression code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_call_expr.h"
#include "llvm_access_expr.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_locals.h"
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

            // For method calls, we need to handle the object specially
            ASTNode *object_node = field_access->data.field_access.object;
            LLVMValueRef self_obj = NULL;
            LLVMValueRef self_ptr = NULL;

            // Check if the object is an identifier (local variable)
            if (object_node->type == AST_IDENTIFIER) {
                const char *var_name = object_node->data.identifier.name;
                // Look up the local variable to get its alloca
                LocalVar *var_entry = lookup_local_var_entry(data, var_name);
                if (var_entry) {
                    // Use the alloca directly as the pointer
                    self_ptr = var_entry->alloca;
                    self_obj =
                        LLVMBuildLoad2(data->builder, var_entry->type, var_entry->alloca, var_name);
                }
            }
            // Check if the object is itself a field access (for chained access like
            // o.inner.method())
            else if (object_node->type == AST_FIELD_ACCESS) {
                // For field access, we need to get the pointer to the field
                self_ptr = generate_field_access_ptr(data, object_node);
                if (self_ptr) {
                    // We have the pointer, now load the value too
                    self_obj = generate_field_access(data, object_node);
                }
            }

            // If we don't have a pointer yet, generate the expression normally
            if (!self_ptr) {
                self_obj = generate_expression(data, field_access->data.field_access.object);
                if (!self_obj) {
                    return NULL;
                }
                // For now, we'll need to store it to get a pointer
                // This is a fallback case that shouldn't normally happen
                LLVMTypeRef obj_type = LLVMTypeOf(self_obj);
                self_ptr = LLVMBuildAlloca(data->builder, obj_type, "self_tmp");
                LLVMBuildStore(data->builder, self_obj, self_ptr);
            }

            if (self_ptr) {
                // This is likely an instance method call
                const char *method_name = field_access->data.field_access.field_name;

                // Look up the method - for now, we'll use a simple naming convention
                char mangled_name[256];

                // Get the type name from the object's type info
                const char *type_name = "Unknown"; // Default
                if (field_access->data.field_access.object->type_info &&
                    field_access->data.field_access.object->type_info->name) {
                    type_name = field_access->data.field_access.object->type_info->name;
                } else if (object_node->type_info && object_node->type_info->name) {
                    type_name = object_node->type_info->name;
                }

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

                // First argument is self pointer
                args[0] = self_ptr;

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

                // Check if this is a Never type method call
                bool is_never_method = false;
                if (field_access && field_access->type_info) {
                    TypeInfo *method_type_info = field_access->type_info;
                    if (method_type_info->category == TYPE_INFO_FUNCTION && 
                        method_type_info->data.function.return_type &&
                        method_type_info->data.function.return_type->category == TYPE_INFO_PRIMITIVE &&
                        method_type_info->data.function.return_type->data.primitive.kind == PRIMITIVE_INFO_NEVER) {
                        is_never_method = true;
                    }
                }

                // For Never type method calls, add unreachable instruction
                if (is_never_method) {
                    LLVMBuildUnreachable(data->builder);
                    free(args);
                    return LLVMGetUndef(data->void_type);
                }

                // For void method calls, we still need to return a value for LLVM consistency
                // The calling code will handle void returns appropriately

                free(args);
                return result;
            }
        }
    }

    // Check if this is a predeclared function call like len() or log()
    if (node->data.call_expr.function->type == AST_IDENTIFIER) {
        const char *func_name = node->data.call_expr.function->data.identifier.name;

        // Handle len() predeclared function
        if (strcmp(func_name, "len") == 0) {
            return generate_len_function_call(data, node);
        }
        
        // Handle log() predeclared function
        if (strcmp(func_name, "log") == 0) {
            return generate_log_function_call(data, node);
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

    // Check if this is a Never type function call
    bool is_never_call = false;
    if (node->data.call_expr.function && node->data.call_expr.function->type_info) {
        TypeInfo *func_type_info = node->data.call_expr.function->type_info;
        if (func_type_info->category == TYPE_INFO_FUNCTION && 
            func_type_info->data.function.return_type &&
            func_type_info->data.function.return_type->category == TYPE_INFO_PRIMITIVE &&
            func_type_info->data.function.return_type->data.primitive.kind == PRIMITIVE_INFO_NEVER) {
            is_never_call = true;
        }
    }

    // For Never type function calls, add unreachable instruction
    if (is_never_call) {
        LLVMBuildUnreachable(data->builder);
        // Return a placeholder value that will never be used
        if (args)
            free(args);
        return LLVMGetUndef(data->void_type);
    }

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
                ok_type,                              // ok value
                err_type                              // err value
            };
            LLVMTypeRef result_type = LLVMStructTypeInContext(data->context, fields, 3, 0);

            // Create the struct value
            LLVMValueRef result_alloca = LLVMBuildAlloca(data->builder, result_type, "result");

            // Set discriminant = 0 (Ok)
            LLVMValueRef disc_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 0, "disc_ptr");
            LLVMBuildStore(data->builder,
                           LLVMConstInt(LLVMInt8TypeInContext(data->context), 0, false), disc_ptr);

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
                ok_type,                              // ok value
                err_type                              // err value
            };
            LLVMTypeRef result_type = LLVMStructTypeInContext(data->context, fields, 3, 0);

            // Create the struct value
            LLVMValueRef result_alloca = LLVMBuildAlloca(data->builder, result_type, "result");

            // Set discriminant = 1 (Err)
            LLVMValueRef disc_ptr =
                LLVMBuildStructGEP2(data->builder, result_type, result_alloca, 0, "disc_ptr");
            LLVMBuildStore(data->builder,
                           LLVMConstInt(LLVMInt8TypeInContext(data->context), 1, false), disc_ptr);

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

// Generate code for len() function calls
LLVMValueRef generate_len_function_call(LLVMBackendData *data, const ASTNode *node) {
    // len() should have exactly one argument (slice or array)
    if (!node->data.call_expr.args || node->data.call_expr.args->count != 1) {
        LLVM_REPORT_ERROR(data, node, "len() requires exactly one argument");
    }

    ASTNode *arg_node = node->data.call_expr.args->nodes[0];
    LLVMValueRef arg = generate_expression(data, arg_node);
    if (!arg) {
        LLVM_REPORT_ERROR(data, arg_node, "Failed to generate argument for len()");
    }

    // Check the argument type to determine how to extract length
    if (!arg_node->type_info) {
        LLVM_REPORT_ERROR(data, arg_node, "Argument to len() missing type info");
    }

    if (arg_node->type_info->category == TYPE_INFO_SLICE) {
        // Check if this is actually a fixed-size array
        if (arg_node->type_info->type_descriptor &&
            arg_node->type_info->type_descriptor->category == TYPE_ARRAY) {
            // Fixed-size array - return the compile-time constant size
            size_t array_size = arg_node->type_info->type_descriptor->data.array.size;
            return LLVMConstInt(data->i64_type, array_size, false); // usize is i64
        } else {
            // True slice - extract length from slice struct
            // Slice is a struct { ptr, length }
            LLVMValueRef length = LLVMBuildExtractValue(data->builder, arg, 1, "slice_len");
            return length;
        }
    } else {
        LLVM_REPORT_ERROR(data, arg_node, "len() can only be called on slices and arrays");
    }
}

// Generate code for log() function calls
LLVMValueRef generate_log_function_call(LLVMBackendData *data, const ASTNode *node) {
    // log() should have exactly one argument (string message)
    if (!node->data.call_expr.args || node->data.call_expr.args->count != 1) {
        LLVM_REPORT_ERROR(data, node, "log() requires exactly one argument");
    }

    ASTNode *arg_node = node->data.call_expr.args->nodes[0];
    LLVMValueRef arg = generate_expression(data, arg_node);
    if (!arg) {
        LLVM_REPORT_ERROR(data, arg_node, "Failed to generate argument for log()");
    }

    // Get the log function from the module
    LLVMValueRef log_fn = LLVMGetNamedFunction(data->module, "log");
    if (!log_fn) {
        LLVM_REPORT_ERROR(data, node, "log() function not found in module");
    }

    // Create arguments array
    LLVMValueRef args[] = {arg};
    
    // Get function type
    LLVMTypeRef fn_type = LLVMGlobalGetValueType(log_fn);
    if (!fn_type) {
        LLVM_REPORT_ERROR(data, node, "Failed to get log() function type");
    }

    // Call the log function
    LLVMValueRef result = LLVMBuildCall2(data->builder, fn_type, log_fn, args, 1, "");
    
    return result;
}