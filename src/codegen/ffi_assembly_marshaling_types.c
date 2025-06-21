/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Marshaling Type Determination
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_assembly_generator.h"
#include "ffi_assembly_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Helper functions for type checking
static bool is_string_type(const TypeInfo *type_info) {
    return type_info && 
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_STRING;
}

static bool is_slice_type(const TypeInfo *type_info) {
    return type_info && type_info->category == TYPE_INFO_SLICE;
}

static bool is_result_type(const TypeInfo *type_info) {
    // Result type has its own category TYPE_INFO_RESULT, not enum
    return type_info && type_info->category == TYPE_INFO_RESULT;
}

static bool is_integer_type(const TypeInfo *type_info, int bits, bool is_signed) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE) return false;
    
    switch (type_info->data.primitive.kind) {
        case PRIMITIVE_INFO_I8:  return bits == 8 && is_signed;
        case PRIMITIVE_INFO_U8:  return bits == 8 && !is_signed;
        case PRIMITIVE_INFO_I16: return bits == 16 && is_signed;
        case PRIMITIVE_INFO_U16: return bits == 16 && !is_signed;
        case PRIMITIVE_INFO_I32: return bits == 32 && is_signed;
        case PRIMITIVE_INFO_U32: return bits == 32 && !is_signed;
        case PRIMITIVE_INFO_I64: return bits == 64 && is_signed;
        case PRIMITIVE_INFO_U64: return bits == 64 && !is_signed;
        default: return false;
    }
}

static bool is_float_type(const TypeInfo *type_info) {
    return type_info &&
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_F32;
}

static bool is_double_type(const TypeInfo *type_info) {
    return type_info &&
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_F64;
}

static bool is_pointer_type(const TypeInfo *type_info) {
    return type_info && type_info->category == TYPE_INFO_POINTER;
}

// =============================================================================
// MARSHALING TYPE DETERMINATION
// =============================================================================

FFIMarshalingType ffi_determine_marshaling_type(FFIAssemblyGenerator *generator, ASTNode *arg) {
    if (!generator || !arg) return FFI_MARSHAL_DIRECT;
    
    // Check if the argument has type information
    if (!arg->type_info) {
        // Fallback to AST node type analysis
        switch (arg->type) {
            case AST_STRING_LITERAL:
                return FFI_MARSHAL_STRING;
            case AST_ARRAY_LITERAL:
                return FFI_MARSHAL_SLICE;
            case AST_IDENTIFIER:
                // Would need symbol table lookup for precise type
                return FFI_MARSHAL_DIRECT;
            default:
                return FFI_MARSHAL_DIRECT;
        }
    }
    
    // Use type information for precise marshaling
    if (is_string_type(arg->type_info)) {
        return FFI_MARSHAL_STRING;
    } else if (is_slice_type(arg->type_info)) {
        return FFI_MARSHAL_SLICE;
    } else if (is_result_type(arg->type_info)) {
        return FFI_MARSHAL_RESULT;
    } else {
        // All other types (primitives, pointers, unknown) use direct marshaling
        return FFI_MARSHAL_DIRECT;
    }
}

FFIMarshalingType ffi_determine_return_marshaling_type(FFIAssemblyGenerator *generator, ASTNode *call) {
    if (!generator || !call) return FFI_MARSHAL_DIRECT;
    
    // Check if the call has type information for return type
    if (!call->type_info) {
        return FFI_MARSHAL_DIRECT;
    }
    
    // Use type information for precise marshaling
    if (is_string_type(call->type_info)) {
        return FFI_MARSHAL_STRING;
    } else if (is_slice_type(call->type_info)) {
        return FFI_MARSHAL_SLICE;
    } else if (is_result_type(call->type_info)) {
        return FFI_MARSHAL_RESULT;
    } else {
        return FFI_MARSHAL_DIRECT;
    }
} 
