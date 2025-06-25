/**
 * Asthra Programming Language LLVM Cast Expressions
 * Implementation of type casting code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_cast_expr.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>

// Generate code for cast expressions
LLVMValueRef generate_cast_expr(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    if (!node->data.cast_expr.expression) {
        LLVM_REPORT_ERROR(data, node, "Cast expression missing source expression");
    }

    // Check that we have either target_type AST node or type_info
    if (!node->data.cast_expr.target_type && !node->type_info) {
        LLVM_REPORT_ERROR(data, node, "Cast expression missing target type");
    }

    // Generate the source expression
    LLVMValueRef source_value = generate_expression(data, node->data.cast_expr.expression);
    if (!source_value) {
        LLVM_REPORT_ERROR(data, node, "Failed to generate source expression for cast");
    }

    // Get source and target types
    LLVMTypeRef source_type = LLVMTypeOf(source_value);

    // Get target type from type_info if available, otherwise from target_type AST node
    LLVMTypeRef target_type = NULL;
    if (node->type_info) {
        target_type = asthra_type_to_llvm(data, node->type_info);
    } else if (node->data.cast_expr.target_type->type_info) {
        target_type = asthra_type_to_llvm(data, node->data.cast_expr.target_type->type_info);
    } else {
        LLVM_REPORT_ERROR(data, node, "Cast expression missing type information");
    }

    if (!target_type) {
        LLVM_REPORT_ERROR(data, node, "Failed to determine target type for cast");
    }

    // If types are identical, no cast needed
    if (source_type == target_type) {
        return source_value;
    }

    LLVMTypeKind source_kind = LLVMGetTypeKind(source_type);
    LLVMTypeKind target_kind = LLVMGetTypeKind(target_type);

    // Perform type-specific casting
    switch (source_kind) {
    case LLVMIntegerTypeKind:
        return cast_from_integer(data, node, source_value, source_type, target_type, target_kind);

    case LLVMFloatTypeKind:
    case LLVMDoubleTypeKind:
        return cast_from_float(data, node, source_value, source_type, target_type, target_kind);

    case LLVMPointerTypeKind:
        return cast_from_pointer(data, node, source_value, source_type, target_type, target_kind);

    case LLVMStructTypeKind:
        return cast_from_struct(data, node, source_value, source_type, target_type, target_kind);

    case LLVMArrayTypeKind:
        return cast_from_array(data, node, source_value, source_type, target_type, target_kind);

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported cast from type %d to type %d",
                                 source_kind, target_kind);
    }
}

// Cast from integer types
LLVMValueRef cast_from_integer(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                              LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind) {
    switch (target_kind) {
    case LLVMIntegerTypeKind: {
        unsigned source_width = LLVMGetIntTypeWidth(source_type);
        unsigned target_width = LLVMGetIntTypeWidth(target_type);

        if (source_width == target_width) {
            // Same width, just bitcast
            return source_value;
        } else if (source_width < target_width) {
            // Sign extend (for signed) or zero extend (for unsigned)
            // For now, assume signed extension - could be enhanced with type info
            return LLVMBuildSExt(data->builder, source_value, target_type, "cast_sext");
        } else {
            // Truncate
            return LLVMBuildTrunc(data->builder, source_value, target_type, "cast_trunc");
        }
    }

    case LLVMFloatTypeKind:
    case LLVMDoubleTypeKind:
        // Integer to floating point
        return LLVMBuildSIToFP(data->builder, source_value, target_type, "cast_itof");

    case LLVMPointerTypeKind:
        // Integer to pointer (inttoptr)
        return LLVMBuildIntToPtr(data->builder, source_value, target_type, "cast_itoptr");

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported cast from integer to type %d",
                                 target_kind);
    }
}

// Cast from floating point types
LLVMValueRef cast_from_float(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                            LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind) {
    switch (target_kind) {
    case LLVMIntegerTypeKind:
        // Floating point to integer
        return LLVMBuildFPToSI(data->builder, source_value, target_type, "cast_ftoi");

    case LLVMFloatTypeKind:
    case LLVMDoubleTypeKind:
        // Float to float conversion
        if (LLVMGetTypeKind(source_type) == LLVMFloatTypeKind &&
            target_kind == LLVMDoubleTypeKind) {
            // f32 to f64
            return LLVMBuildFPExt(data->builder, source_value, target_type, "cast_fpext");
        } else if (LLVMGetTypeKind(source_type) == LLVMDoubleTypeKind &&
                   target_kind == LLVMFloatTypeKind) {
            // f64 to f32
            return LLVMBuildFPTrunc(data->builder, source_value, target_type, "cast_fptrunc");
        } else {
            // Same type
            return source_value;
        }

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported cast from float to type %d",
                                 target_kind);
    }
}

// Cast from pointer types
LLVMValueRef cast_from_pointer(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                              LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind) {
    switch (target_kind) {
    case LLVMPointerTypeKind:
        // Pointer to pointer (bitcast)
        return LLVMBuildBitCast(data->builder, source_value, target_type, "cast_ptrcast");

    case LLVMIntegerTypeKind:
        // Pointer to integer (ptrtoint)
        return LLVMBuildPtrToInt(data->builder, source_value, target_type, "cast_ptrtoi");

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported cast from pointer to type %d",
                                 target_kind);
    }
}

// Cast from struct types
LLVMValueRef cast_from_struct(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                             LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind) {
    switch (target_kind) {
    case LLVMStructTypeKind:
        // Struct to struct - only allow if they have the same layout
        // For safety, we'll use bitcast
        return LLVMBuildBitCast(data->builder, source_value, target_type, "cast_struct");

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported cast from struct to type %d",
                                 target_kind);
    }
}

// Cast from array types
LLVMValueRef cast_from_array(LLVMBackendData *data, const ASTNode *node, LLVMValueRef source_value,
                            LLVMTypeRef source_type, LLVMTypeRef target_type, LLVMTypeKind target_kind) {
    switch (target_kind) {
    case LLVMPointerTypeKind:
        // Array to pointer (decay)
        {
            LLVMValueRef indices[2] = {LLVMConstInt(data->i64_type, 0, false),
                                       LLVMConstInt(data->i64_type, 0, false)};
            return LLVMBuildGEP2(data->builder, source_type, source_value, indices, 2,
                                 "cast_array_decay");
        }

    default:
        LLVM_REPORT_ERROR_PRINTF(data, node, "Unsupported cast from array to type %d",
                                 target_kind);
    }
}