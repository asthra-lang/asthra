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

    // Use type information from semantic analysis if available
    LLVMTypeRef float_type = data->f64_type; // default
    if (node->type_info && node->type_info->category == TYPE_INFO_PRIMITIVE) {
        switch (node->type_info->data.primitive.kind) {
        case PRIMITIVE_INFO_F32:
            float_type = data->f32_type;
            break;
        case PRIMITIVE_INFO_F64:
            float_type = data->f64_type;
            break;
        default:
            float_type = data->f64_type;
            break;
        }
    }
    return LLVMConstReal(float_type, node->data.float_literal.value);
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

// Generate code for tuple literals
LLVMValueRef generate_tuple_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    // Need forward declaration of generate_expression
    extern LLVMValueRef generate_expression(LLVMBackendData *data, const ASTNode *node);

    // Get the elements from the tuple literal
    ASTNodeList *elements = node->data.tuple_literal.elements;
    if (!elements) {
        LLVM_REPORT_ERROR(data, node, "Tuple literal has no elements");
    }

    size_t element_count = ast_node_list_size(elements);
    if (element_count < 2) {
        LLVM_REPORT_ERROR(data, node, "Tuple literals must have at least 2 elements");
    }

    // Generate values for each element
    LLVMValueRef *element_values = malloc(element_count * sizeof(LLVMValueRef));
    LLVMTypeRef *element_types = malloc(element_count * sizeof(LLVMTypeRef));
    
    if (!element_values || !element_types) {
        free(element_values);
        free(element_types);
        LLVM_REPORT_ERROR(data, node, "Failed to allocate memory for tuple elements");
    }

    // Generate each element
    for (size_t i = 0; i < element_count; i++) {
        ASTNode *element = ast_node_list_get(elements, i);
        element_values[i] = generate_expression(data, element);
        if (!element_values[i]) {
            free(element_values);
            free(element_types);
            LLVM_REPORT_ERROR(data, node, "Failed to generate tuple element");
        }
        element_types[i] = LLVMTypeOf(element_values[i]);
    }

    // Create anonymous struct type for the tuple
    LLVMTypeRef tuple_type = LLVMStructTypeInContext(data->context, element_types, 
                                                      (unsigned)element_count, false);

    // Create the tuple value as a struct
    LLVMValueRef tuple_value = LLVMGetUndef(tuple_type);
    for (size_t i = 0; i < element_count; i++) {
        tuple_value = LLVMBuildInsertValue(data->builder, tuple_value, 
                                           element_values[i], (unsigned)i, "");
    }

    free(element_values);
    free(element_types);

    return tuple_value;
}