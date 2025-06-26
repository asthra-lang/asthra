/**
 * Asthra Programming Language LLVM Binary Operations
 * Implementation of binary operation code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_binary_ops.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include <stdio.h>
#include <stdlib.h>

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

    LLVMValueRef left = generate_expression(data, node->data.binary_expr.left);
    if (!left) {
        LLVM_REPORT_ERROR(data, node, "Failed to generate left operand for binary operation");
    }

    LLVMValueRef right = generate_expression(data, node->data.binary_expr.right);
    if (!right) {
        LLVM_REPORT_ERROR(data, node, "Failed to generate right operand for binary operation");
    }

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

    case BINOP_BITWISE_AND:
        return LLVMBuildAnd(data->builder, left, right, "bitand");

    case BINOP_BITWISE_OR:
        return LLVMBuildOr(data->builder, left, right, "bitor");

    case BINOP_BITWISE_XOR:
        return LLVMBuildXor(data->builder, left, right, "bitxor");

    case BINOP_LSHIFT:
        return LLVMBuildShl(data->builder, left, right, "shl");

    case BINOP_RSHIFT:
        // TODO: Handle signed vs unsigned right shift
        return LLVMBuildAShr(data->builder, left, right, "ashr");

    default:
        return NULL;
    }
}