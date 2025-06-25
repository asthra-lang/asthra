/**
 * Asthra Programming Language LLVM Expression Generation
 * Implementation of expression code generation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_expr_gen.h"
#include "llvm_types.h"
#include "llvm_debug.h"
#include "llvm_locals.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Forward declaration for mutual recursion
LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node);

// Generate code for binary operations
LLVMValueRef generate_binary_op(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef left = generate_expression(data, node->data.binary_expr.left);
    LLVMValueRef right = generate_expression(data, node->data.binary_expr.right);
    
    if (!left || !right) return NULL;
    
    switch (node->data.binary_expr.operator) {
        case BINOP_ADD:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFAdd(data->builder, left, right, "add");
            } else {
                return LLVMBuildAdd(data->builder, left, right, "add");
            }
            
        case BINOP_SUB:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFSub(data->builder, left, right, "sub");
            } else {
                return LLVMBuildSub(data->builder, left, right, "sub");
            }
            
        case BINOP_MUL:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFMul(data->builder, left, right, "mul");
            } else {
                return LLVMBuildMul(data->builder, left, right, "mul");
            }
            
        case BINOP_DIV:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFDiv(data->builder, left, right, "div");
            } else {
                // TODO: Handle signed vs unsigned division
                return LLVMBuildSDiv(data->builder, left, right, "div");
            }
            
        case BINOP_MOD:
            // TODO: Handle signed vs unsigned modulo
            return LLVMBuildSRem(data->builder, left, right, "mod");
            
        case BINOP_EQ:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOEQ, left, right, "eq");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntEQ, left, right, "eq");
            }
            
        case BINOP_NE:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealONE, left, right, "ne");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntNE, left, right, "ne");
            }
            
        case BINOP_LT:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOLT, left, right, "lt");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSLT, left, right, "lt");
            }
            
        case BINOP_GT:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOGT, left, right, "gt");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSGT, left, right, "gt");
            }
            
        case BINOP_LE:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOLE, left, right, "le");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSLE, left, right, "le");
            }
            
        case BINOP_GE:
            if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
                return LLVMBuildFCmp(data->builder, LLVMRealOGE, left, right, "ge");
            } else {
                // TODO: Handle signed vs unsigned comparison
                return LLVMBuildICmp(data->builder, LLVMIntSGE, left, right, "ge");
            }
            
        case BINOP_AND:
            return LLVMBuildAnd(data->builder, left, right, "and");
            
        case BINOP_OR:
            return LLVMBuildOr(data->builder, left, right, "or");
            
        default:
            return NULL;
    }
}

// Generate code for identifiers (variable/function references)
LLVMValueRef generate_identifier(LLVMBackendData *data, const ASTNode *node) {
    const char *name = node->data.identifier.name;
    
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
    return NULL;
}

// Generate code for unary operations
LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef operand = generate_expression(data, node->data.unary_expr.operand);
    if (!operand) return NULL;
    
    switch (node->data.unary_expr.operator) {
        case UNOP_MINUS:
            if (LLVMGetTypeKind(LLVMTypeOf(operand)) == LLVMFloatTypeKind ||
                LLVMGetTypeKind(LLVMTypeOf(operand)) == LLVMDoubleTypeKind) {
                return LLVMBuildFNeg(data->builder, operand, "neg");
            } else {
                return LLVMBuildNeg(data->builder, operand, "neg");
            }
            
        case UNOP_NOT:
            // Logical NOT - convert to boolean first if needed
            if (LLVMGetTypeKind(LLVMTypeOf(operand)) != LLVMIntegerTypeKind ||
                LLVMGetIntTypeWidth(LLVMTypeOf(operand)) != 1) {
                // Convert to bool by comparing with zero
                operand = LLVMBuildICmp(data->builder, LLVMIntNE, operand,
                                        LLVMConstNull(LLVMTypeOf(operand)), "tobool");
            }
            return LLVMBuildNot(data->builder, operand, "not");
            
        case UNOP_BITWISE_NOT:
            return LLVMBuildNot(data->builder, operand, "bitnot");
            
        case UNOP_DEREF:
            {
                // Dereference pointer
                // Get the pointee type from the pointer
                LLVMTypeRef ptr_type = LLVMTypeOf(operand);
                LLVMTypeRef elem_type = LLVMGetElementType(ptr_type);
                return LLVMBuildLoad2(data->builder, elem_type, operand, "deref");
            }
            
        case UNOP_ADDRESS_OF:
            // Take address - operand must be an lvalue
            // TODO: Properly handle lvalue checking
            return operand; // For now, assume operand is already an address
            
        case UNOP_SIZEOF:
            // sizeof operator - return size of type
            {
                LLVMTypeRef type = LLVMTypeOf(operand);
                LLVMValueRef size = LLVMSizeOf(type);
                // Convert to i64 for consistency
                return LLVMBuildZExt(data->builder, size, data->i64_type, "sizeof");
            }
            
        default:
            return NULL;
    }
}

// Generate code for function calls
LLVMValueRef generate_call_expr(LLVMBackendData *data, const ASTNode *node) {
    // Get the function to call
    if (node->data.call_expr.function) {
        if (node->data.call_expr.function->type == AST_FIELD_ACCESS) {
            // Check if this is an instance method call (object.method())
            // We need to check this BEFORE calling generate_expression
            ASTNode *field_access = node->data.call_expr.function;
            
            // Try to generate the object
            LLVMValueRef self_obj = generate_expression(data, field_access->data.field_access.object);
            if (self_obj) {
                // This is likely an instance method call
                const char *method_name = field_access->data.field_access.field_name;
                
                // Look up the method - for now, we'll use a simple naming convention
                char mangled_name[256];
                
                // Get the type name from the object's type info
                const char *type_name = "Point"; // TODO: Get from type info
                snprintf(mangled_name, sizeof(mangled_name), "%s_instance_%s", type_name, method_name);
                
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
                
                LLVMValueRef result = LLVMBuildCall2(data->builder, fn_type, function, args, (unsigned)total_args, "method_call");
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
                const char *type_name = field_access->data.field_access.object->data.identifier.name;
                const char *func_name = field_access->data.field_access.field_name;
                
                // Handle Option.Some and Option.None
                if (strcmp(type_name, "Option") == 0) {
                    if (strcmp(func_name, "Some") == 0) {
                        // Option.Some(value) - create an Option struct
                        if (node->data.call_expr.args && node->data.call_expr.args->count == 1) {
                            // Generate the value
                            LLVMValueRef value = generate_expression(data, node->data.call_expr.args->nodes[0]);
                            if (!value) return NULL;
                            
                            // Create Option struct { bool present; T value; }
                            LLVMTypeRef value_type = LLVMTypeOf(value);
                            LLVMTypeRef fields[2] = {
                                data->bool_type,    // present flag
                                value_type          // value
                            };
                            LLVMTypeRef option_type = LLVMStructTypeInContext(data->context, fields, 2, 0);
                            
                            // Create the struct value
                            LLVMValueRef option_alloca = LLVMBuildAlloca(data->builder, option_type, "option");
                            
                            // Set present = true
                            LLVMValueRef present_ptr = LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 0, "present_ptr");
                            LLVMBuildStore(data->builder, LLVMConstInt(data->bool_type, 1, false), present_ptr);
                            
                            // Set value
                            LLVMValueRef value_ptr = LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 1, "value_ptr");
                            LLVMBuildStore(data->builder, value, value_ptr);
                            
                            // Load and return the struct
                            return LLVMBuildLoad2(data->builder, option_type, option_alloca, "option_value");
                        }
                    } else if (strcmp(func_name, "None") == 0) {
                        // Option.None - this is not a function call, just a constant
                        // But since we're in generate_call_expr, this means it's used as Option.None()
                        // We should return an Option struct with present = false
                        
                        // Get the expected type from node->type_info if available
                        // For now, create a generic Option<i32>
                        LLVMTypeRef fields[2] = {
                            data->bool_type,    // present flag
                            data->i32_type      // value (placeholder)
                        };
                        LLVMTypeRef option_type = LLVMStructTypeInContext(data->context, fields, 2, 0);
                        
                        // Create the struct value
                        LLVMValueRef option_alloca = LLVMBuildAlloca(data->builder, option_type, "option_none");
                        
                        // Set present = false
                        LLVMValueRef present_ptr = LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 0, "present_ptr");
                        LLVMBuildStore(data->builder, LLVMConstInt(data->bool_type, 0, false), present_ptr);
                        
                        // Value doesn't matter for None, but initialize it
                        LLVMValueRef value_ptr = LLVMBuildStructGEP2(data->builder, option_type, option_alloca, 1, "value_ptr");
                        LLVMBuildStore(data->builder, LLVMConstInt(data->i32_type, 0, false), value_ptr);
                        
                        // Load and return the struct
                        return LLVMBuildLoad2(data->builder, option_type, option_alloca, "option_none_value");
                    }
                }
            }
        }
        
        return NULL;
    }
    
    // Check if this is a special builtin function call that needs parameter transformation
    const char *function_name = NULL;
    if (node->data.call_expr.function->type == AST_IDENTIFIER) {
        function_name = node->data.call_expr.function->data.identifier.name;
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
        if (args) free(args);
        return NULL;
    }
    
    LLVMValueRef result = LLVMBuildCall2(data->builder, fn_type, function, args, (unsigned)actual_arg_count, "call");
    
    if (args) free(args);
    return result;
}

// Generate code for array/slice indexing
LLVMValueRef generate_index_expr(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef array = generate_expression(data, node->data.index_access.array);
    LLVMValueRef index = generate_expression(data, node->data.index_access.index);
    
    if (!array || !index) return NULL;
    
    // Get element pointer
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i64_type, 0, false), // First index for array decay
        index                                     // Actual index
    };
    
    // Get array type for GEP
    LLVMTypeRef array_type = LLVMTypeOf(array);
    LLVMValueRef element_ptr = LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "elemptr");
    
    // Load the value - need to get element type from array
    // For LLVM 15+, we can't use LLVMGetElementType on opaque pointers
    // We need to get type information from the AST node
    LLVMTypeRef elem_type = data->i32_type; // Default fallback
    
    // Try to get type from node's type info
    if (node->type_info && node->type_info->category == TYPE_INFO_SLICE) {
        elem_type = asthra_type_to_llvm(data, node->type_info->data.slice.element_type);
    }
    
    return LLVMBuildLoad2(data->builder, elem_type, element_ptr, "elem");
}

// Generate code for field access
LLVMValueRef generate_field_access(LLVMBackendData *data, const ASTNode *node) {
    // Special handling for type names like Option.None
    if (node->data.field_access.object->type == AST_IDENTIFIER) {
        const char *type_name = node->data.field_access.object->data.identifier.name;
        const char *field_name = node->data.field_access.field_name;
        
        // Handle Option.None (when not used as a function call)
        if (strcmp(type_name, "Option") == 0 && strcmp(field_name, "None") == 0) {
            // Create an Option struct with present = false
            // Get the expected type from node->type_info if available
            LLVMTypeRef value_type = data->i32_type; // Default
            
            if (node->type_info && node->type_info->category == TYPE_INFO_OPTION) {
                value_type = asthra_type_to_llvm(data, node->type_info->data.option.value_type);
            }
            
            LLVMTypeRef fields[2] = {
                data->bool_type,    // present flag
                value_type          // value
            };
            LLVMTypeRef option_type = LLVMStructTypeInContext(data->context, fields, 2, 0);
            
            // Create a constant None value
            LLVMValueRef values[2] = {
                LLVMConstInt(data->bool_type, 0, false),  // present = false
                LLVMConstNull(value_type)                  // null value
            };
            
            return LLVMConstNamedStruct(option_type, values, 2);
        }
    }
    
    LLVMValueRef object = generate_expression(data, node->data.field_access.object);
    if (!object) return NULL;
    
    const char *field_name = node->data.field_access.field_name;
    
    // TODO: Look up field index from struct type info
    // For now, we'll need to integrate with the type system
    
    // Placeholder: assume field index 0
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i32_type, 0, false),
        LLVMConstInt(data->i32_type, 0, false) // TODO: Get actual field index
    };
    
    LLVMTypeRef object_type = LLVMTypeOf(object);
    LLVMValueRef field_ptr = LLVMBuildGEP2(data->builder, object_type, object, indices, 2, field_name);
    // TODO: Get actual field type from struct info
    LLVMTypeRef field_type = data->i32_type; // Placeholder
    return LLVMBuildLoad2(data->builder, field_type, field_ptr, field_name);
}

// Generate code for array literals
LLVMValueRef generate_array_literal(LLVMBackendData *data, const ASTNode *node) {
    size_t elem_count = node->data.array_literal.elements ? 
                        node->data.array_literal.elements->count : 0;
    
    if (elem_count == 0) {
        // Empty array
        return LLVMConstArray(data->i32_type, NULL, 0);
    }
    
    // Generate element values
    LLVMValueRef *elements = malloc(elem_count * sizeof(LLVMValueRef));
    LLVMTypeRef elem_type = NULL;
    
    for (size_t i = 0; i < elem_count; i++) {
        elements[i] = generate_expression(data, node->data.array_literal.elements->nodes[i]);
        if (!elements[i]) {
            free(elements);
            return NULL;
        }
        
        // Get element type from first element
        if (i == 0) {
            elem_type = LLVMTypeOf(elements[i]);
        }
    }
    
    // Create array constant
    LLVMValueRef array = LLVMConstArray(elem_type, elements, (unsigned)elem_count);
    free(elements);
    
    return array;
}

// Generate code for expressions
LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node) {
    if (!node) return NULL;
    
    // Set debug location for this expression
    set_debug_location(data, node);
    
    switch (node->type) {
        // Literals
        case AST_INTEGER_LITERAL:
            {
                // Use type information from semantic analysis if available
                LLVMTypeRef int_type = data->i32_type; // default
                if (node->type_info && node->type_info->category == TYPE_INFO_PRIMITIVE) {
                    switch (node->type_info->data.primitive.kind) {
                        case PRIMITIVE_INFO_I8:
                            int_type = LLVMInt8Type();
                            break;
                        case PRIMITIVE_INFO_I16:
                            int_type = LLVMInt16Type();
                            break;
                        case PRIMITIVE_INFO_I32:
                            int_type = data->i32_type;
                            break;
                        case PRIMITIVE_INFO_I64:
                            int_type = data->i64_type;
                            break;
                        default:
                            int_type = data->i32_type;
                            break;
                    }
                }
                return LLVMConstInt(int_type, node->data.integer_literal.value, false);
            }
            
        case AST_FLOAT_LITERAL:
            return LLVMConstReal(data->f64_type, node->data.float_literal.value);
            
        case AST_STRING_LITERAL:
            {
                // Create global string constant
                LLVMValueRef str = LLVMBuildGlobalStringPtr(data->builder, 
                    node->data.string_literal.value, ".str");
                return str;
            }
            
        case AST_BOOL_LITERAL:
        case AST_BOOLEAN_LITERAL:
            return LLVMConstInt(data->bool_type, node->data.bool_literal.value ? 1 : 0, false);
            
        case AST_CHAR_LITERAL:
            return LLVMConstInt(LLVMInt32TypeInContext(data->context), 
                                node->data.char_literal.value, false);
            
        case AST_UNIT_LITERAL:
            // Unit value is an empty struct
            return LLVMConstNamedStruct(data->unit_type, NULL, 0);
            
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
            
        // TODO: Implement remaining expression types
        // case AST_CAST_EXPR:
        case AST_STRUCT_LITERAL:
        case AST_TUPLE_LITERAL:
        case AST_SLICE_EXPR:
        case AST_ASSOCIATED_FUNC_CALL:
            // Not yet implemented
            return NULL;
            
        default:
            return NULL;
    }
}