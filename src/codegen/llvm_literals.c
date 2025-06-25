/**
 * Asthra Programming Language LLVM Literals
 * Implementation of literal value code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_literals.h"
#include "llvm_debug.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>

// Generate code for integer literals
LLVMValueRef generate_integer_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    // Use type information from semantic analysis if available
    LLVMTypeRef int_type = data->i32_type; // default
    if (node->type_info && node->type_info->category == TYPE_INFO_PRIMITIVE) {
        switch (node->type_info->data.primitive.kind) {
        case PRIMITIVE_INFO_I8:
            int_type = LLVMInt8Type();
            break;
        case PRIMITIVE_INFO_I16:
            int_type = LLVMInt16Type();
            break;
        case PRIMITIVE_INFO_I32:
            int_type = data->i32_type;
            break;
        case PRIMITIVE_INFO_I64:
            int_type = data->i64_type;
            break;
        default:
            int_type = data->i32_type;
            break;
        }
    }
    return LLVMConstInt(int_type, node->data.integer_literal.value, false);
}

// Generate code for float literals
LLVMValueRef generate_float_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    return LLVMConstReal(data->f64_type, node->data.float_literal.value);
}

// Generate code for string literals
LLVMValueRef generate_string_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    // Create global string constant
    LLVMValueRef str =
        LLVMBuildGlobalStringPtr(data->builder, node->data.string_literal.value, ".str");
    return str;
}

// Generate code for boolean literals
LLVMValueRef generate_bool_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    return LLVMConstInt(data->bool_type, node->data.bool_literal.value ? 1 : 0, false);
}

// Generate code for character literals
LLVMValueRef generate_char_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    return LLVMConstInt(LLVMInt32TypeInContext(data->context), node->data.char_literal.value,
                        false);
}

// Generate code for unit literals
LLVMValueRef generate_unit_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    // Unit value is an empty struct
    return LLVMConstNamedStruct(data->unit_type, NULL, 0);
}