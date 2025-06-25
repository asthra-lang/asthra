/**
 * Asthra Programming Language LLVM For Loop Iteration
 * Implementation of for loop code generation with different iterable types
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_for_loops.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_locals.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration for generate_statement function
void generate_statement(LLVMBackendData *data, const ASTNode *node);

// Forward declarations for internal helper functions
static bool generate_range_loop(LLVMBackendData *data, const ASTNode *node,
                                const ASTNode *range_call);
static bool generate_array_loop(LLVMBackendData *data, const ASTNode *node, LLVMValueRef array);
static bool generate_slice_loop(LLVMBackendData *data, const ASTNode *node, LLVMValueRef slice);
static bool is_range_function_call(const ASTNode *node);
static bool get_range_bounds(const ASTNode *range_call, LLVMValueRef *start, LLVMValueRef *end,
                             LLVMBackendData *data);

// Generate code for for loop iteration over different iterable types
bool generate_for_loop_iteration(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node || node->type != AST_FOR_STMT) {
        return false;
    }

    if (!node->data.for_stmt.iterable) {
        llvm_backend_report_error(data, node, "For loop missing iterable expression");
        return false;
    }

    if (!node->data.for_stmt.variable) {
        llvm_backend_report_error(data, node, "For loop missing variable name");
        return false;
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
            return false;
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
            return false;
        }
    }

    return success;
}

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