/**
 * Asthra Programming Language LLVM Type Conversion
 * Implementation of Asthra to LLVM type conversion functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_types.h"
#include "../analysis/type_info_lifecycle.h"
#include <assert.h>
#include <stdlib.h>

// DWARF type encoding constants from DWARF standard
#define DW_ATE_void 0x00
#define DW_ATE_address 0x01
#define DW_ATE_boolean 0x02
#define DW_ATE_complex_float 0x03
#define DW_ATE_float 0x04
#define DW_ATE_signed 0x05
#define DW_ATE_signed_char 0x06
#define DW_ATE_unsigned 0x07
#define DW_ATE_unsigned_char 0x08

// Initialize type cache in LLVMBackendData
void llvm_types_cache_init(LLVMBackendData *data) {
    assert(data && data->context);

    // Cache commonly used LLVM types
    data->i32_type = LLVMInt32TypeInContext(data->context);
    data->i64_type = LLVMInt64TypeInContext(data->context);
    data->f32_type = LLVMFloatTypeInContext(data->context);
    data->f64_type = LLVMDoubleTypeInContext(data->context);
    data->bool_type = LLVMInt1TypeInContext(data->context);
    data->void_type = LLVMVoidTypeInContext(data->context);
    data->ptr_type = LLVMPointerType(LLVMInt8TypeInContext(data->context), 0);

    // Unit type is an empty struct
    data->unit_type = LLVMStructTypeInContext(data->context, NULL, 0, 0);
}

// Convert Asthra type to LLVM type
LLVMTypeRef asthra_type_to_llvm(LLVMBackendData *data, const TypeInfo *type) {
    if (!type)
        return data->void_type;

    switch (type->category) {
    case TYPE_INFO_PRIMITIVE:
        switch (type->data.primitive.kind) {
        case PRIMITIVE_INFO_I32:
            return data->i32_type;
        case PRIMITIVE_INFO_I64:
            return data->i64_type;
        case PRIMITIVE_INFO_U32:
            return LLVMInt32TypeInContext(data->context);
        case PRIMITIVE_INFO_U64:
            return LLVMInt64TypeInContext(data->context);
        case PRIMITIVE_INFO_I8:
            return LLVMInt8TypeInContext(data->context);
        case PRIMITIVE_INFO_U8:
            return LLVMInt8TypeInContext(data->context);
        case PRIMITIVE_INFO_I16:
            return LLVMInt16TypeInContext(data->context);
        case PRIMITIVE_INFO_U16:
            return LLVMInt16TypeInContext(data->context);
        case PRIMITIVE_INFO_I128:
            return LLVMInt128TypeInContext(data->context);
        case PRIMITIVE_INFO_U128:
            return LLVMInt128TypeInContext(data->context);
        case PRIMITIVE_INFO_F32:
            return data->f32_type;
        case PRIMITIVE_INFO_F64:
            return data->f64_type;
        case PRIMITIVE_INFO_BOOL:
            return data->bool_type;
        case PRIMITIVE_INFO_STRING:
            return data->ptr_type; // Strings are char*
        case PRIMITIVE_INFO_VOID:
            return data->void_type;
        case PRIMITIVE_INFO_NEVER:
            return data->void_type; // Never type functions don't return
        default:
            return data->void_type;
        }

    case TYPE_INFO_SLICE:
        // Check if this is actually a fixed-size array by looking at the TypeDescriptor
        if (type->type_descriptor && type->type_descriptor->category == TYPE_ARRAY) {
            // This is a fixed-size array, not a slice
            const TypeDescriptor *desc = type->type_descriptor;
            TypeInfo *elem_type_info = type_info_from_descriptor(desc->data.array.element_type);
            if (elem_type_info) {
                LLVMTypeRef elem_type = asthra_type_to_llvm(data, elem_type_info);
                type_info_release(elem_type_info);
                return LLVMArrayType(elem_type, (unsigned)desc->data.array.size);
            }
        }

        // Regular slice: struct with ptr + length
        {
            LLVMTypeRef elem_type = asthra_type_to_llvm(data, type->data.slice.element_type);
            LLVMTypeRef fields[2] = {
                LLVMPointerType(elem_type, 0), // data pointer
                data->i64_type                 // length
            };
            return LLVMStructTypeInContext(data->context, fields, 2, 0);
        }

    case TYPE_INFO_POINTER: {
        LLVMTypeRef pointee = asthra_type_to_llvm(data, type->data.pointer.pointee_type);
        return LLVMPointerType(pointee, 0);
    }

    case TYPE_INFO_FUNCTION: {
        // Convert parameter types
        size_t param_count = type->data.function.param_count;
        LLVMTypeRef *param_types = NULL;
        if (param_count > 0) {
            param_types = malloc(param_count * sizeof(LLVMTypeRef));
            for (size_t i = 0; i < param_count; i++) {
                param_types[i] = asthra_type_to_llvm(data, type->data.function.param_types[i]);
            }
        }

        LLVMTypeRef ret_type = asthra_type_to_llvm(data, type->data.function.return_type);
        LLVMTypeRef fn_type = LLVMFunctionType(ret_type, param_types, (unsigned)param_count, false);

        if (param_types)
            free(param_types);
        return LLVMPointerType(fn_type, 0);
    }

    case TYPE_INFO_STRUCT: {
        // Create LLVM struct type from field information
        if (type->data.struct_info.field_count == 0) {
            // Empty struct
            return LLVMStructTypeInContext(data->context, NULL, 0,
                                           type->data.struct_info.is_packed);
        }

        // Convert field types
        LLVMTypeRef *field_types = malloc(type->data.struct_info.field_count * sizeof(LLVMTypeRef));
        for (size_t i = 0; i < type->data.struct_info.field_count; i++) {
            SymbolEntry *field = type->data.struct_info.fields[i];
            if (field && field->type) {
                // Convert TypeDescriptor to TypeInfo
                TypeInfo *field_type_info = type_info_from_descriptor(field->type);
                if (field_type_info) {
                    field_types[i] = asthra_type_to_llvm(data, field_type_info);
                    type_info_release(field_type_info);
                } else {
                    field_types[i] = data->i32_type;
                }
            } else {
                // Default to i32 if type info is missing
                field_types[i] = data->i32_type;
            }
        }

        // Create the struct type
        LLVMTypeRef struct_type = LLVMStructTypeInContext(
            data->context, field_types, (unsigned)type->data.struct_info.field_count,
            type->data.struct_info.is_packed);

        free(field_types);
        return struct_type;
    }

    case TYPE_INFO_ENUM:
        // Enums are typically represented as integers
        return data->i32_type;

    case TYPE_INFO_TUPLE: {
        // Tuples are represented as anonymous structs
        if (type->data.tuple.element_count == 0) {
            // Empty tuple is unit type
            return data->unit_type;
        }

        // Convert element types
        LLVMTypeRef *element_types = malloc(type->data.tuple.element_count * sizeof(LLVMTypeRef));
        for (size_t i = 0; i < type->data.tuple.element_count; i++) {
            element_types[i] = asthra_type_to_llvm(data, type->data.tuple.element_types[i]);
        }

        // Create anonymous struct type for tuple
        LLVMTypeRef tuple_type = LLVMStructTypeInContext(data->context, element_types,
                                                         (unsigned)type->data.tuple.element_count,
                                                         0 // Not packed
        );

        free(element_types);
        return tuple_type;
    }

    case TYPE_INFO_OPTION:
        // Option<T> is represented as a struct { bool present; T value; }
        {
            LLVMTypeRef value_type = asthra_type_to_llvm(data, type->data.option.value_type);
            LLVMTypeRef fields[2] = {
                data->bool_type, // present flag
                value_type       // value
            };
            return LLVMStructTypeInContext(data->context, fields, 2, 0);
        }

    default:
        return data->void_type;
    }
}

// Convert Asthra type to debug type
LLVMMetadataRef asthra_type_to_debug_type(LLVMBackendData *data, const TypeInfo *type) {
    if (!type || !data->di_builder)
        return data->di_void_type;

    switch (type->category) {
    case TYPE_INFO_PRIMITIVE:
        switch (type->data.primitive.kind) {
        case PRIMITIVE_INFO_I32:
            return data->di_i32_type;
        case PRIMITIVE_INFO_I64:
            return data->di_i64_type;
        case PRIMITIVE_INFO_U32:
            return LLVMDIBuilderCreateBasicType(data->di_builder, "u32", 3, 32, DW_ATE_unsigned,
                                                LLVMDIFlagZero);
        case PRIMITIVE_INFO_U64:
            return LLVMDIBuilderCreateBasicType(data->di_builder, "u64", 3, 64, DW_ATE_unsigned,
                                                LLVMDIFlagZero);
        case PRIMITIVE_INFO_F32:
            return data->di_f32_type;
        case PRIMITIVE_INFO_F64:
            return data->di_f64_type;
        case PRIMITIVE_INFO_BOOL:
            return data->di_bool_type;
        case PRIMITIVE_INFO_VOID:
            return data->di_void_type;
        case PRIMITIVE_INFO_NEVER:
            return data->di_void_type; // Never type as void in debug info
        case PRIMITIVE_INFO_STRING:
            // String as char* for now
            return LLVMDIBuilderCreatePointerType(
                data->di_builder,
                LLVMDIBuilderCreateBasicType(data->di_builder, "char", 4, 8, DW_ATE_signed_char,
                                             LLVMDIFlagZero),
                64, 0, 0, "", 0);
        default:
            return data->di_void_type;
        }

    case TYPE_INFO_POINTER: {
        LLVMMetadataRef pointee_type =
            asthra_type_to_debug_type(data, type->data.pointer.pointee_type);
        return LLVMDIBuilderCreatePointerType(data->di_builder, pointee_type, 64, 0, 0, "", 0);
    }

    case TYPE_INFO_FUNCTION: {
        // Create function debug type
        size_t param_count = type->data.function.param_count;
        LLVMMetadataRef *param_types = NULL;

        // First element is return type
        size_t type_count = param_count + 1;
        param_types = malloc(type_count * sizeof(LLVMMetadataRef));

        // Return type first
        param_types[0] = type->data.function.return_type
                             ? asthra_type_to_debug_type(data, type->data.function.return_type)
                             : data->di_void_type;

        // Then parameter types
        for (size_t i = 0; i < param_count; i++) {
            param_types[i + 1] =
                asthra_type_to_debug_type(data, type->data.function.param_types[i]);
        }

        LLVMMetadataRef fn_type = LLVMDIBuilderCreateSubroutineType(
            data->di_builder, data->di_file, param_types, (unsigned)type_count, LLVMDIFlagZero);

        free(param_types);
        return fn_type;
    }

    case TYPE_INFO_STRUCT:
        // TODO: Implement struct type debug info
        return data->di_ptr_type;

    case TYPE_INFO_OPTION:
        // TODO: Implement Option<T> debug info as composite type
        return data->di_ptr_type;

    default:
        return data->di_void_type;
    }
}