/**
 * Asthra Programming Language LLVM Unary Operations
 * Implementation of unary operation code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_unary_ops.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include <stdio.h>
#include <stdlib.h>

// Generate code for unary operations
LLVMValueRef generate_unary_op(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    if (!node->data.unary_expr.operand) {
        LLVM_REPORT_ERROR(data, node, "Unary operation missing operand");
    }

    LLVMValueRef operand = generate_expression(data, node->data.unary_expr.operand);
    if (!operand) {
        LLVM_REPORT_ERROR(data, node, "Failed to generate operand for unary operation");
    }

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

    case UNOP_DEREF: {
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
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported unary operator: %d",
                                 node->data.unary_expr.operator);
    }
}