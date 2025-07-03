/**
 * Asthra Programming Language LLVM Binary Operations
 * Implementation of binary operation code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_binary_ops.h"
#include "../analysis/type_info_types.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_types.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Helper function to determine if a type is signed
static bool is_signed_integer_type(TypeInfo *type_info) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE) {
        return true; // Default to signed for safety
    }

    switch (type_info->data.primitive.kind) {
    case PRIMITIVE_INFO_I8:
    case PRIMITIVE_INFO_I16:
    case PRIMITIVE_INFO_I32:
    case PRIMITIVE_INFO_I64:
    case PRIMITIVE_INFO_I128:
    case PRIMITIVE_INFO_ISIZE:
        return true;

    case PRIMITIVE_INFO_U8:
    case PRIMITIVE_INFO_U16:
    case PRIMITIVE_INFO_U32:
    case PRIMITIVE_INFO_U64:
    case PRIMITIVE_INFO_U128:
    case PRIMITIVE_INFO_USIZE:
        return false;

    default:
        return true; // Default to signed for other types
    }
}

// Generate code for binary operations
LLVMValueRef generate_binary_op(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    if (!node->data.binary_expr.left) {
        LLVM_REPORT_ERROR(data, node, "Binary operation missing left operand");
    }

    if (!node->data.binary_expr.right) {
        LLVM_REPORT_ERROR(data, node, "Binary operation missing right operand");
    }

    // Special handling for short-circuit operators
    if (node->data.binary_expr.operator== BINOP_AND || node->data.binary_expr.operator== BINOP_OR) {
        // For AND/OR, we handle evaluation in the switch cases themselves
        // to implement proper short-circuit behavior
    }

    LLVMValueRef left = NULL;
    LLVMValueRef right = NULL;

    // For non-short-circuit operators, evaluate both operands immediately
    if (node->data.binary_expr.operator!= BINOP_AND && node->data.binary_expr.operator!= BINOP_OR) {
        left = generate_expression(data, node->data.binary_expr.left);
        if (!left) {
            LLVM_REPORT_ERROR(data, node, "Failed to generate left operand for binary operation");
        }

        right = generate_expression(data, node->data.binary_expr.right);
        if (!right) {
            LLVM_REPORT_ERROR(data, node, "Failed to generate right operand for binary operation");
        }
    }

    switch (node->data.binary_expr.operator) {
    case BINOP_ADD:
        // Check if this is string concatenation
        if (node->type_info && node->type_info->category == TYPE_INFO_PRIMITIVE &&
            node->type_info->data.primitive.kind == PRIMITIVE_INFO_STRING) {
            // Call string concatenation runtime function
            if (!data->runtime_string_concat_fn) {
                LLVM_REPORT_ERROR(data, node, "String concatenation function not declared");
            }
            LLVMValueRef args[] = {left, right};
            // Get the function type properly
            LLVMTypeRef param_types[] = {data->ptr_type, data->ptr_type};
            LLVMTypeRef fn_type = LLVMFunctionType(data->ptr_type, param_types, 2, false);
            return LLVMBuildCall2(data->builder, fn_type, data->runtime_string_concat_fn, args, 2,
                                  "string_concat");
        }
        // OPTIMIZATION: Check for pointer arithmetic (ptr + integer)
        else if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMPointerTypeKind) {
            // Left is pointer, right should be integer offset
            // Use GEP for more efficient pointer arithmetic
            TypeInfo *left_type_info = node->data.binary_expr.left->type_info;
            if (left_type_info && left_type_info->category == TYPE_INFO_POINTER) {
                TypeInfo *pointee_type_info = left_type_info->data.pointer.pointee_type;
                LLVMTypeRef pointee_type = asthra_type_to_llvm(data, pointee_type_info);
                return LLVMBuildGEP2(data->builder, pointee_type, left, &right, 1, "ptr_add");
            } else {
                // Fallback to regular pointer addition
                return LLVMBuildAdd(data->builder, left, right, "add");
            }
        } else if (LLVMGetTypeKind(LLVMTypeOf(right)) == LLVMPointerTypeKind) {
            // Right is pointer, left should be integer offset (commutative)
            // Use GEP for more efficient pointer arithmetic
            TypeInfo *right_type_info = node->data.binary_expr.right->type_info;
            if (right_type_info && right_type_info->category == TYPE_INFO_POINTER) {
                TypeInfo *pointee_type_info = right_type_info->data.pointer.pointee_type;
                LLVMTypeRef pointee_type = asthra_type_to_llvm(data, pointee_type_info);
                return LLVMBuildGEP2(data->builder, pointee_type, right, &left, 1, "ptr_add");
            } else {
                // Fallback to regular addition
                return LLVMBuildAdd(data->builder, left, right, "add");
            }
        } else if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
                   LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
            return LLVMBuildFAdd(data->builder, left, right, "add");
        } else {
            return LLVMBuildAdd(data->builder, left, right, "add");
        }

    case BINOP_SUB:
        // OPTIMIZATION: Check for pointer arithmetic (ptr - integer) or pointer difference
        if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMPointerTypeKind) {
            if (LLVMGetTypeKind(LLVMTypeOf(right)) == LLVMPointerTypeKind) {
                // Pointer difference - calculate number of elements between pointers
                TypeInfo *left_type_info = node->data.binary_expr.left->type_info;
                if (left_type_info && left_type_info->category == TYPE_INFO_POINTER) {
                    TypeInfo *pointee_type_info = left_type_info->data.pointer.pointee_type;
                    LLVMTypeRef pointee_type = asthra_type_to_llvm(data, pointee_type_info);

                    // Convert pointers to integers for subtraction
                    LLVMValueRef left_int =
                        LLVMBuildPtrToInt(data->builder, left, data->i64_type, "ptr_to_int_left");
                    LLVMValueRef right_int =
                        LLVMBuildPtrToInt(data->builder, right, data->i64_type, "ptr_to_int_right");
                    LLVMValueRef byte_diff =
                        LLVMBuildSub(data->builder, left_int, right_int, "byte_diff");

                    // Divide by element size to get element count
                    LLVMTypeRef elem_types[] = {pointee_type};
                    size_t elem_size =
                        LLVMStoreSizeOfType(LLVMGetModuleDataLayout(data->module), pointee_type);
                    LLVMValueRef size_val = LLVMConstInt(data->i64_type, elem_size, false);
                    return LLVMBuildSDiv(data->builder, byte_diff, size_val, "ptr_diff");
                }
            } else {
                // Pointer - integer: use negative GEP
                TypeInfo *left_type_info = node->data.binary_expr.left->type_info;
                if (left_type_info && left_type_info->category == TYPE_INFO_POINTER) {
                    TypeInfo *pointee_type_info = left_type_info->data.pointer.pointee_type;
                    LLVMTypeRef pointee_type = asthra_type_to_llvm(data, pointee_type_info);
                    LLVMValueRef neg_offset = LLVMBuildNeg(data->builder, right, "neg_offset");
                    return LLVMBuildGEP2(data->builder, pointee_type, left, &neg_offset, 1,
                                         "ptr_sub");
                }
            }
        } else if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
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
            // Handle signed vs unsigned division
            TypeInfo *left_type = node->data.binary_expr.left->type_info;
            if (is_signed_integer_type(left_type)) {
                return LLVMBuildSDiv(data->builder, left, right, "sdiv");
            } else {
                return LLVMBuildUDiv(data->builder, left, right, "udiv");
            }
        }

    case BINOP_MOD: {
        // Handle signed vs unsigned modulo
        TypeInfo *left_type = node->data.binary_expr.left->type_info;
        if (is_signed_integer_type(left_type)) {
            return LLVMBuildSRem(data->builder, left, right, "srem");
        } else {
            return LLVMBuildURem(data->builder, left, right, "urem");
        }
    }

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
            // Handle signed vs unsigned comparison
            TypeInfo *left_type = node->data.binary_expr.left->type_info;
            if (is_signed_integer_type(left_type)) {
                return LLVMBuildICmp(data->builder, LLVMIntSLT, left, right, "slt");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntULT, left, right, "ult");
            }
        }

    case BINOP_GT:
        if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
            LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
            return LLVMBuildFCmp(data->builder, LLVMRealOGT, left, right, "gt");
        } else {
            // Handle signed vs unsigned comparison
            TypeInfo *left_type = node->data.binary_expr.left->type_info;
            if (is_signed_integer_type(left_type)) {
                return LLVMBuildICmp(data->builder, LLVMIntSGT, left, right, "sgt");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntUGT, left, right, "ugt");
            }
        }

    case BINOP_LE:
        if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
            LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
            return LLVMBuildFCmp(data->builder, LLVMRealOLE, left, right, "le");
        } else {
            // Handle signed vs unsigned comparison
            TypeInfo *left_type = node->data.binary_expr.left->type_info;
            if (is_signed_integer_type(left_type)) {
                return LLVMBuildICmp(data->builder, LLVMIntSLE, left, right, "sle");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntULE, left, right, "ule");
            }
        }

    case BINOP_GE:
        if (LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMFloatTypeKind ||
            LLVMGetTypeKind(LLVMTypeOf(left)) == LLVMDoubleTypeKind) {
            return LLVMBuildFCmp(data->builder, LLVMRealOGE, left, right, "ge");
        } else {
            // Handle signed vs unsigned comparison
            TypeInfo *left_type = node->data.binary_expr.left->type_info;
            if (is_signed_integer_type(left_type)) {
                return LLVMBuildICmp(data->builder, LLVMIntSGE, left, right, "sge");
            } else {
                return LLVMBuildICmp(data->builder, LLVMIntUGE, left, right, "uge");
            }
        }

    case BINOP_AND: {
        // Implement short-circuit evaluation for logical AND
        // First evaluate the left operand
        left = generate_expression(data, node->data.binary_expr.left);
        if (!left) {
            LLVM_REPORT_ERROR(data, node, "Failed to generate left operand for logical AND");
        }

        // Create blocks for short-circuit evaluation
        LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(data->builder);
        LLVMValueRef function = LLVMGetBasicBlockParent(current_bb);

        LLVMBasicBlockRef eval_right_bb =
            LLVMAppendBasicBlockInContext(data->context, function, "and.rhs");
        LLVMBasicBlockRef merge_bb =
            LLVMAppendBasicBlockInContext(data->context, function, "and.end");

        // Check if left is true; if false, short-circuit
        LLVMBuildCondBr(data->builder, left, eval_right_bb, merge_bb);

        // Evaluate right side
        LLVMPositionBuilderAtEnd(data->builder, eval_right_bb);
        LLVMValueRef right_val = generate_expression(data, node->data.binary_expr.right);
        if (!right_val) {
            LLVM_REPORT_ERROR(data, node, "Failed to generate right operand for logical AND");
        }
        LLVMBasicBlockRef right_bb = LLVMGetInsertBlock(data->builder);
        LLVMBuildBr(data->builder, merge_bb);

        // Merge results
        LLVMPositionBuilderAtEnd(data->builder, merge_bb);
        LLVMValueRef phi = LLVMBuildPhi(data->builder, data->bool_type, "and.result");

        LLVMValueRef false_val = LLVMConstInt(data->bool_type, 0, 0);
        LLVMValueRef incoming_values[] = {false_val, right_val};
        LLVMBasicBlockRef incoming_blocks[] = {current_bb, right_bb};
        LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

        return phi;
    }

    case BINOP_OR: {
        // Implement short-circuit evaluation for logical OR
        // First evaluate the left operand
        left = generate_expression(data, node->data.binary_expr.left);
        if (!left) {
            LLVM_REPORT_ERROR(data, node, "Failed to generate left operand for logical OR");
        }

        // Create blocks for short-circuit evaluation
        LLVMBasicBlockRef current_bb = LLVMGetInsertBlock(data->builder);
        LLVMValueRef function = LLVMGetBasicBlockParent(current_bb);

        LLVMBasicBlockRef eval_right_bb =
            LLVMAppendBasicBlockInContext(data->context, function, "or.rhs");
        LLVMBasicBlockRef merge_bb =
            LLVMAppendBasicBlockInContext(data->context, function, "or.end");

        // Check if left is true; if true, short-circuit
        LLVMBuildCondBr(data->builder, left, merge_bb, eval_right_bb);

        // Evaluate right side
        LLVMPositionBuilderAtEnd(data->builder, eval_right_bb);
        LLVMValueRef right_val = generate_expression(data, node->data.binary_expr.right);
        if (!right_val) {
            LLVM_REPORT_ERROR(data, node, "Failed to generate right operand for logical OR");
        }
        LLVMBasicBlockRef right_bb = LLVMGetInsertBlock(data->builder);
        LLVMBuildBr(data->builder, merge_bb);

        // Merge results
        LLVMPositionBuilderAtEnd(data->builder, merge_bb);
        LLVMValueRef phi = LLVMBuildPhi(data->builder, data->bool_type, "or.result");

        LLVMValueRef true_val = LLVMConstInt(data->bool_type, 1, 0);
        LLVMValueRef incoming_values[] = {true_val, right_val};
        LLVMBasicBlockRef incoming_blocks[] = {current_bb, right_bb};
        LLVMAddIncoming(phi, incoming_values, incoming_blocks, 2);

        return phi;
    }

    case BINOP_BITWISE_AND:
        return LLVMBuildAnd(data->builder, left, right, "bitand");

    case BINOP_BITWISE_OR:
        return LLVMBuildOr(data->builder, left, right, "bitor");

    case BINOP_BITWISE_XOR:
        return LLVMBuildXor(data->builder, left, right, "bitxor");

    case BINOP_LSHIFT:
        return LLVMBuildShl(data->builder, left, right, "shl");

    case BINOP_RSHIFT: {
        // Handle signed vs unsigned right shift
        TypeInfo *left_type = node->data.binary_expr.left->type_info;
        if (is_signed_integer_type(left_type)) {
            return LLVMBuildAShr(data->builder, left, right, "ashr");
        } else {
            return LLVMBuildLShr(data->builder, left, right, "lshr");
        }
    }

    default:
        return NULL;
    }
}