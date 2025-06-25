/**
 * Asthra Programming Language LLVM Access Expressions
 * Implementation of field access, indexing, and array literal code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_access_expr.h"
#include "llvm_debug.h"
#include "llvm_expr_gen.h"
#include "llvm_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generate code for array/slice indexing
LLVMValueRef generate_index_expr(LLVMBackendData *data, const ASTNode *node) {
    LLVMValueRef array = generate_expression(data, node->data.index_access.array);
    LLVMValueRef index = generate_expression(data, node->data.index_access.index);

    if (!array || !index)
        return NULL;

    // Get element pointer
    LLVMValueRef indices[2] = {
        LLVMConstInt(data->i64_type, 0, false), // First index for array decay
        index                                   // Actual index
    };

    // Get array type for GEP
    LLVMTypeRef array_type = LLVMTypeOf(array);
    LLVMValueRef element_ptr =
        LLVMBuildGEP2(data->builder, array_type, array, indices, 2, "elemptr");

    // Load the value - need to get element type from array
    // For LLVM 15+, we can't use LLVMGetElementType on opaque pointers
    // We need to get type information from the AST node
    LLVMTypeRef elem_type = data->i32_type; // Default fallback

    // Try to get type from node's type info
    if (node->type_info && node->type_info->category == TYPE_INFO_SLICE) {
        elem_type = asthra_type_to_llvm(data, node->type_info->data.slice.element_type);
    }

    return LLVMBuildLoad2(data->builder, elem_type, element_ptr, "elem");
}

// Generate code for field access
LLVMValueRef generate_field_access(LLVMBackendData *data, const ASTNode *node) {
    // Special handling for type names like Option.None
    if (node->data.field_access.object->type == AST_IDENTIFIER) {
        const char *type_name = node->data.field_access.object->data.identifier.name;
        const char *field_name = node->data.field_access.field_name;

        // Handle Option.None (when not used as a function call)
        if (strcmp(type_name, "Option") == 0 && strcmp(field_name, "None") == 0) {
            // Create an Option struct with present = false
            // Get the expected type from node->type_info if available
            LLVMTypeRef value_type = data->i32_type; // Default

            if (node->type_info && node->type_info->category == TYPE_INFO_OPTION) {
                value_type = asthra_type_to_llvm(data, node->type_info->data.option.value_type);
            }

            LLVMTypeRef fields[2] = {
                data->bool_type, // present flag
                value_type       // value
            };
            LLVMTypeRef option_type = LLVMStructTypeInContext(data->context, fields, 2, 0);

            // Create a constant None value
            LLVMValueRef values[2] = {
                LLVMConstInt(data->bool_type, 0, false), // present = false
                LLVMConstNull(value_type)                // null value
            };

            return LLVMConstNamedStruct(option_type, values, 2);
        }
    }

    LLVMValueRef object = generate_expression(data, node->data.field_access.object);
    if (!object)
        return NULL;

    const char *field_name = node->data.field_access.field_name;

    // Get struct type info from the object's AST node
    TypeInfo *struct_type_info = NULL;
    if (node->data.field_access.object->type_info) {
        struct_type_info = node->data.field_access.object->type_info;
    }

    // Find field index and type
    uint32_t field_index = 0;
    LLVMTypeRef field_type = data->i32_type; // Default

    if (struct_type_info && struct_type_info->category == TYPE_INFO_STRUCT) {
        // Search for field by name
        for (size_t i = 0; i < struct_type_info->data.struct_info.field_count; i++) {
            SymbolEntry *field = struct_type_info->data.struct_info.fields[i];
            if (field && field->name && strcmp(field->name, field_name) == 0) {
                field_index = (uint32_t)i;
                // Get field type
                if (field->type) {
                    // Convert TypeDescriptor to TypeInfo
                    TypeInfo *field_type_info = type_info_from_descriptor(field->type);
                    if (field_type_info) {
                        field_type = asthra_type_to_llvm(data, field_type_info);
                        type_info_release(field_type_info);
                    }
                }
                break;
            }
        }
    }

    // Build GEP indices
    LLVMValueRef indices[2] = {LLVMConstInt(data->i32_type, 0, false),
                               LLVMConstInt(data->i32_type, field_index, false)};

    LLVMTypeRef object_type = LLVMTypeOf(object);
    LLVMValueRef field_ptr =
        LLVMBuildGEP2(data->builder, object_type, object, indices, 2, field_name);
    return LLVMBuildLoad2(data->builder, field_type, field_ptr, field_name);
}

// Generate code for array literals
LLVMValueRef generate_array_literal(LLVMBackendData *data, const ASTNode *node) {
    size_t elem_count =
        node->data.array_literal.elements ? node->data.array_literal.elements->count : 0;

    if (elem_count == 0) {
        // Empty array
        return LLVMConstArray(data->i32_type, NULL, 0);
    }

    // Generate element values
    LLVMValueRef *elements = malloc(elem_count * sizeof(LLVMValueRef));
    LLVMTypeRef elem_type = NULL;

    for (size_t i = 0; i < elem_count; i++) {
        elements[i] = generate_expression(data, node->data.array_literal.elements->nodes[i]);
        if (!elements[i]) {
            free(elements);
            return NULL;
        }

        // Get element type from first element
        if (i == 0) {
            elem_type = LLVMTypeOf(elements[i]);
        }
    }

    // Create array constant
    LLVMValueRef array = LLVMConstArray(elem_type, elements, (unsigned)elem_count);
    free(elements);

    return array;
}