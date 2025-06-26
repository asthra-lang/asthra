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

// Generate code for functions
void generate_function(LLVMBackendData *data, const ASTNode *node) {
    if (!node || (node->type != AST_FUNCTION_DECL && node->type != AST_METHOD_DECL)) {
        return;
    }

    const char *func_name = NULL;
    ASTNodeList *params = NULL;
    ASTNode *body = NULL;

    if (node->type == AST_FUNCTION_DECL) {
        func_name = node->data.function_decl.name;
        params = node->data.function_decl.params;
        body = node->data.function_decl.body;
    } else { // AST_METHOD_DECL
        func_name = node->data.method_decl.name;
        params = node->data.method_decl.params;
        body = node->data.method_decl.body;
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

                param_types[i] = asthra_type_to_llvm(data, param_type_info);
            } else {
                param_types[i] = data->void_type;
            }
        }
    }

    // Create function type
    LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, param_count, false);

    // Create function
    LLVMValueRef function = NULL;

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

        // Call the Asthra main function
        LLVMBuildCall2(data->builder, fn_type, function, NULL, 0, "");

        // Return 0
        LLVMBuildRet(data->builder, LLVMConstInt(data->i32_type, 0, false));

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

                // Check if this is the last statement and it's an expression statement
                if (i == statements->count - 1 && stmt->type == AST_EXPR_STMT) {
                    // For the last expression, generate it as a value (potential return)
                    last_value = generate_expression(data, stmt->data.expr_stmt.expression);
                } else {
                    // Generate as regular statement
                    generate_statement(data, stmt);
                }
            }
        }

        // Add implicit return if needed
        LLVMBasicBlockRef current_block = LLVMGetInsertBlock(data->builder);

        if (!LLVMGetBasicBlockTerminator(current_block)) {
            if (ret_type == data->void_type) {
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

    // TODO: Implement other top-level declarations
    case AST_STRUCT_DECL:
    case AST_ENUM_DECL:
    case AST_CONST_DECL:
        // Not yet implemented
        break;

    case AST_IMPL_BLOCK:
        // Generate functions from impl block
        if (node->data.impl_block.methods) {
            for (size_t i = 0; i < node->data.impl_block.methods->count; i++) {
                ASTNode *method = node->data.impl_block.methods->nodes[i];
                if (method &&
                    (method->type == AST_FUNCTION_DECL || method->type == AST_METHOD_DECL)) {
                    // Generate the method as a regular function
                    // The semantic analyzer should have already mangled the name
                    generate_function(data, method);
                }
            }
        }
        break;

    default:
        break;
    }
}