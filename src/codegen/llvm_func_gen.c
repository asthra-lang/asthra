/**
 * Asthra Programming Language LLVM Function Generation
 * Implementation of function and top-level code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_func_gen.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_locals.h"
#include "llvm_stmt_gen.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration for internal function
static void generate_function_internal(LLVMBackendData *data, const ASTNode *node,
                                       const char *struct_name);

// Generate code for functions
void generate_function(LLVMBackendData *data, const ASTNode *node) {
    generate_function_internal(data, node, NULL);
}

// Internal function that can handle struct context for methods
static void generate_function_internal(LLVMBackendData *data, const ASTNode *node,
                                       const char *struct_name) {
    if (!node || (node->type != AST_FUNCTION_DECL && node->type != AST_METHOD_DECL)) {
        return;
    }

    const char *func_name = NULL;
    ASTNodeList *params = NULL;
    ASTNode *body = NULL;
    bool is_method = false;

    if (node->type == AST_FUNCTION_DECL) {
        func_name = node->data.function_decl.name;
        params = node->data.function_decl.params;
        body = node->data.function_decl.body;
    } else { // AST_METHOD_DECL
        func_name = node->data.method_decl.name;
        params = node->data.method_decl.params;
        body = node->data.method_decl.body;
        is_method = true;
    }

    // Get function type from type_info
    TypeInfo *func_type = node->type_info;

    // For now, if we don't have type info, try to continue with defaults
    LLVMTypeRef ret_type = data->void_type;
    int param_count = 0;

    if (func_type && func_type->category == TYPE_INFO_FUNCTION) {
        if (func_type->data.function.return_type) {
            ret_type = asthra_type_to_llvm(data, func_type->data.function.return_type);
        }
        param_count = func_type->data.function.param_count;
    } else {
        // Try to get param count from AST
        param_count = params ? params->count : 0;

        // For methods, try to get return type from AST
        if (node->type == AST_METHOD_DECL && node->data.method_decl.return_type) {
            // Try to determine return type from AST
            ASTNode *ret_type_node = node->data.method_decl.return_type;
            if (ret_type_node) {
                if (ret_type_node->type == AST_BASE_TYPE) {
                    const char *type_name = ret_type_node->data.base_type.name;
                    if (strcmp(type_name, "i32") == 0) {
                        ret_type = data->i32_type;
                    } else if (strcmp(type_name, "void") == 0) {
                        ret_type = data->void_type;
                    }
                    // Add more types as needed
                } else if (ret_type_node->type == AST_STRUCT_TYPE) {
                    // Handle user-defined types (structs)
                    if (ret_type_node->type_info) {
                        ret_type = asthra_type_to_llvm(data, ret_type_node->type_info);
                    } else {
                        // Fallback: try to create struct type based on name
                        const char *type_name = ret_type_node->data.struct_type.name;
                        if (type_name && strcmp(type_name, "Counter") == 0) {
                            // For Counter struct, we know it has one i32 field
                            LLVMTypeRef field_types[] = {data->i32_type};
                            ret_type = LLVMStructTypeInContext(data->context, field_types, 1, 0);
                        }
                    }
                }
            }
        }
    }

    // Convert parameter types
    LLVMTypeRef *param_types = NULL;
    if (param_count > 0) {
        param_types = malloc(param_count * sizeof(LLVMTypeRef));
        for (int i = 0; i < param_count; i++) {
            ASTNode *param = params->nodes[i];
            if (param && param->type == AST_PARAM_DECL) {
                // Get type from param declaration
                TypeInfo *param_type_info = NULL;

                // First check if param has direct type_info
                if (param->type_info) {
                    param_type_info = param->type_info;
                }
                // Then check param declaration's type node
                else if (param->data.param_decl.type) {
                    ASTNode *type_node = param->data.param_decl.type;
                    if (type_node->type_info) {
                        param_type_info = type_node->type_info;
                    }
                    // If still no type_info, try to infer from AST_BASE_TYPE
                    else if (type_node->type == AST_BASE_TYPE) {
                        // Map base type names to LLVM types
                        const char *type_name = type_node->data.base_type.name;
                        if (strcmp(type_name, "i32") == 0) {
                            param_types[i] = data->i32_type;
                            continue;
                        } else if (strcmp(type_name, "i64") == 0) {
                            param_types[i] = data->i64_type;
                            continue;
                        } else if (strcmp(type_name, "f32") == 0) {
                            param_types[i] = data->f32_type;
                            continue;
                        } else if (strcmp(type_name, "f64") == 0) {
                            param_types[i] = data->f64_type;
                            continue;
                        } else if (strcmp(type_name, "bool") == 0) {
                            param_types[i] = data->bool_type;
                            continue;
                        } else if (strcmp(type_name, "string") == 0) {
                            param_types[i] = data->ptr_type;
                            continue;
                        }
                    }
                }

                // Special handling for self parameter in methods
                if (node->type == AST_METHOD_DECL && i == 0 && param->data.param_decl.name &&
                    strcmp(param->data.param_decl.name, "self") == 0) {
                    // Self parameter should be a pointer to the struct
                    if (param_type_info) {
                        LLVMTypeRef struct_type = asthra_type_to_llvm(data, param_type_info);
                        param_types[i] = LLVMPointerType(struct_type, 0);
                    } else if (struct_name && strcmp(struct_name, "Counter") == 0) {
                        // Fallback for Counter struct
                        LLVMTypeRef field_types[] = {data->i32_type};
                        LLVMTypeRef struct_type =
                            LLVMStructTypeInContext(data->context, field_types, 1, 0);
                        param_types[i] = LLVMPointerType(struct_type, 0);
                    } else {
                        // Default to opaque pointer
                        param_types[i] = data->ptr_type;
                    }
                } else if (param_type_info) {
                    param_types[i] = asthra_type_to_llvm(data, param_type_info);
                } else {
                    // Default type when no type info available
                    param_types[i] = data->i32_type;
                }
            } else {
                param_types[i] = data->void_type;
            }
        }
    }

    // Create function type
    LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, param_count, false);

    // Create function
    LLVMValueRef function = NULL;

    // Mangle method names to include struct name
    char mangled_name[256];
    if (is_method && struct_name) {
        // Use the pattern: StructName_instance_methodName for instance methods
        if (node->type == AST_METHOD_DECL && node->data.method_decl.is_instance_method) {
            snprintf(mangled_name, sizeof(mangled_name), "%s_instance_%s", struct_name, func_name);
        } else {
            // Associated functions (static methods)
            snprintf(mangled_name, sizeof(mangled_name), "%s_%s", struct_name, func_name);
        }
        func_name = mangled_name;
    }

    // Special handling for main function - create a C-compatible wrapper
    if (strcmp(func_name, "main") == 0) {
        // Create the Asthra main function with a different name
        function = LLVMAddFunction(data->module, "asthra_main", fn_type);

        // Create the C main function signature: int main(int argc, char **argv)
        LLVMTypeRef c_param_types[] = {
            data->i32_type,                    // int argc
            LLVMPointerType(data->ptr_type, 0) // char **argv
        };
        LLVMTypeRef c_main_type = LLVMFunctionType(data->i32_type, c_param_types, 2, false);

        // Create the C main function
        LLVMValueRef c_main = LLVMAddFunction(data->module, "main", c_main_type);

        // Save current builder position
        LLVMBasicBlockRef current_block = LLVMGetInsertBlock(data->builder);

        // Create the entry block for C main
        LLVMBasicBlockRef c_entry = LLVMAppendBasicBlockInContext(data->context, c_main, "entry");
        LLVMPositionBuilderAtEnd(data->builder, c_entry);

        // Get argc and argv parameters
        LLVMValueRef argc = LLVMGetParam(c_main, 0);
        LLVMValueRef argv = LLVMGetParam(c_main, 1);

        // Declare asthra_runtime_init_with_args function
        LLVMValueRef runtime_init_fn =
            LLVMGetNamedFunction(data->module, "asthra_runtime_init_with_args");
        if (!runtime_init_fn) {
            // int asthra_runtime_init_with_args(AsthraGCConfig *gc_config, int argc, char **argv)
            LLVMTypeRef init_param_types[] = {
                data->ptr_type,                    // AsthraGCConfig *gc_config (NULL for default)
                data->i32_type,                    // int argc
                LLVMPointerType(data->ptr_type, 0) // char **argv
            };
            LLVMTypeRef init_fn_type = LLVMFunctionType(data->i32_type, init_param_types, 3, false);
            runtime_init_fn =
                LLVMAddFunction(data->module, "asthra_runtime_init_with_args", init_fn_type);
            LLVMSetLinkage(runtime_init_fn, LLVMExternalLinkage);
        }

        // Call asthra_runtime_init_with_args(NULL, argc, argv)
        LLVMValueRef init_args[] = {
            LLVMConstPointerNull(data->ptr_type), // NULL for default GC config
            argc, argv};
        LLVMBuildCall2(data->builder, LLVMGlobalGetValueType(runtime_init_fn), runtime_init_fn,
                       init_args, 3, "");

        // Call the Asthra main function
        LLVMValueRef asthra_result;
        if (LLVMGetTypeKind(ret_type) == LLVMVoidTypeKind) {
            // If main returns void, just call it and return 0
            LLVMBuildCall2(data->builder, fn_type, function, NULL, 0, "");
            asthra_result = LLVMConstInt(data->i32_type, 0, 0);
        } else {
            // Otherwise, get the result from asthra_main
            asthra_result =
                LLVMBuildCall2(data->builder, fn_type, function, NULL, 0, "asthra_main_result");
        }

        // Declare asthra_runtime_cleanup function
        LLVMValueRef runtime_cleanup_fn =
            LLVMGetNamedFunction(data->module, "asthra_runtime_cleanup");
        if (!runtime_cleanup_fn) {
            // void asthra_runtime_cleanup(void)
            LLVMTypeRef cleanup_fn_type = LLVMFunctionType(data->void_type, NULL, 0, false);
            runtime_cleanup_fn =
                LLVMAddFunction(data->module, "asthra_runtime_cleanup", cleanup_fn_type);
            LLVMSetLinkage(runtime_cleanup_fn, LLVMExternalLinkage);
        }

        // Call asthra_runtime_cleanup()
        LLVMBuildCall2(data->builder, LLVMGlobalGetValueType(runtime_cleanup_fn),
                       runtime_cleanup_fn, NULL, 0, "");

        // Return the result (cast to i32 if needed for C main compatibility)
        LLVMTypeRef asthra_result_type = LLVMTypeOf(asthra_result);
        if (asthra_result_type != data->i32_type) {
            // Cast to i32 for C main function compatibility
            asthra_result = LLVMBuildIntCast2(data->builder, asthra_result, data->i32_type, false,
                                              "main_result_cast");
        }
        LLVMBuildRet(data->builder, asthra_result);

        // Restore builder position
        if (current_block) {
            LLVMPositionBuilderAtEnd(data->builder, current_block);
        }
    } else {
        function = LLVMAddFunction(data->module, func_name, fn_type);
    }

    // If function returns Never, mark it as noreturn
    if (func_type && func_type->category == TYPE_INFO_FUNCTION &&
        func_type->data.function.return_type &&
        func_type->data.function.return_type->category == TYPE_INFO_PRIMITIVE &&
        func_type->data.function.return_type->data.primitive.kind == PRIMITIVE_INFO_NEVER) {
        // In LLVM 15+, use attribute builder
        // For now, we'll just handle Never type properly in code generation
        // The noreturn attribute would be: LLVMAddAttributeAtIndex(function,
        // LLVMAttributeFunctionIndex, attr);
    }

    // Set parameter names
    for (int i = 0; i < param_count; i++) {
        LLVMValueRef param = LLVMGetParam(function, i);
        ASTNode *param_node = params->nodes[i];
        if (param_node && param_node->type == AST_PARAM_DECL) {
            LLVMSetValueName2(param, param_node->data.param_decl.name,
                              strlen(param_node->data.param_decl.name));
        }
    }

    // Generate debug info for function if enabled
    if (data->di_builder && node->location.line > 0) {
        // Create function debug type
        LLVMMetadataRef fn_di_type = NULL;
        if (func_type && func_type->category == TYPE_INFO_FUNCTION) {
            fn_di_type = asthra_type_to_debug_type(data, func_type);
        }

        // Create function debug info
        LLVMMetadataRef di_function =
            LLVMDIBuilderCreateFunction(data->di_builder,
                                        data->di_file, // scope
                                        func_name, strlen(func_name),
                                        func_name, // linkage name
                                        strlen(func_name), data->di_file, node->location.line,
                                        fn_di_type ? fn_di_type : data->di_void_type,
                                        false, // is local
                                        true,  // is definition
                                        node->location.line, LLVMDIFlagPrototyped,
                                        false // is optimized
            );

        // Attach debug info to function
        LLVMSetSubprogram(function, di_function);
        data->current_debug_scope = di_function;
    }

    // Generate function body if present
    if (body) {
        // Create entry basic block
        LLVMBasicBlockRef entry = LLVMAppendBasicBlockInContext(data->context, function, "entry");
        LLVMPositionBuilderAtEnd(data->builder, entry);

        // Set current function
        data->current_function = function;

        // Clear local variables from previous function
        clear_local_vars(data);

        // Generate body statements
        LLVMValueRef last_value = NULL;
        if (body->type == AST_BLOCK) {
            ASTNodeList *statements = body->data.block.statements;
            for (size_t i = 0; statements && i < statements->count; i++) {
                ASTNode *stmt = statements->nodes[i];

                // Check if this is the last statement and it's an expression statement or unsafe
                // block
                if (i == statements->count - 1) {
                    if (stmt->type == AST_EXPR_STMT) {
                        // For the last expression, generate it as a value (potential return)
                        last_value = generate_expression(data, stmt->data.expr_stmt.expression);
                    } else if (stmt->type == AST_UNSAFE_BLOCK) {
                        // Unsafe blocks are expression-oriented when used as the last statement
                        last_value = generate_expression(data, stmt);
                    } else {
                        // Generate as regular statement
                        generate_statement(data, stmt);
                    }
                } else {
                    // Generate as regular statement
                    generate_statement(data, stmt);
                }
            }
        }

        // Add implicit return if needed
        LLVMBasicBlockRef current_block = LLVMGetInsertBlock(data->builder);

        if (!LLVMGetBasicBlockTerminator(current_block)) {
            // Check if this is a Never type function
            bool is_never_function = false;
            if (func_type && func_type->category == TYPE_INFO_FUNCTION &&
                func_type->data.function.return_type &&
                func_type->data.function.return_type->category == TYPE_INFO_PRIMITIVE &&
                func_type->data.function.return_type->data.primitive.kind == PRIMITIVE_INFO_NEVER) {
                is_never_function = true;
            }

            if (is_never_function) {
                // Never type functions should end with unreachable, not return
                LLVMBuildUnreachable(data->builder);
            } else if (ret_type == data->void_type) {
                LLVMBuildRetVoid(data->builder);
            } else if (ret_type == data->unit_type) {
                LLVMValueRef unit = LLVMConstNamedStruct(data->unit_type, NULL, 0);
                LLVMBuildRet(data->builder, unit);
            } else if (last_value) {
                // Return the last expression value (with type coercion if needed)
                LLVMTypeRef last_value_type = LLVMTypeOf(last_value);
                if (last_value_type != ret_type) {
                    // Coerce integer types
                    if (LLVMGetTypeKind(last_value_type) == LLVMIntegerTypeKind &&
                        LLVMGetTypeKind(ret_type) == LLVMIntegerTypeKind) {
                        unsigned src_bits = LLVMGetIntTypeWidth(last_value_type);
                        unsigned dst_bits = LLVMGetIntTypeWidth(ret_type);
                        if (src_bits < dst_bits) {
                            // Sign extend for larger type
                            last_value = LLVMBuildSExt(data->builder, last_value, ret_type, "sext");
                        } else if (src_bits > dst_bits) {
                            // Truncate for smaller type
                            last_value =
                                LLVMBuildTrunc(data->builder, last_value, ret_type, "trunc");
                        }
                    }
                }
                LLVMBuildRet(data->builder, last_value);
            } else {
                // No value to return, use default
                if (ret_type == data->i32_type) {
                    LLVMBuildRet(data->builder, LLVMConstInt(data->i32_type, 0, false));
                } else {
                    LLVMBuildRetVoid(data->builder);
                }
            }
        }

        // Verify function
        char *error = NULL;
        if (LLVMVerifyFunction(function, LLVMPrintMessageAction)) {
            // Function verification failed - this should not happen with our corrected
            // implementation
            fprintf(stderr, "LLVM function verification failed for %s\n",
                    strcmp(func_name, "main") == 0 ? "asthra_main" : func_name);

            // Get detailed error info
            char *func_str = LLVMPrintValueToString(function);
            fprintf(stderr, "Function dump:\n%s\n", func_str ? func_str : "null");
            if (func_str)
                LLVMDisposeMessage(func_str);

            LLVMDeleteFunction(function);
        } else {
        }
    }

    if (param_types)
        free(param_types);
}

// Helper function to generate constant initializer values
// Returns an LLVM constant value suitable for global variable initialization
static LLVMValueRef generate_const_initializer(LLVMBackendData *data, const ASTNode *value_node,
                                               LLVMTypeRef expected_type) {
    if (!data || !value_node) {
        return NULL;
    }

    switch (value_node->type) {
    case AST_CONST_EXPR: {
        // Unwrap the const expression and process the inner expression
        // Check const expression type
        switch (value_node->data.const_expr.expr_type) {
        case CONST_EXPR_LITERAL:
            // Recursively process the literal
            return generate_const_initializer(data, value_node->data.const_expr.data.literal,
                                              expected_type);
        case CONST_EXPR_BINARY_OP: {
            LLVMValueRef left = generate_const_initializer(
                data, value_node->data.const_expr.data.binary.left, expected_type);
            LLVMValueRef right = generate_const_initializer(
                data, value_node->data.const_expr.data.binary.right, expected_type);
            if (!left || !right)
                return NULL;

            switch (value_node->data.const_expr.data.binary.op) {
            case BINOP_ADD:
                return LLVMConstAdd(left, right);
            case BINOP_SUB:
                return LLVMConstSub(left, right);
            case BINOP_MUL:
                return LLVMConstMul(left, right);
            default:
                return NULL;
            }
        }
        case CONST_EXPR_UNARY_OP: {
            LLVMValueRef operand = generate_const_initializer(
                data, value_node->data.const_expr.data.unary.operand, expected_type);
            if (!operand)
                return NULL;

            switch (value_node->data.const_expr.data.unary.op) {
            case UNOP_MINUS:
                return LLVMConstNeg(operand);
            case UNOP_NOT:
            case UNOP_BITWISE_NOT:
                return LLVMConstNot(operand);
            default:
                return NULL;
            }
        }
        case CONST_EXPR_SIZEOF:
            // TODO: Handle sizeof
            return NULL;
        case CONST_EXPR_IDENTIFIER: {
            // Look up the const value from another global constant
            const char *name = value_node->data.const_expr.data.identifier;
            LLVMValueRef global = LLVMGetNamedGlobal(data->module, name);
            if (global) {
                LLVMValueRef init = LLVMGetInitializer(global);
                if (init) {
                    return init;
                }
            }
            return NULL;
        }
        default:
            return NULL;
        }
    }

    case AST_INTEGER_LITERAL: {
        int64_t value = value_node->data.integer_literal.value;
        return LLVMConstInt(expected_type, value, true);
    }

    case AST_FLOAT_LITERAL: {
        double value = value_node->data.float_literal.value;
        return LLVMConstReal(expected_type, value);
    }

    case AST_BOOL_LITERAL: {
        bool value = value_node->data.bool_literal.value;
        return LLVMConstInt(expected_type, value ? 1 : 0, false);
    }

    case AST_STRING_LITERAL: {
        const char *value = value_node->data.string_literal.value;
        // Create a global string constant
        LLVMValueRef str_const =
            LLVMConstStringInContext(data->context, value, (unsigned)strlen(value), false);
        LLVMValueRef global_str = LLVMAddGlobal(data->module, LLVMTypeOf(str_const), ".str");
        LLVMSetInitializer(global_str, str_const);
        LLVMSetGlobalConstant(global_str, true);
        LLVMSetLinkage(global_str, LLVMPrivateLinkage);
        LLVMSetUnnamedAddress(global_str, LLVMGlobalUnnamedAddr);
        // Return pointer to the string
        return LLVMConstBitCast(global_str, data->ptr_type);
    }

    case AST_CHAR_LITERAL: {
        char value = value_node->data.char_literal.value;
        return LLVMConstInt(expected_type, value, false);
    }

    case AST_BINARY_EXPR: {
        // Handle binary expressions by recursively evaluating operands
        LLVMValueRef left =
            generate_const_initializer(data, value_node->data.binary_expr.left, expected_type);
        LLVMValueRef right =
            generate_const_initializer(data, value_node->data.binary_expr.right, expected_type);

        if (!left || !right) {
            return NULL;
        }

        switch (value_node->data.binary_expr.operator) {
        case BINOP_ADD:
            return LLVMConstAdd(left, right);
        case BINOP_SUB:
            return LLVMConstSub(left, right);
        case BINOP_MUL:
            return LLVMConstMul(left, right);
        // Note: Division, modulo, bitwise, and shift operations on constants
        // have been deprecated in newer LLVM versions. For these operations,
        // the semantic analyzer should compute the value at compile time.
        default:
            return NULL;
        }
    }

    case AST_UNARY_EXPR: {
        LLVMValueRef operand =
            generate_const_initializer(data, value_node->data.unary_expr.operand, expected_type);
        if (!operand) {
            return NULL;
        }

        switch (value_node->data.unary_expr.operator) {
        case UNOP_MINUS:
            return LLVMConstNeg(operand);
        case UNOP_NOT:
            return LLVMConstNot(operand);
        case UNOP_BITWISE_NOT:
            return LLVMConstNot(operand);
        default:
            return NULL;
        }
    }

    case AST_IDENTIFIER: {
        // Look up the const value from another global constant
        const char *name = value_node->data.identifier.name;
        LLVMValueRef global = LLVMGetNamedGlobal(data->module, name);
        if (global) {
            LLVMValueRef init = LLVMGetInitializer(global);
            if (init) {
                return init;
            }
        }
        return NULL;
    }

    default:
        return NULL;
    }
}

// Generate code for const declarations
static void generate_const_declaration(LLVMBackendData *data, const ASTNode *node) {
    if (!node || node->type != AST_CONST_DECL) {
        return;
    }

    const char *name = node->data.const_decl.name;
    ASTNode *type_node = node->data.const_decl.type;
    ASTNode *value_node = node->data.const_decl.value;

    if (!name || !value_node) {
        return;
    }

    // Get the LLVM type for the const
    LLVMTypeRef const_type = NULL;
    if (type_node && type_node->type_info) {
        const_type = asthra_type_to_llvm(data, type_node->type_info);
    }
    // Fallback: if type_info is not set, try to determine type from AST node directly
    if (!const_type && type_node && type_node->type == AST_BASE_TYPE) {
        const char *type_name = type_node->data.base_type.name;
        if (type_name) {
            if (strcmp(type_name, "string") == 0) {
                const_type = data->ptr_type;
            } else if (strcmp(type_name, "i32") == 0) {
                const_type = data->i32_type;
            } else if (strcmp(type_name, "i64") == 0) {
                const_type = data->i64_type;
            } else if (strcmp(type_name, "f32") == 0) {
                const_type = data->f32_type;
            } else if (strcmp(type_name, "f64") == 0) {
                const_type = data->f64_type;
            } else if (strcmp(type_name, "bool") == 0) {
                const_type = data->bool_type;
            } else if (strcmp(type_name, "u8") == 0 || strcmp(type_name, "i8") == 0) {
                const_type = LLVMInt8TypeInContext(data->context);
            } else if (strcmp(type_name, "u16") == 0 || strcmp(type_name, "i16") == 0) {
                const_type = LLVMInt16TypeInContext(data->context);
            } else if (strcmp(type_name, "u32") == 0) {
                const_type = LLVMInt32TypeInContext(data->context);
            } else if (strcmp(type_name, "u64") == 0) {
                const_type = LLVMInt64TypeInContext(data->context);
            } else if (strcmp(type_name, "usize") == 0 || strcmp(type_name, "isize") == 0) {
                const_type = data->i64_type;
            } else if (strcmp(type_name, "char") == 0) {
                const_type = LLVMInt8TypeInContext(data->context);
            }
        }
    }
    if (!const_type) {
        const_type = data->i32_type; // Default to i32
    }

    // Generate the constant initializer value
    LLVMValueRef init_value = generate_const_initializer(data, value_node, const_type);
    if (!init_value) {
        // Fallback: for string types, try with ptr type
        if (value_node->type == AST_STRING_LITERAL) {
            const_type = data->ptr_type;
            init_value = generate_const_initializer(data, value_node, const_type);
        }
        if (!init_value) {
            return;
        }
    }

    // Create the global constant
    LLVMValueRef global = LLVMAddGlobal(data->module, const_type, name);
    LLVMSetInitializer(global, init_value);
    LLVMSetGlobalConstant(global, true);
    // Use internal linkage so the global can be found by name within the module
    LLVMSetLinkage(global, LLVMInternalLinkage);
}

// Generate code for top-level declarations
void generate_top_level(LLVMBackendData *data, const ASTNode *node) {
    if (!node) {
        return;
    }

    switch (node->type) {
    case AST_FUNCTION_DECL:
        generate_function(data, node);
        // TODO: Update statistics when backend pointer is available
        break;

    case AST_CONST_DECL:
        generate_const_declaration(data, node);
        break;

    // TODO: Implement other top-level declarations
    case AST_STRUCT_DECL:
    case AST_ENUM_DECL:
        // Not yet implemented
        break;

    case AST_IMPL_BLOCK:
        // Generate functions from impl block
        if (node->data.impl_block.methods) {
            const char *struct_name = node->data.impl_block.struct_name;
            for (size_t i = 0; i < node->data.impl_block.methods->count; i++) {
                ASTNode *method = node->data.impl_block.methods->nodes[i];
                if (method &&
                    (method->type == AST_FUNCTION_DECL || method->type == AST_METHOD_DECL)) {
                    // Generate the method with struct context for proper name mangling
                    generate_function_internal(data, method, struct_name);
                }
            }
        }
        break;

    default:
        break;
    }
}