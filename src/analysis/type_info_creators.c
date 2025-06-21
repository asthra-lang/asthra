#include "type_info.h"
#include "semantic_analyzer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// TYPE INFO CREATION HELPERS
// =============================================================================

TypeInfo *type_info_create_primitive(const char *name, int primitive_kind, size_t size) {
    TypeInfo *type_info = type_info_create(name, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_PRIMITIVE;
    type_info->size = size;
    type_info->alignment = size; // Simple alignment rule
    type_info->flags.is_copyable = true;
    type_info->flags.is_movable = true;
    type_info->flags.is_ffi_compatible = true;
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    type_info->data.primitive.kind = (enum PrimitiveInfoKind)primitive_kind;
    
    // Set type-specific properties
    switch (primitive_kind) {
        case PRIMITIVE_INFO_I8:
        case PRIMITIVE_INFO_I16:
        case PRIMITIVE_INFO_I32:
        case PRIMITIVE_INFO_I64:
        case PRIMITIVE_INFO_ISIZE:
            type_info->data.primitive.is_signed = true;
            type_info->data.primitive.is_integer = true;
            break;
        case PRIMITIVE_INFO_U8:
        case PRIMITIVE_INFO_U16:
        case PRIMITIVE_INFO_U32:
        case PRIMITIVE_INFO_U64:
        case PRIMITIVE_INFO_USIZE:
            type_info->data.primitive.is_signed = false;
            type_info->data.primitive.is_integer = true;
            break;
        case PRIMITIVE_INFO_F32:
        case PRIMITIVE_INFO_F64:
            type_info->data.primitive.is_signed = true;
            type_info->data.primitive.is_integer = false;
            break;
        case PRIMITIVE_INFO_BOOL:
        case PRIMITIVE_INFO_CHAR:
            type_info->data.primitive.is_signed = false;
            type_info->data.primitive.is_integer = false;
            break;
        case PRIMITIVE_INFO_STRING:
            type_info->flags.is_ffi_compatible = false; // Needs conversion
            type_info->data.primitive.is_signed = false;
            type_info->data.primitive.is_integer = false;
            break;
        case PRIMITIVE_INFO_VOID:
            type_info->size = 0;
            type_info->flags.is_zero_sized = true;
            type_info->flags.is_copyable = false;
            type_info->flags.is_movable = false;
            break;
    }
    
    return type_info;
}

TypeInfo *type_info_create_slice(TypeInfo *element_type, bool is_mutable) {
    if (!element_type) return NULL;
    
    char name_buffer[256];
    snprintf(name_buffer, (size_t)sizeof(name_buffer), "[]%s", element_type->name);
    
    TypeInfo *type_info = type_info_create(name_buffer, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_SLICE;
    type_info->size = sizeof(void*) + sizeof(size_t); // Pointer + length
    type_info->alignment = sizeof(void*);
    type_info->flags.is_mutable = is_mutable;
    type_info->flags.is_copyable = true;
    type_info->flags.is_movable = true;
    type_info->flags.is_unsized = true;
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    type_info->data.slice.element_type = element_type;
    type_info->data.slice.is_mutable = is_mutable;
    type_info_retain(element_type);
    
    return type_info;
}

TypeInfo *type_info_create_result(TypeInfo *ok_type, TypeInfo *err_type) {
    if (!ok_type || !err_type) return NULL;
    
    char name_buffer[512];
    snprintf(name_buffer, (size_t)sizeof(name_buffer), "Result<%s, %s>", ok_type->name, err_type->name);
    
    TypeInfo *type_info = type_info_create(name_buffer, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_RESULT;
    type_info->size = sizeof(bool) + (ok_type->size > err_type->size ? ok_type->size : err_type->size);
    type_info->alignment = ok_type->alignment > err_type->alignment ? ok_type->alignment : err_type->alignment;
    type_info->flags.is_copyable = ok_type->flags.is_copyable && err_type->flags.is_copyable;
    type_info->flags.is_movable = true;
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    type_info->data.result.ok_type = ok_type;
    type_info->data.result.err_type = err_type;
    type_info_retain(ok_type);
    type_info_retain(err_type);
    
    return type_info;
}

TypeInfo *type_info_create_pointer(TypeInfo *pointee_type, bool is_mutable) {
    if (!pointee_type) return NULL;
    
    char name_buffer[256];
    snprintf(name_buffer, (size_t)sizeof(name_buffer), "*%s%s", 
             is_mutable ? "mut " : "const ", pointee_type->name);
    
    TypeInfo *type_info = type_info_create(name_buffer, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_POINTER;
    type_info->size = sizeof(void*);
    type_info->alignment = sizeof(void*);
    type_info->flags.is_mutable = is_mutable;
    type_info->flags.is_copyable = true;
    type_info->flags.is_movable = true;
    type_info->flags.is_ffi_compatible = true;
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    type_info->data.pointer.pointee_type = pointee_type;
    type_info->data.pointer.is_mutable = is_mutable;
    type_info_retain(pointee_type);
    
    return type_info;
}

TypeInfo *type_info_create_function(TypeInfo **param_types, size_t param_count, 
                                   TypeInfo *return_type) {
    // Build function name
    char name_buffer[1024];
    strcpy(name_buffer, "fn(");
    
    for (size_t i = 0; i < param_count; i++) {
        if (i > 0) strcat(name_buffer, ", ");
        if (param_types[i]) {
            strncat(name_buffer, param_types[i]->name, sizeof(name_buffer) - strlen(name_buffer) - 1);
        } else {
            strcat(name_buffer, "unknown");
        }
    }
    
    strcat(name_buffer, ") -> ");
    if (return_type) {
        strncat(name_buffer, return_type->name, sizeof(name_buffer) - strlen(name_buffer) - 1);
    } else {
        strcat(name_buffer, "void");
    }
    
    TypeInfo *type_info = type_info_create(name_buffer, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_FUNCTION;
    type_info->size = sizeof(void*); // Function pointer size
    type_info->alignment = sizeof(void*);
    type_info->flags.is_copyable = true;
    type_info->flags.is_movable = true;
    type_info->flags.is_ffi_compatible = true;
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    // Copy parameter types
    if (param_count > 0 && param_types) {
        type_info->data.function.param_types = malloc(param_count * sizeof(TypeInfo*));
        if (!type_info->data.function.param_types) {
            type_info_release(type_info);
            return NULL;
        }
        
        for (size_t i = 0; i < param_count; i++) {
            type_info->data.function.param_types[i] = param_types[i];
            if (param_types[i]) {
                type_info_retain(param_types[i]);
            }
        }
    }
    
    type_info->data.function.param_count = param_count;
    type_info->data.function.return_type = return_type;
    
    if (return_type) {
        type_info_retain(return_type);
    }
    
    return type_info;
}

TypeInfo *type_info_create_struct(const char *name, SymbolEntry **fields, size_t field_count) {
    if (!name) return NULL;
    
    TypeInfo *type_info = type_info_create(name, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_STRUCT;
    type_info->flags.is_copyable = true; // Structs are copyable by default
    type_info->flags.is_movable = true;
    type_info->flags.is_ffi_compatible = true; // Depends on fields, but assume true initially
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    type_info->data.struct_info.fields = fields;
    type_info->data.struct_info.field_count = field_count;
    
    if (field_count > 0) {
        type_info->data.struct_info.field_offsets = calloc(field_count, sizeof(size_t));
        if (!type_info->data.struct_info.field_offsets) {
            type_info_release(type_info);
            return NULL;
        }
    }
    
    return type_info;
}

TypeInfo *type_info_create_module(const char *module_name, SymbolEntry **exports, size_t export_count) {
    if (!module_name) return NULL;
    
    char name_buffer[256];
    snprintf(name_buffer, (size_t)sizeof(name_buffer), "module:%s", module_name);
    
    TypeInfo *type_info = type_info_create(name_buffer, 0);
    if (!type_info) return NULL;
    
    type_info->category = (int)TYPE_INFO_MODULE;
    type_info->size = 0; // Modules are zero-sized
    type_info->alignment = 1;
    type_info->flags.is_zero_sized = true;
    type_info->flags.is_copyable = false;
    type_info->flags.is_movable = false;
    
    // Update statistics after category is set
    type_info_update_stats(type_info);
    
    type_info->data.module.module_name = strdup(module_name);
    type_info->data.module.exports = exports;
    type_info->data.module.export_count = export_count;
    
    if (!type_info->data.module.module_name) {
        type_info_release(type_info);
        return NULL;
    }
    
    return type_info;
} 
