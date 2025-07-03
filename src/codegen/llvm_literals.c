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
#include <string.h>

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
            int_type = LLVMInt8TypeInContext(data->context);
            break;
        case PRIMITIVE_INFO_U8:
            int_type = LLVMInt8TypeInContext(data->context);
            break;
        case PRIMITIVE_INFO_I16:
            int_type = LLVMInt16TypeInContext(data->context);
            break;
        case PRIMITIVE_INFO_U16:
            int_type = LLVMInt16TypeInContext(data->context);
            break;
        case PRIMITIVE_INFO_I32:
            int_type = data->i32_type;
            break;
        case PRIMITIVE_INFO_U32:
            int_type = LLVMInt32TypeInContext(data->context);
            break;
        case PRIMITIVE_INFO_I64:
            int_type = data->i64_type;
            break;
        case PRIMITIVE_INFO_U64:
            int_type = LLVMInt64TypeInContext(data->context);
            break;
        case PRIMITIVE_INFO_USIZE:
            int_type = data->i64_type; // usize is i64 on 64-bit platforms
            break;
        case PRIMITIVE_INFO_ISIZE:
            int_type = data->i64_type; // isize is i64 on 64-bit platforms
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
    extern LLVMValueRef generate_expression(LLVMBackendData * data, const ASTNode *node);

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
    LLVMTypeRef tuple_type =
        LLVMStructTypeInContext(data->context, element_types, (unsigned)element_count, false);

    // Create the tuple value as a struct
    LLVMValueRef tuple_value = LLVMGetUndef(tuple_type);
    for (size_t i = 0; i < element_count; i++) {
        tuple_value =
            LLVMBuildInsertValue(data->builder, tuple_value, element_values[i], (unsigned)i, "");
    }

    free(element_values);
    free(element_types);

    return tuple_value;
}

// Generate code for struct literals
LLVMValueRef generate_struct_literal(LLVMBackendData *data, const ASTNode *node) {
    if (!data || !node) {
        return NULL;
    }

    // Need forward declaration of generate_expression
    extern LLVMValueRef generate_expression(LLVMBackendData * data, const ASTNode *node);

    // Get struct name and field initializations
    const char *struct_name = node->data.struct_literal.struct_name;
    ASTNodeList *field_inits = node->data.struct_literal.field_inits;

    if (!struct_name) {
        LLVM_REPORT_ERROR(data, node, "Struct literal has no struct name");
    }

    // Get the struct type from type info
    if (!node->type_info || node->type_info->category != TYPE_INFO_STRUCT) {
        LLVM_REPORT_ERROR_PRINTF(data, node, "Struct literal for '%s' has invalid type info",
                                 struct_name);
    }

    // Convert struct type info to LLVM type
    LLVMTypeRef struct_type = asthra_type_to_llvm(data, node->type_info);
    if (!struct_type) {
        LLVM_REPORT_ERROR_PRINTF(data, node, "Failed to convert struct type '%s' to LLVM",
                                 struct_name);
    }

    // Create an undefined value of the struct type
    LLVMValueRef struct_value = LLVMGetUndef(struct_type);

    // Handle empty structs (no field initializations)
    if (!field_inits || ast_node_list_size(field_inits) == 0) {
        // For empty structs, return the undefined value
        return struct_value;
    }

    // Process field initializations
    size_t field_init_count = ast_node_list_size(field_inits);
    for (size_t i = 0; i < field_init_count; i++) {
        ASTNode *field_init = ast_node_list_get(field_inits, i);
        if (!field_init || field_init->type != AST_ASSIGNMENT) {
            LLVM_REPORT_ERROR(data, node, "Invalid field initialization in struct literal");
        }

        // Get field name from the assignment target
        ASTNode *field_target = field_init->data.assignment.target;
        if (!field_target || field_target->type != AST_IDENTIFIER) {
            LLVM_REPORT_ERROR(data, field_init,
                              "Field initialization target must be an identifier");
        }

        const char *field_name = field_target->data.identifier.name;
        if (!field_name) {
            LLVM_REPORT_ERROR(data, field_init, "Field initialization has no field name");
        }

        // Find the field index in the struct
        // This requires looking up the field in the struct's type info
        size_t field_index = (size_t)-1;
        if (node->type_info->data.struct_info.fields) {
            for (size_t j = 0; j < node->type_info->data.struct_info.field_count; j++) {
                SymbolEntry *field_entry = node->type_info->data.struct_info.fields[j];
                if (field_entry && field_entry->name &&
                    strcmp(field_entry->name, field_name) == 0) {
                    field_index = j;
                    break;
                }
            }
        }

        if (field_index == (size_t)-1) {
            LLVM_REPORT_ERROR_PRINTF(data, field_init, "Field '%s' not found in struct '%s'",
                                     field_name, struct_name);
        }

        // Generate the field value expression
        ASTNode *field_value_node = field_init->data.assignment.value;
        LLVMValueRef field_value = generate_expression(data, field_value_node);
        if (!field_value) {
            LLVM_REPORT_ERROR_PRINTF(data, field_init, "Failed to generate value for field '%s'",
                                     field_name);
        }

        // Insert the field value into the struct
        struct_value = LLVMBuildInsertValue(data->builder, struct_value, field_value,
                                            (unsigned)field_index, "");
    }

    return struct_value;
}