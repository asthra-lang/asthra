/**
 * Asthra Programming Language LLVM Basic Statements
 * Implementation of basic statement code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_basic_stmts.h"
#include "../parser/ast_types.h"
#include "llvm_backend_internal.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_locals.h"
#include "llvm_pattern_matching.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration for generate_statement function
void generate_statement(LLVMBackendData *data, const ASTNode *node);

// Generate code for return statements
void generate_return_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_RETURN_STMT) {
        return;
    }

    // Get the function return type
    LLVMTypeRef fn_type = LLVMGlobalGetValueType(data->current_function);
    LLVMTypeRef fn_ret_type = LLVMGetReturnType(fn_type);

    // Get current function's return type info to check for Never type
    TypeInfo *current_func_return_type = NULL;
    if (data->current_function) {
        LLVMValueRef current_func = data->current_function;
        // Check if current function has Never return type by examining function type
        LLVMTypeRef current_func_type = LLVMGlobalGetValueType(current_func);
        // For now, we'll detect this during the expression check below
    }

    // Handle no expression case (implicit return)
    if (!node->data.return_stmt.expression) {
        if (fn_ret_type == data->void_type) {
            LLVMBuildRetVoid(data->builder);
        } else if (fn_ret_type == data->unit_type) {
            // Return unit value for unit type
            LLVMValueRef unit_val = LLVMConstNamedStruct(data->unit_type, NULL, 0);
            LLVMBuildRet(data->builder, unit_val);
        } else {
            // For other types, return a default value (e.g., zero)
            // This handles test cases or error recovery
            LLVMValueRef default_val = LLVMConstNull(fn_ret_type);
            LLVMBuildRet(data->builder, default_val);
        }
        return;
    }

    // Special handling for unit literal returns
    if (node->data.return_stmt.expression->type == AST_UNIT_LITERAL) {
        if (fn_ret_type == data->void_type) {
            // For void functions, return void
            LLVMBuildRetVoid(data->builder);
        } else if (fn_ret_type == data->unit_type) {
            // For unit-returning functions, return unit value
            LLVMValueRef unit_val = LLVMConstNamedStruct(data->unit_type, NULL, 0);
            LLVMBuildRet(data->builder, unit_val);
        } else {
            // For other types, generate the expression normally
            LLVMValueRef ret_val = generate_expression(data, node->data.return_stmt.expression);
            if (ret_val) {
                // Check if we need to cast the return value to the expected function return type
                LLVMTypeRef ret_val_type = LLVMTypeOf(ret_val);
                if (ret_val_type != fn_ret_type) {
                    // Handle integer type conversions (e.g., usize to i32)
                    if (LLVMGetTypeKind(ret_val_type) == LLVMIntegerTypeKind &&
                        LLVMGetTypeKind(fn_ret_type) == LLVMIntegerTypeKind) {
                        unsigned src_bits = LLVMGetIntTypeWidth(ret_val_type);
                        unsigned dst_bits = LLVMGetIntTypeWidth(fn_ret_type);

                        if (src_bits > dst_bits) {
                            // Truncate larger type to smaller type (e.g., i64 to i32)
                            ret_val =
                                LLVMBuildTrunc(data->builder, ret_val, fn_ret_type, "ret_trunc");
                        } else if (src_bits < dst_bits) {
                            // Extend smaller type to larger type (e.g., i32 to i64)
                            ret_val = LLVMBuildZExt(data->builder, ret_val, fn_ret_type, "ret_ext");
                        }
                        // If src_bits == dst_bits, types are the same width, no conversion needed
                    }
                }
                LLVMBuildRet(data->builder, ret_val);
            } else {
                llvm_backend_report_error(data, node, "Failed to generate return value");
            }
        }
        return;
    }

    // Check if the return expression is a Never type call
    bool is_never_return = false;
    if (node->data.return_stmt.expression->type == AST_CALL_EXPR) {
        ASTNode *call_node = node->data.return_stmt.expression;
        if (call_node->data.call_expr.function && call_node->data.call_expr.function->type_info) {
            TypeInfo *func_type_info = call_node->data.call_expr.function->type_info;
            if (func_type_info->category == TYPE_INFO_FUNCTION &&
                func_type_info->data.function.return_type &&
                func_type_info->data.function.return_type->category == TYPE_INFO_PRIMITIVE &&
                func_type_info->data.function.return_type->data.primitive.kind ==
                    PRIMITIVE_INFO_NEVER) {
                is_never_return = true;
            }
        }
        // Also check if the call expression itself has Never type info
        else if (call_node->type_info && call_node->type_info->category == TYPE_INFO_PRIMITIVE &&
                 call_node->type_info->data.primitive.kind == PRIMITIVE_INFO_NEVER) {
            is_never_return = true;
        }
    }

    // For Never type calls, just generate the expression (which will add unreachable)
    // and don't generate any return instruction
    if (is_never_return) {
        generate_expression(data, node->data.return_stmt.expression);
        // Never type calls end with unreachable, so no return instruction needed
        return;
    }

    // Check if the return expression is a field access that might be a Never type method
    bool is_never_method_return = false;
    if (node->data.return_stmt.expression->type == AST_CALL_EXPR) {
        ASTNode *call_node = node->data.return_stmt.expression;
        if (call_node->data.call_expr.function &&
            call_node->data.call_expr.function->type == AST_FIELD_ACCESS) {
            ASTNode *field_access = call_node->data.call_expr.function;
            if (field_access->type_info &&
                field_access->type_info->category == TYPE_INFO_FUNCTION &&
                field_access->type_info->data.function.return_type &&
                field_access->type_info->data.function.return_type->category ==
                    TYPE_INFO_PRIMITIVE &&
                field_access->type_info->data.function.return_type->data.primitive.kind ==
                    PRIMITIVE_INFO_NEVER) {
                is_never_method_return = true;
            }
        }
    }

    // For Never type method calls, just generate the expression (which will add unreachable)
    // and don't generate any return instruction
    if (is_never_method_return) {
        generate_expression(data, node->data.return_stmt.expression);
        // Never type method calls end with unreachable, so no return instruction needed
        return;
    }

    // Check if the return expression is a void method call
    bool is_void_method_return = false;
    if (node->data.return_stmt.expression->type == AST_CALL_EXPR) {
        ASTNode *call_node = node->data.return_stmt.expression;
        if (call_node->data.call_expr.function &&
            call_node->data.call_expr.function->type == AST_FIELD_ACCESS) {
            ASTNode *field_access = call_node->data.call_expr.function;
            if (field_access->type_info &&
                field_access->type_info->category == TYPE_INFO_FUNCTION &&
                field_access->type_info->data.function.return_type &&
                field_access->type_info->data.function.return_type->category ==
                    TYPE_INFO_PRIMITIVE &&
                field_access->type_info->data.function.return_type->data.primitive.kind ==
                    PRIMITIVE_INFO_VOID) {
                is_void_method_return = true;
            }
        }
    }

    // For void method calls, just generate the expression and emit ret void
    if (is_void_method_return) {
        generate_expression(data, node->data.return_stmt.expression);
        // For void functions, emit ret void
        LLVMBuildRetVoid(data->builder);
        return;
    }

    // For all other expressions, generate normally
    LLVMValueRef ret_val = generate_expression(data, node->data.return_stmt.expression);
    if (ret_val) {
        // Check the type of the returned value
        LLVMTypeRef ret_val_type = LLVMTypeOf(ret_val);

        // Special handling for void values - if we're returning a void expression,
        // just emit ret void instead of trying to return the void value
        if (ret_val_type == data->void_type || LLVMGetTypeKind(ret_val_type) == LLVMVoidTypeKind) {
            LLVMBuildRetVoid(data->builder);
            return;
        }

        // Check if we need to cast the return value to the expected function return type
        if (ret_val_type != fn_ret_type) {
            // Handle integer type conversions (e.g., usize to i32)
            if (LLVMGetTypeKind(ret_val_type) == LLVMIntegerTypeKind &&
                LLVMGetTypeKind(fn_ret_type) == LLVMIntegerTypeKind) {
                unsigned src_bits = LLVMGetIntTypeWidth(ret_val_type);
                unsigned dst_bits = LLVMGetIntTypeWidth(fn_ret_type);

                if (src_bits > dst_bits) {
                    // Truncate larger type to smaller type (e.g., i64 to i32)
                    ret_val = LLVMBuildTrunc(data->builder, ret_val, fn_ret_type, "ret_trunc");
                } else if (src_bits < dst_bits) {
                    // Extend smaller type to larger type (e.g., i32 to i64)
                    ret_val = LLVMBuildZExt(data->builder, ret_val, fn_ret_type, "ret_ext");
                }
                // If src_bits == dst_bits, types are the same width, no conversion needed
            }
        }
        LLVMBuildRet(data->builder, ret_val);
    } else {
        // If expression generation failed but function expects void, return void
        if (fn_ret_type == data->void_type) {
            LLVMBuildRetVoid(data->builder);
        } else {
            llvm_backend_report_error(data, node, "Failed to generate return value");
        }
    }
}

// Generate code for variable declarations (let statements)
void generate_let_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_LET_STMT) {
        return;
    }

    const char *var_name = node->data.let_stmt.name;
    LLVMTypeRef var_type = data->i32_type; // Default type

    // Get type from type_info if available
    if (node->type_info) {
        var_type = asthra_type_to_llvm(data, node->type_info);
    } else if (node->data.let_stmt.type && node->data.let_stmt.type->type_info) {
        var_type = asthra_type_to_llvm(data, node->data.let_stmt.type->type_info);
    }

    // Check if the type is void - void types don't allocate space
    if (var_type == data->void_type) {
        // For void types, we don't allocate any memory
        // Initialize if there's an initializer (evaluate for side effects)
        if (node->data.let_stmt.initializer) {
            generate_expression(data, node->data.let_stmt.initializer);
        }
        // Don't register void variables - they have no storage
        return;
    }

    // Allocate space on stack for non-void types
    LLVMValueRef alloca = LLVMBuildAlloca(data->builder, var_type, var_name);

    // Initialize if there's an initializer
    if (node->data.let_stmt.initializer) {
        LLVMValueRef init_val = generate_expression(data, node->data.let_stmt.initializer);
        if (init_val) {
            LLVMBuildStore(data->builder, init_val, alloca);
        }
    }

    // Generate debug info for local variable
    if (data->di_builder && data->current_debug_scope && node->location.line > 0) {
        LLVMMetadataRef di_var_type = asthra_type_to_debug_type(
            data, node->type_info
                      ? node->type_info
                      : (node->data.let_stmt.type ? node->data.let_stmt.type->type_info : NULL));

        if (di_var_type) {
            LLVMMetadataRef di_var = LLVMDIBuilderCreateAutoVariable(
                data->di_builder, data->current_debug_scope, var_name, strlen(var_name),
                data->di_file, node->location.line, di_var_type,
                true, // preserve through opts
                LLVMDIFlagZero,
                0 // align
            );

            // Create debug declare
            LLVMMetadataRef expr = LLVMDIBuilderCreateExpression(data->di_builder, NULL, 0);
            LLVMMetadataRef loc = LLVMDIBuilderCreateDebugLocation(
                data->context, node->location.line, node->location.column,
                data->current_debug_scope, NULL);

            // Insert declare instruction for debug info
            // TODO: Update to use LLVMDIBuilderInsertDeclareRecordAtEnd for LLVM 20+
            // LLVMValueRef dbg_declare = LLVMDIBuilderInsertDeclareAtEnd(
            //     data->di_builder,
            //     alloca,
            //     di_var,
            //     expr,
            //     loc,
            //     LLVMGetInsertBlock(data->builder)
            // );
        }
    }

    // Register the local variable for later lookup
    register_local_var(data, var_name, alloca, var_type);
}

// Generate code for assignment statements
void generate_assignment_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_ASSIGNMENT) {
        return;
    }

    // Generate lvalue address
    LLVMValueRef target = NULL;

    if (node->data.assignment.target->type == AST_IDENTIFIER) {
        // Simple variable assignment
        const char *var_name = node->data.assignment.target->data.identifier.name;
        // Look up local variable first
        target = lookup_local_var(data, var_name);
        if (!target) {
            // Try global variable
            target = LLVMGetNamedGlobal(data->module, var_name);
        }
    } else if (node->data.assignment.target->type == AST_FIELD_ACCESS) {
        // Field assignment
        target = generate_expression(data, node->data.assignment.target);
    } else if (node->data.assignment.target->type == AST_INDEX_ACCESS) {
        // Array element assignment
        LLVMValueRef array =
            generate_expression(data, node->data.assignment.target->data.index_access.array);
        LLVMValueRef index =
            generate_expression(data, node->data.assignment.target->data.index_access.index);

        if (array && index) {
            LLVMValueRef indices[2] = {LLVMConstInt(data->i64_type, 0, false), index};
            LLVMTypeRef array_type = LLVMTypeOf(array);
            target = LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "elemptr");
        }
    } else if (node->data.assignment.target->type == AST_UNARY_EXPR &&
               node->data.assignment.target->data.unary_expr.operator== UNOP_DEREF) {
        // Pointer dereference assignment (*ptr = value)
        // RADICAL DEBUGGING: Instead of using the pointer variable,
        // try to directly identify what variable the pointer points to
        ASTNode *ptr_node = node->data.assignment.target->data.unary_expr.operand;

        // Check if this is a simple case like *(&variable)
        // where we can directly identify the target variable
        if (ptr_node->type == AST_IDENTIFIER) {
            // Load the pointer value (which should be an address)
            target = generate_expression(data, ptr_node);
        } else {
            target = generate_expression(data, ptr_node);
        }
    }

    // Generate rvalue
    if (target) {
        LLVMValueRef value = generate_expression(data, node->data.assignment.value);
        if (value) {
            LLVMBuildStore(data->builder, value, target);
        }
    }
}

// Generate code for if statements
void generate_if_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_IF_STMT) {
        return;
    }

    // Generate condition
    LLVMValueRef cond = generate_expression(data, node->data.if_stmt.condition);
    if (!cond)
        return;

    // Convert to boolean if needed
    if (LLVMGetTypeKind(LLVMTypeOf(cond)) != LLVMIntegerTypeKind ||
        LLVMGetIntTypeWidth(LLVMTypeOf(cond)) != 1) {
        cond = LLVMBuildICmp(data->builder, LLVMIntNE, cond, LLVMConstNull(LLVMTypeOf(cond)),
                             "ifcond");
    }

    // Create basic blocks
    LLVMValueRef function = data->current_function;
    LLVMBasicBlockRef then_bb = LLVMAppendBasicBlockInContext(data->context, function, "then");
    LLVMBasicBlockRef else_bb = NULL;
    LLVMBasicBlockRef merge_bb = LLVMAppendBasicBlockInContext(data->context, function, "ifcont");

    if (node->data.if_stmt.else_block) {
        else_bb = LLVMAppendBasicBlockInContext(data->context, function, "else");
        LLVMBuildCondBr(data->builder, cond, then_bb, else_bb);
    } else {
        LLVMBuildCondBr(data->builder, cond, then_bb, merge_bb);
    }

    // Generate then block
    LLVMPositionBuilderAtEnd(data->builder, then_bb);
    generate_statement(data, node->data.if_stmt.then_block);
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
        LLVMBuildBr(data->builder, merge_bb);
    }

    // Generate else block if present
    if (else_bb) {
        LLVMPositionBuilderAtEnd(data->builder, else_bb);
        generate_statement(data, node->data.if_stmt.else_block);
        if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
            LLVMBuildBr(data->builder, merge_bb);
        }
    }

    // Continue with merge block
    LLVMPositionBuilderAtEnd(data->builder, merge_bb);
}

// Generate code for expression statements
void generate_expression_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_EXPR_STMT) {
        return;
    }

    // Expression statement - evaluate for side effects
    generate_expression(data, node->data.expr_stmt.expression);
}

// Generate code for break statements
void generate_break_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_BREAK_STMT) {
        return;
    }

    LoopContext *loop = llvm_backend_get_current_loop(data);
    if (!loop) {
        llvm_backend_report_error(data, node, "Break statement outside of loop");
        return;
    }

    // Jump to the break block (loop exit)
    LLVMBuildBr(data->builder, loop->break_block);
}

// Generate code for continue statements
void generate_continue_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_CONTINUE_STMT) {
        return;
    }

    LoopContext *loop = llvm_backend_get_current_loop(data);
    if (!loop) {
        llvm_backend_report_error(data, node, "Continue statement outside of loop");
        return;
    }

    // Jump to the continue block (next iteration)
    LLVMBuildBr(data->builder, loop->continue_block);
}

// Generate code for match statements
void generate_match_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_MATCH_STMT) {
        return;
    }

    // Generate code for match statement
    if (!node->data.match_stmt.expression) {
        llvm_backend_report_error(data, node, "Match statement missing expression");
        return;
    }

    if (!node->data.match_stmt.arms || node->data.match_stmt.arms->count == 0) {
        llvm_backend_report_error(data, node, "Match statement has no arms");
        return;
    }

    // Generate the expression to match against
    LLVMValueRef match_value = generate_expression(data, node->data.match_stmt.expression);
    if (!match_value) {
        llvm_backend_report_error(data, node, "Failed to generate match expression");
        return;
    }

    LLVMValueRef function = data->current_function;
    LLVMBasicBlockRef match_end_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "match_end");

    // Create the first pattern check block and branch to it
    LLVMBasicBlockRef first_check_bb = NULL;
    if (node->data.match_stmt.arms->count > 0) {
        first_check_bb = LLVMAppendBasicBlockInContext(data->context, function, "match_check_0");
        LLVMBuildBr(data->builder, first_check_bb);
        LLVMPositionBuilderAtEnd(data->builder, first_check_bb);
    }

    // Generate code for each match arm
    for (size_t i = 0; i < node->data.match_stmt.arms->count; i++) {
        ASTNode *arm = node->data.match_stmt.arms->nodes[i];
        if (!arm || arm->type != AST_MATCH_ARM) {
            llvm_backend_report_error(data, node, "Invalid match arm");
            continue;
        }

        // Create blocks for this arm
        char arm_name[64];
        snprintf(arm_name, sizeof(arm_name), "match_arm_%zu", i);
        LLVMBasicBlockRef arm_block =
            LLVMAppendBasicBlockInContext(data->context, function, arm_name);

        // Determine the next check block (where to go if this pattern doesn't match)
        LLVMBasicBlockRef next_check_block;
        if (i == node->data.match_stmt.arms->count - 1) {
            // Last arm - if it doesn't match, go to end (or error)
            next_check_block = match_end_bb;
        } else {
            // Create a block for checking the next pattern
            char next_check_name[64];
            snprintf(next_check_name, sizeof(next_check_name), "match_check_%zu", i + 1);
            next_check_block = LLVMAppendBasicBlockInContext(data->context, function, next_check_name);
        }

        // Generate pattern check from current position
        if (!generate_pattern_check(data, arm->data.match_arm.pattern, match_value, arm_block,
                                    next_check_block)) {
            llvm_backend_report_error(data, arm, "Failed to generate pattern check");
            continue;
        }

        // Generate arm body
        LLVMPositionBuilderAtEnd(data->builder, arm_block);

        // Bind pattern variables if needed
        if (arm->data.match_arm.pattern) {
            generate_identifier_pattern_binding(data, arm->data.match_arm.pattern, match_value);
        }

        // Generate guard check if present
        if (arm->data.match_arm.guard) {
            LLVMValueRef guard_value = generate_expression(data, arm->data.match_arm.guard);
            if (guard_value) {
                // Convert guard to boolean if needed
                if (LLVMGetTypeKind(LLVMTypeOf(guard_value)) != LLVMIntegerTypeKind ||
                    LLVMGetIntTypeWidth(LLVMTypeOf(guard_value)) != 1) {
                    guard_value = LLVMBuildICmp(data->builder, LLVMIntNE, guard_value,
                                                LLVMConstNull(LLVMTypeOf(guard_value)), "guard");
                }

                // Create blocks for guard check
                char guard_true_name[64];
                snprintf(guard_true_name, sizeof(guard_true_name), "match_guard_true_%zu", i);
                LLVMBasicBlockRef guard_true_bb =
                    LLVMAppendBasicBlockInContext(data->context, function, guard_true_name);

                // If guard fails, go to next arm
                LLVMBuildCondBr(data->builder, guard_value, guard_true_bb, next_check_block);
                LLVMPositionBuilderAtEnd(data->builder, guard_true_bb);
            }
        }

        // Generate arm body
        if (arm->data.match_arm.body) {
            generate_statement(data, arm->data.match_arm.body);
        }

        // Jump to end unless the body had a terminator
        if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
            LLVMBuildBr(data->builder, match_end_bb);
        }

        // Position builder at next check block for next iteration
        if (i < node->data.match_stmt.arms->count - 1) {
            LLVMPositionBuilderAtEnd(data->builder, next_check_block);
        }
    }

    // Continue after match
    LLVMPositionBuilderAtEnd(data->builder, match_end_bb);
}

// Generate code for block statements
void generate_block_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_BLOCK) {
        return;
    }

    // Create lexical scope for debug info
    LLVMMetadataRef old_scope = data->current_debug_scope;
    if (data->di_builder && data->current_debug_scope && node->location.line > 0) {
        data->current_debug_scope = LLVMDIBuilderCreateLexicalBlock(
            data->di_builder, data->current_debug_scope, data->di_file, node->location.line,
            node->location.column);
    }

    // Generate all statements in the block
    if (node->data.block.statements) {
        for (size_t i = 0; i < node->data.block.statements->count; i++) {
            generate_statement(data, node->data.block.statements->nodes[i]);
        }
    }

    // Restore previous scope
    data->current_debug_scope = old_scope;
}