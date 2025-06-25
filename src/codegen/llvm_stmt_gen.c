/**
 * Asthra Programming Language LLVM Statement Generation
 * Implementation of statement code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_stmt_gen.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_locals.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for pattern matching helpers
static bool generate_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                   LLVMValueRef value, LLVMBasicBlockRef match_block,
                                   LLVMBasicBlockRef next_block);
static bool generate_literal_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                           LLVMValueRef value, LLVMBasicBlockRef match_block,
                                           LLVMBasicBlockRef next_block);
static bool generate_enum_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                        LLVMValueRef value, LLVMBasicBlockRef match_block,
                                        LLVMBasicBlockRef next_block);
static bool generate_wildcard_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                            LLVMValueRef value, LLVMBasicBlockRef match_block,
                                            LLVMBasicBlockRef next_block);
static bool generate_identifier_pattern_binding(LLVMBackendData *data, const ASTNode *pattern,
                                                LLVMValueRef value);

// Forward declarations for for loop iterable helpers
static bool generate_range_loop(LLVMBackendData *data, const ASTNode *node,
                                const ASTNode *range_call);
static bool generate_array_loop(LLVMBackendData *data, const ASTNode *node, LLVMValueRef array);
static bool generate_slice_loop(LLVMBackendData *data, const ASTNode *node, LLVMValueRef slice);
static bool is_range_function_call(const ASTNode *node);
static bool get_range_bounds(const ASTNode *range_call, LLVMValueRef *start, LLVMValueRef *end,
                             LLVMBackendData *data);

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
    case AST_RETURN_STMT: {
        LLVMValueRef ret_val = NULL;
        if (node->data.return_stmt.expression) {
            // Check if the expression is a unit literal for void return
            if (node->data.return_stmt.expression->type == AST_UNIT_LITERAL) {
                // For unit literals in void functions, use RetVoid
                // In LLVM 15+, use LLVMGlobalGetValueType to get the function type
                LLVMTypeRef fn_type = LLVMGlobalGetValueType(data->current_function);
                LLVMTypeRef fn_ret_type = LLVMGetReturnType(fn_type);
                if (fn_ret_type == data->void_type) {
                    LLVMBuildRetVoid(data->builder);
                    break;
                }
            }
            ret_val = generate_expression(data, node->data.return_stmt.expression);
        }

        if (ret_val) {
            LLVMBuildRet(data->builder, ret_val);
        } else {
            LLVMBuildRetVoid(data->builder);
        }
    } break;

    case AST_LET_STMT: // Variable declaration
    {
        const char *var_name = node->data.let_stmt.name;
        LLVMTypeRef var_type = data->i32_type; // Default type

        // Get type from type_info if available
        if (node->type_info) {
            var_type = asthra_type_to_llvm(data, node->type_info);
        } else if (node->data.let_stmt.type && node->data.let_stmt.type->type_info) {
            var_type = asthra_type_to_llvm(data, node->data.let_stmt.type->type_info);
        }

        // Allocate space on stack
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
                data,
                node->type_info
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
    } break;

    case AST_ASSIGNMENT: {
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
        }

        // Generate rvalue
        if (target) {
            LLVMValueRef value = generate_expression(data, node->data.assignment.value);
            if (value) {
                LLVMBuildStore(data->builder, value, target);
            }
        }
    } break;

    case AST_IF_STMT: {
        // Generate condition
        LLVMValueRef cond = generate_expression(data, node->data.if_stmt.condition);
        if (!cond)
            break;

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
        LLVMBasicBlockRef merge_bb =
            LLVMAppendBasicBlockInContext(data->context, function, "ifcont");

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
    } break;

    case AST_FOR_STMT: {
        // Asthra for loops iterate over collections/ranges
        // for i in iterable { ... }

        if (!node->data.for_stmt.iterable) {
            llvm_backend_report_error(data, node, "For loop missing iterable expression");
            break;
        }

        if (!node->data.for_stmt.variable) {
            llvm_backend_report_error(data, node, "For loop missing variable name");
            break;
        }

        // Determine the type of iterable and generate appropriate loop
        bool success = false;

        // Check if it's a range function call
        if (is_range_function_call(node->data.for_stmt.iterable)) {
            success = generate_range_loop(data, node, node->data.for_stmt.iterable);
        } else {
            // Generate the iterable expression
            LLVMValueRef iterable_value = generate_expression(data, node->data.for_stmt.iterable);
            if (!iterable_value) {
                llvm_backend_report_error(data, node, "Failed to generate iterable expression");
                break;
            }

            // Check the type of the iterable value
            LLVMTypeRef iterable_type = LLVMTypeOf(iterable_value);
            LLVMTypeKind type_kind = LLVMGetTypeKind(iterable_type);

            if (type_kind == LLVMArrayTypeKind) {
                // Fixed-size array iteration
                success = generate_array_loop(data, node, iterable_value);
            } else if (type_kind == LLVMStructTypeKind) {
                // Could be a slice (represented as struct with ptr + len)
                success = generate_slice_loop(data, node, iterable_value);
            } else if (type_kind == LLVMPointerTypeKind) {
                // Could be a pointer to array or string
                success = generate_array_loop(data, node, iterable_value);
            } else {
                llvm_backend_report_error_printf(data, node, "Unsupported iterable type kind: %d",
                                                 type_kind);
                break;
            }
        }

        if (!success) {
            llvm_backend_report_error(data, node, "Failed to generate for loop iteration");
        }
    } break;

    case AST_BLOCK: {
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
    } break;

    case AST_EXPR_STMT: {
        // Expression statement - evaluate for side effects
        generate_expression(data, node->data.expr_stmt.expression);
    } break;

    case AST_BREAK_STMT: {
        LoopContext *loop = llvm_backend_get_current_loop(data);
        if (!loop) {
            llvm_backend_report_error(data, node, "Break statement outside of loop");
            break;
        }

        // Jump to the break block (loop exit)
        LLVMBuildBr(data->builder, loop->break_block);
    } break;

    case AST_CONTINUE_STMT: {
        LoopContext *loop = llvm_backend_get_current_loop(data);
        if (!loop) {
            llvm_backend_report_error(data, node, "Continue statement outside of loop");
            break;
        }

        // Jump to the continue block (next iteration)
        LLVMBuildBr(data->builder, loop->continue_block);
    } break;

    case AST_MATCH_STMT: {
        // Generate code for match statement
        if (!node->data.match_stmt.expression) {
            llvm_backend_report_error(data, node, "Match statement missing expression");
            break;
        }

        if (!node->data.match_stmt.arms || node->data.match_stmt.arms->count == 0) {
            llvm_backend_report_error(data, node, "Match statement has no arms");
            break;
        }

        // Generate the expression to match against
        LLVMValueRef match_value = generate_expression(data, node->data.match_stmt.expression);
        if (!match_value) {
            llvm_backend_report_error(data, node, "Failed to generate match expression");
            break;
        }

        LLVMValueRef function = data->current_function;
        LLVMBasicBlockRef match_end_bb =
            LLVMAppendBasicBlockInContext(data->context, function, "match_end");

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

            char next_name[64];
            snprintf(next_name, sizeof(next_name), "match_next_%zu", i);
            LLVMBasicBlockRef next_block =
                (i == node->data.match_stmt.arms->count - 1)
                    ? match_end_bb
                    : LLVMAppendBasicBlockInContext(data->context, function, next_name);

            // Generate pattern check
            if (!generate_pattern_check(data, arm->data.match_arm.pattern, match_value, arm_block,
                                        next_block)) {
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
                        guard_value =
                            LLVMBuildICmp(data->builder, LLVMIntNE, guard_value,
                                          LLVMConstNull(LLVMTypeOf(guard_value)), "guard");
                    }

                    // Create blocks for guard check
                    char guard_true_name[64];
                    snprintf(guard_true_name, sizeof(guard_true_name), "match_guard_true_%zu", i);
                    LLVMBasicBlockRef guard_true_bb =
                        LLVMAppendBasicBlockInContext(data->context, function, guard_true_name);

                    // If guard fails, go to next arm
                    LLVMBuildCondBr(data->builder, guard_value, guard_true_bb, next_block);
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

            // Position builder for next iteration
            if (i < node->data.match_stmt.arms->count - 1) {
                LLVMPositionBuilderAtEnd(data->builder, next_block);
            }
        }

        // Continue after match
        LLVMPositionBuilderAtEnd(data->builder, match_end_bb);
    } break;

    default:
        llvm_backend_report_error_printf(data, node, "Unknown statement type: %d", node->type);
        break;
    }
}

// ===== Pattern Matching Helper Functions =====

// Generate pattern check - returns true if pattern check was generated successfully
static bool generate_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                   LLVMValueRef value, LLVMBasicBlockRef match_block,
                                   LLVMBasicBlockRef next_block) {
    if (!data || !pattern || !value || !match_block || !next_block) {
        return false;
    }

    switch (pattern->type) {
    case AST_INTEGER_LITERAL:
    case AST_FLOAT_LITERAL:
    case AST_STRING_LITERAL:
    case AST_BOOL_LITERAL:
    case AST_CHAR_LITERAL:
        return generate_literal_pattern_check(data, pattern, value, match_block, next_block);

    case AST_ENUM_PATTERN:
        return generate_enum_pattern_check(data, pattern, value, match_block, next_block);

    case AST_WILDCARD_PATTERN:
        return generate_wildcard_pattern_check(data, pattern, value, match_block, next_block);

    case AST_IDENTIFIER:
        // Identifier patterns always match (binding pattern)
        LLVMBuildBr(data->builder, match_block);
        return true;

    case AST_STRUCT_PATTERN:
        // TODO: Implement struct pattern matching
        llvm_backend_report_error(data, pattern, "Struct pattern matching not yet implemented");
        return false;

    case AST_TUPLE_PATTERN:
        // TODO: Implement tuple pattern matching
        llvm_backend_report_error(data, pattern, "Tuple pattern matching not yet implemented");
        return false;

    default:
        llvm_backend_report_error_printf(data, pattern, "Unsupported pattern type: %d",
                                         pattern->type);
        return false;
    }
}

// Generate literal pattern check
static bool generate_literal_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                           LLVMValueRef value, LLVMBasicBlockRef match_block,
                                           LLVMBasicBlockRef next_block) {
    if (!data || !pattern || !value || !match_block || !next_block) {
        return false;
    }

    LLVMValueRef pattern_value = generate_expression(data, pattern);
    if (!pattern_value) {
        return false;
    }

    // Generate comparison
    LLVMValueRef cmp_result = NULL;
    LLVMTypeKind value_kind = LLVMGetTypeKind(LLVMTypeOf(value));

    if (value_kind == LLVMFloatTypeKind || value_kind == LLVMDoubleTypeKind) {
        // Floating point comparison
        cmp_result = LLVMBuildFCmp(data->builder, LLVMRealOEQ, value, pattern_value, "pattern_cmp");
    } else {
        // Integer comparison (includes booleans and characters)
        cmp_result = LLVMBuildICmp(data->builder, LLVMIntEQ, value, pattern_value, "pattern_cmp");
    }

    if (!cmp_result) {
        return false;
    }

    // Generate conditional branch
    LLVMBuildCondBr(data->builder, cmp_result, match_block, next_block);
    return true;
}

// Generate enum pattern check
static bool generate_enum_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                        LLVMValueRef value, LLVMBasicBlockRef match_block,
                                        LLVMBasicBlockRef next_block) {
    if (!data || !pattern || !value || !match_block || !next_block) {
        return false;
    }

    // For now, implement a simple enum variant check
    // TODO: This needs to be enhanced to handle actual enum types and extract variant data

    // Assume enum is represented as an integer tag
    // Extract the tag from the enum value (assume first field)
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i32_type, 0, false), // struct index
        LLVMConstInt(data->i32_type, 0, false)  // tag field index
    };

    LLVMTypeRef enum_type = LLVMTypeOf(value);
    LLVMValueRef tag_ptr =
        LLVMBuildGEP2(data->builder, enum_type, value, indices, 2, "enum_tag_ptr");
    LLVMValueRef tag_value = LLVMBuildLoad2(data->builder, data->i32_type, tag_ptr, "enum_tag");

    // For now, assume the variant name maps to an integer constant
    // TODO: This should use actual enum variant information from type system
    LLVMValueRef expected_tag = LLVMConstInt(data->i32_type, 0, false); // Placeholder

    LLVMValueRef cmp_result =
        LLVMBuildICmp(data->builder, LLVMIntEQ, tag_value, expected_tag, "enum_pattern_cmp");
    LLVMBuildCondBr(data->builder, cmp_result, match_block, next_block);

    return true;
}

// Generate wildcard pattern check (always matches)
static bool generate_wildcard_pattern_check(LLVMBackendData *data, const ASTNode *pattern,
                                            LLVMValueRef value, LLVMBasicBlockRef match_block,
                                            LLVMBasicBlockRef next_block) {
    if (!data || !match_block) {
        return false;
    }

    // Wildcard pattern always matches
    LLVMBuildBr(data->builder, match_block);
    return true;
}

// Generate identifier pattern binding
static bool generate_identifier_pattern_binding(LLVMBackendData *data, const ASTNode *pattern,
                                                LLVMValueRef value) {
    if (!data || !pattern || !value) {
        return false;
    }

    // Only bind for identifier patterns
    if (pattern->type == AST_IDENTIFIER) {
        const char *var_name = pattern->data.identifier.name;
        if (var_name) {
            // Create local variable for the binding
            LLVMTypeRef value_type = LLVMTypeOf(value);
            LLVMValueRef alloca = LLVMBuildAlloca(data->builder, value_type, var_name);
            LLVMBuildStore(data->builder, value, alloca);

            // Register the binding for later lookup
            register_local_var(data, var_name, alloca, value_type);
            return true;
        }
    }

    return false;
}

// ===== For Loop Iterable Helper Functions =====

// Check if a node is a range function call
static bool is_range_function_call(const ASTNode *node) {
    if (!node || node->type != AST_CALL_EXPR) {
        return false;
    }

    // Check if the function being called is an identifier named "range"
    if (node->data.call_expr.function && node->data.call_expr.function->type == AST_IDENTIFIER &&
        node->data.call_expr.function->data.identifier.name &&
        strcmp(node->data.call_expr.function->data.identifier.name, "range") == 0) {
        return true;
    }

    return false;
}

// Extract range bounds from range function call
static bool get_range_bounds(const ASTNode *range_call, LLVMValueRef *start, LLVMValueRef *end,
                             LLVMBackendData *data) {
    if (!range_call || !range_call->data.call_expr.args) {
        return false;
    }

    size_t arg_count = range_call->data.call_expr.args->count;

    if (arg_count == 1) {
        // range(end) -> start=0, end=arg
        *start = LLVMConstInt(data->i32_type, 0, false);
        *end = generate_expression(data, range_call->data.call_expr.args->nodes[0]);
        return *end != NULL;
    } else if (arg_count == 2) {
        // range(start, end) -> start=arg1, end=arg2
        *start = generate_expression(data, range_call->data.call_expr.args->nodes[0]);
        *end = generate_expression(data, range_call->data.call_expr.args->nodes[1]);
        return *start != NULL && *end != NULL;
    }

    return false;
}

// Generate code for range iteration
static bool generate_range_loop(LLVMBackendData *data, const ASTNode *node,
                                const ASTNode *range_call) {
    if (!data || !node || !range_call) {
        return false;
    }

    // Extract range bounds
    LLVMValueRef start_val, end_val;
    if (!get_range_bounds(range_call, &start_val, &end_val, data)) {
        llvm_backend_report_error(data, node, "Invalid range function call arguments");
        return false;
    }

    // Create basic blocks
    LLVMValueRef function = data->current_function;
    LLVMBasicBlockRef loop_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "range_loop");
    LLVMBasicBlockRef loop_body_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "range_body");
    LLVMBasicBlockRef loop_end_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "range_end");

    // Create loop variable
    LLVMTypeRef loop_var_type = data->i32_type;
    LLVMValueRef loop_var =
        LLVMBuildAlloca(data->builder, loop_var_type, node->data.for_stmt.variable);
    LLVMBuildStore(data->builder, start_val, loop_var);

    // Register loop variable for later lookup
    register_local_var(data, node->data.for_stmt.variable, loop_var, loop_var_type);

    // Push loop context for break/continue statements
    llvm_backend_push_loop_context(data, loop_bb, loop_end_bb);

    // Jump to loop header
    LLVMBuildBr(data->builder, loop_bb);

    // Loop header - check condition
    LLVMPositionBuilderAtEnd(data->builder, loop_bb);
    LLVMValueRef current_val = LLVMBuildLoad2(data->builder, loop_var_type, loop_var, "range_var");
    LLVMValueRef cond =
        LLVMBuildICmp(data->builder, LLVMIntSLT, current_val, end_val, "range_cond");
    LLVMBuildCondBr(data->builder, cond, loop_body_bb, loop_end_bb);

    // Loop body
    LLVMPositionBuilderAtEnd(data->builder, loop_body_bb);
    generate_statement(data, node->data.for_stmt.body);

    // Check if the body ended with a terminator (break/continue/return)
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
        // Increment loop variable and continue to next iteration
        LLVMValueRef inc_val = LLVMBuildAdd(data->builder, current_val,
                                            LLVMConstInt(loop_var_type, 1, false), "range_inc");
        LLVMBuildStore(data->builder, inc_val, loop_var);
        LLVMBuildBr(data->builder, loop_bb);
    }

    // Pop loop context
    llvm_backend_pop_loop_context(data);

    // Continue after loop
    LLVMPositionBuilderAtEnd(data->builder, loop_end_bb);

    return true;
}

// Generate code for array iteration
static bool generate_array_loop(LLVMBackendData *data, const ASTNode *node, LLVMValueRef array) {
    if (!data || !node || !array) {
        return false;
    }

    // Get array type information
    LLVMTypeRef array_type = LLVMTypeOf(array);
    LLVMTypeKind type_kind = LLVMGetTypeKind(array_type);

    LLVMValueRef array_length = NULL;
    LLVMTypeRef element_type = NULL;

    if (type_kind == LLVMArrayTypeKind) {
        // Fixed-size array
        unsigned array_size = LLVMGetArrayLength(array_type);
        array_length = LLVMConstInt(data->i32_type, array_size, false);
        element_type = LLVMGetElementType(array_type);
    } else if (type_kind == LLVMPointerTypeKind) {
        // Assume it's a pointer to an array or a dynamically-sized array
        // For now, we'll need additional type information to determine length
        llvm_backend_report_error(data, node, "Dynamic array iteration not yet fully implemented");
        return false;
    } else {
        llvm_backend_report_error(data, node, "Invalid array type for iteration");
        return false;
    }

    // Create basic blocks
    LLVMValueRef function = data->current_function;
    LLVMBasicBlockRef loop_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "array_loop");
    LLVMBasicBlockRef loop_body_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "array_body");
    LLVMBasicBlockRef loop_end_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "array_end");

    // Create index variable
    LLVMTypeRef index_type = data->i32_type;
    LLVMValueRef index_var = LLVMBuildAlloca(data->builder, index_type, "array_index");
    LLVMBuildStore(data->builder, LLVMConstInt(index_type, 0, false), index_var);

    // Create element variable (loop variable)
    LLVMValueRef element_var =
        LLVMBuildAlloca(data->builder, element_type, node->data.for_stmt.variable);

    // Register loop variable for later lookup
    register_local_var(data, node->data.for_stmt.variable, element_var, element_type);

    // Push loop context for break/continue statements
    llvm_backend_push_loop_context(data, loop_bb, loop_end_bb);

    // Jump to loop header
    LLVMBuildBr(data->builder, loop_bb);

    // Loop header - check condition
    LLVMPositionBuilderAtEnd(data->builder, loop_bb);
    LLVMValueRef current_index =
        LLVMBuildLoad2(data->builder, index_type, index_var, "array_index");
    LLVMValueRef cond =
        LLVMBuildICmp(data->builder, LLVMIntSLT, current_index, array_length, "array_cond");
    LLVMBuildCondBr(data->builder, cond, loop_body_bb, loop_end_bb);

    // Loop body - load current element
    LLVMPositionBuilderAtEnd(data->builder, loop_body_bb);

    // Get pointer to current element
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i32_type, 0, false), // array index
        current_index                           // element index
    };
    LLVMValueRef element_ptr =
        LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "array_elem_ptr");
    LLVMValueRef element_value =
        LLVMBuildLoad2(data->builder, element_type, element_ptr, "array_elem");

    // Store current element in loop variable
    LLVMBuildStore(data->builder, element_value, element_var);

    // Generate loop body
    generate_statement(data, node->data.for_stmt.body);

    // Check if the body ended with a terminator (break/continue/return)
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
        // Increment index and continue to next iteration
        LLVMValueRef inc_index = LLVMBuildAdd(data->builder, current_index,
                                              LLVMConstInt(index_type, 1, false), "array_inc");
        LLVMBuildStore(data->builder, inc_index, index_var);
        LLVMBuildBr(data->builder, loop_bb);
    }

    // Pop loop context
    llvm_backend_pop_loop_context(data);

    // Continue after loop
    LLVMPositionBuilderAtEnd(data->builder, loop_end_bb);

    return true;
}

// Generate code for slice iteration
static bool generate_slice_loop(LLVMBackendData *data, const ASTNode *node, LLVMValueRef slice) {
    if (!data || !node || !slice) {
        return false;
    }

    // Assume slice is represented as struct { ptr: *T, len: usize }
    LLVMTypeRef slice_type = LLVMTypeOf(slice);

    // Extract pointer and length from slice
    LLVMValueRef ptr_indices[2] = {
        LLVMConstInt(data->i32_type, 0, false), // struct index
        LLVMConstInt(data->i32_type, 0, false)  // ptr field (assumed first)
    };
    LLVMValueRef len_indices[2] = {
        LLVMConstInt(data->i32_type, 0, false), // struct index
        LLVMConstInt(data->i32_type, 1, false)  // len field (assumed second)
    };

    LLVMValueRef ptr_ptr =
        LLVMBuildGEP2(data->builder, slice_type, slice, ptr_indices, 2, "slice_ptr_ptr");
    LLVMValueRef len_ptr =
        LLVMBuildGEP2(data->builder, slice_type, slice, len_indices, 2, "slice_len_ptr");

    LLVMValueRef array_ptr = LLVMBuildLoad2(data->builder, data->ptr_type, ptr_ptr, "slice_ptr");
    LLVMValueRef array_length = LLVMBuildLoad2(data->builder, data->i64_type, len_ptr, "slice_len");

    // Convert length to i32 for consistency
    LLVMValueRef length_i32 =
        LLVMBuildTrunc(data->builder, array_length, data->i32_type, "slice_len_i32");

    // Create basic blocks
    LLVMValueRef function = data->current_function;
    LLVMBasicBlockRef loop_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "slice_loop");
    LLVMBasicBlockRef loop_body_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "slice_body");
    LLVMBasicBlockRef loop_end_bb =
        LLVMAppendBasicBlockInContext(data->context, function, "slice_end");

    // Create index variable
    LLVMTypeRef index_type = data->i32_type;
    LLVMValueRef index_var = LLVMBuildAlloca(data->builder, index_type, "slice_index");
    LLVMBuildStore(data->builder, LLVMConstInt(index_type, 0, false), index_var);

    // Create element variable (we'll use a generic pointer for now)
    LLVMValueRef element_var =
        LLVMBuildAlloca(data->builder, data->ptr_type, node->data.for_stmt.variable);

    // Register loop variable for later lookup
    register_local_var(data, node->data.for_stmt.variable, element_var, data->ptr_type);

    // Push loop context for break/continue statements
    llvm_backend_push_loop_context(data, loop_bb, loop_end_bb);

    // Jump to loop header
    LLVMBuildBr(data->builder, loop_bb);

    // Loop header - check condition
    LLVMPositionBuilderAtEnd(data->builder, loop_bb);
    LLVMValueRef current_index =
        LLVMBuildLoad2(data->builder, index_type, index_var, "slice_index");
    LLVMValueRef cond =
        LLVMBuildICmp(data->builder, LLVMIntSLT, current_index, length_i32, "slice_cond");
    LLVMBuildCondBr(data->builder, cond, loop_body_bb, loop_end_bb);

    // Loop body - load current element
    LLVMPositionBuilderAtEnd(data->builder, loop_body_bb);

    // Get pointer to current element (assuming pointer arithmetic)
    LLVMValueRef current_index_64 =
        LLVMBuildZExt(data->builder, current_index, data->i64_type, "slice_index_64");
    LLVMValueRef element_ptr = LLVMBuildGEP2(data->builder, data->ptr_type, array_ptr,
                                             &current_index_64, 1, "slice_elem_ptr");

    // Store current element pointer in loop variable (for now)
    LLVMBuildStore(data->builder, element_ptr, element_var);

    // Generate loop body
    generate_statement(data, node->data.for_stmt.body);

    // Check if the body ended with a terminator (break/continue/return)
    if (!LLVMGetBasicBlockTerminator(LLVMGetInsertBlock(data->builder))) {
        // Increment index and continue to next iteration
        LLVMValueRef inc_index = LLVMBuildAdd(data->builder, current_index,
                                              LLVMConstInt(index_type, 1, false), "slice_inc");
        LLVMBuildStore(data->builder, inc_index, index_var);
        LLVMBuildBr(data->builder, loop_bb);
    }

    // Pop loop context
    llvm_backend_pop_loop_context(data);

    // Continue after loop
    LLVMPositionBuilderAtEnd(data->builder, loop_end_bb);

    return true;
}