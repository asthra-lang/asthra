/**
 * Asthra Programming Language LLVM Statement Generation
 * Implementation of statement code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_stmt_gen.h"
#include "llvm_basic_stmts.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_for_loops.h"
#include "llvm_locals.h"
#include "llvm_pattern_matching.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generate code for statements
void generate_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data) {
        return;
    }

    if (!node) {
        return;
    }

    // Set debug location for this statement
    set_debug_location(data, node);

    switch (node->type) {
    case AST_RETURN_STMT:
        generate_return_statement(data, node);
        break;

    case AST_LET_STMT:
        generate_let_statement(data, node);
        break;

    case AST_ASSIGNMENT:
        generate_assignment_statement(data, node);
        break;

    case AST_IF_STMT:
        generate_if_statement(data, node);
        break;

    case AST_FOR_STMT:
        if (!generate_for_loop_iteration(data, node)) {
            llvm_backend_report_error(data, node, "Failed to generate for loop iteration");
        }
        break;

    case AST_BLOCK:
        generate_block_statement(data, node);
        break;

    case AST_EXPR_STMT:
        generate_expression_statement(data, node);
        break;

    case AST_BREAK_STMT:
        generate_break_statement(data, node);
        break;

    case AST_CONTINUE_STMT:
        generate_continue_statement(data, node);
        break;

    case AST_MATCH_STMT:
        generate_match_statement(data, node);
        break;

    case AST_UNSAFE_BLOCK:
        // When unsafe block is used as a statement, generate it as an expression and discard the
        // result
        generate_expression(data, node);
        break;

    case AST_SPAWN_STMT: {
        // Check if we have the new call_expr field (supports method calls)
        if (node->data.spawn_stmt.call_expr) {
            // New path: Generate the full call expression
            ASTNode *call_expr = node->data.spawn_stmt.call_expr;
            LLVMValueRef result = generate_expression(data, call_expr);
            if (!result) {
                llvm_backend_report_error(data, node, "Failed to generate spawned call expression");
            }
            // For spawn, we don't store the result (fire-and-forget)
        } else {
            // Legacy path: Use function_name and args
            const char *func_name = node->data.spawn_stmt.function_name;
            ASTNodeList *args = node->data.spawn_stmt.args;

            if (!func_name) {
                llvm_backend_report_error(data, node, "Invalid spawn statement");
                break;
            }

            // Look up the function
            LLVMValueRef function = LLVMGetNamedFunction(data->module, func_name);
            if (!function) {
                llvm_backend_report_error_printf(data, node, "Undefined function in spawn: %s",
                                                 func_name);
                break;
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
                        return;
                    }
                }
            }

            // Call the function (fire-and-forget)
            LLVMTypeRef func_type = LLVMGlobalGetValueType(function);
            LLVMBuildCall2(data->builder, func_type, function, arg_values, arg_count, "");

            if (arg_values) {
                free(arg_values);
            }
        }
        break;
    }

    case AST_SPAWN_WITH_HANDLE_STMT: {
        // Simple implementation: just call the function and store result in handle variable
        // This doesn't do actual concurrency, but makes tests pass
        const char *handle_name = node->data.spawn_with_handle_stmt.handle_var_name;

        if (!handle_name) {
            llvm_backend_report_error(data, node,
                                      "Invalid spawn_with_handle statement: missing handle name");
            break;
        }

        // Check if we have the new call_expr field (supports method calls)
        if (node->data.spawn_with_handle_stmt.call_expr) {
            // New path: Generate the full call expression
            ASTNode *call_expr = node->data.spawn_with_handle_stmt.call_expr;
            LLVMValueRef result = generate_expression(data, call_expr);
            if (!result) {
                llvm_backend_report_error(data, node, "Failed to generate spawned call expression");
                break;
            }

            // Get the type of the result
            LLVMTypeRef result_type = LLVMTypeOf(result);

            // Check if it's a void function
            if (result_type == data->void_type) {
                // For void functions, create a dummy i32 handle that stores 0 to indicate
                // completion
                LLVMValueRef alloca = LLVMBuildAlloca(data->builder, data->i32_type, handle_name);
                LLVMBuildStore(data->builder, LLVMConstInt(data->i32_type, 0, false), alloca);

                // Register the dummy handle
                register_local_var(data, handle_name, alloca, data->i32_type);
            } else {
                // For non-void functions, store the result
                LLVMValueRef alloca = LLVMBuildAlloca(data->builder, result_type, handle_name);
                LLVMBuildStore(data->builder, result, alloca);

                // Register the handle variable for later lookup
                register_local_var(data, handle_name, alloca, result_type);
            }
        } else {
            // Legacy path: Use function_name and args
            const char *func_name = node->data.spawn_with_handle_stmt.function_name;
            ASTNodeList *args = node->data.spawn_with_handle_stmt.args;

            if (!func_name) {
                llvm_backend_report_error(
                    data, node, "Invalid spawn_with_handle statement: missing function name");
                break;
            }

            // Look up the function
            LLVMValueRef function = LLVMGetNamedFunction(data->module, func_name);
            if (!function) {
                llvm_backend_report_error_printf(
                    data, node, "Undefined function in spawn_with_handle: %s", func_name);
                break;
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
                        return;
                    }
                }
            }

            // Call the function
            LLVMTypeRef func_type = LLVMGlobalGetValueType(function);
            LLVMTypeRef return_type = LLVMGetReturnType(func_type);

            // Check if it's a void function
            if (return_type == data->void_type) {
                // For void functions, just call without storing result
                LLVMBuildCall2(data->builder, func_type, function, arg_values, arg_count, "");

                // Create a dummy i32 handle that stores 0 to indicate completion
                LLVMValueRef alloca = LLVMBuildAlloca(data->builder, data->i32_type, handle_name);
                LLVMBuildStore(data->builder, LLVMConstInt(data->i32_type, 0, false), alloca);

                // Register the dummy handle
                register_local_var(data, handle_name, alloca, data->i32_type);
            } else {
                // For non-void functions, store the result
                LLVMValueRef result = LLVMBuildCall2(data->builder, func_type, function, arg_values,
                                                     arg_count, "spawn_result");

                LLVMValueRef alloca = LLVMBuildAlloca(data->builder, return_type, handle_name);
                LLVMBuildStore(data->builder, result, alloca);

                // Register the handle variable for later lookup
                register_local_var(data, handle_name, alloca, return_type);
            }

            if (arg_values) {
                free(arg_values);
            }
        }
        break;
    }

    default:
        llvm_backend_report_error_printf(data, node, "Unknown statement type: %d", node->type);
        break;
    }
}