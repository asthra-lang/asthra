#include "type_info.h"
#include "semantic_analyzer.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// HELPER FUNCTIONS FOR TYPE MAPPING
// =============================================================================

// Map TypeDescriptor primitive kinds to TypeInfo primitive kinds
static int map_descriptor_to_info_primitive(int descriptor_kind) {
    switch (descriptor_kind) {
        case PRIMITIVE_VOID: return PRIMITIVE_INFO_VOID;
        case PRIMITIVE_BOOL: return PRIMITIVE_INFO_BOOL;
        case PRIMITIVE_I8: return PRIMITIVE_INFO_I8;
        case PRIMITIVE_I16: return PRIMITIVE_INFO_I16;
        case PRIMITIVE_I32: return PRIMITIVE_INFO_I32;
        case PRIMITIVE_I64: return PRIMITIVE_INFO_I64;
        case PRIMITIVE_U8: return PRIMITIVE_INFO_U8;
        case PRIMITIVE_U16: return PRIMITIVE_INFO_U16;
        case PRIMITIVE_U32: return PRIMITIVE_INFO_U32;
        case PRIMITIVE_U64: return PRIMITIVE_INFO_U64;
        case PRIMITIVE_F32: return PRIMITIVE_INFO_F32;
        case PRIMITIVE_F64: return PRIMITIVE_INFO_F64;
        case PRIMITIVE_CHAR: return PRIMITIVE_INFO_CHAR;
        case PRIMITIVE_STRING: return PRIMITIVE_INFO_STRING;
        default: return PRIMITIVE_INFO_VOID; // fallback
    }
}

// Map TypeInfo primitive kinds to TypeDescriptor primitive kinds
static int map_info_to_descriptor_primitive(enum PrimitiveInfoKind info_kind) {
    switch (info_kind) {
        case PRIMITIVE_INFO_VOID: return PRIMITIVE_VOID;
        case PRIMITIVE_INFO_BOOL: return PRIMITIVE_BOOL;
        case PRIMITIVE_INFO_I8: return PRIMITIVE_I8;
        case PRIMITIVE_INFO_I16: return PRIMITIVE_I16;
        case PRIMITIVE_INFO_I32: return PRIMITIVE_I32;
        case PRIMITIVE_INFO_I64: return PRIMITIVE_I64;
        case PRIMITIVE_INFO_U8: return PRIMITIVE_U8;
        case PRIMITIVE_INFO_U16: return PRIMITIVE_U16;
        case PRIMITIVE_INFO_U32: return PRIMITIVE_U32;
        case PRIMITIVE_INFO_U64: return PRIMITIVE_U64;
        case PRIMITIVE_INFO_F32: return PRIMITIVE_F32;
        case PRIMITIVE_INFO_F64: return PRIMITIVE_F64;
        case PRIMITIVE_INFO_CHAR: return PRIMITIVE_CHAR;
        case PRIMITIVE_INFO_STRING: return PRIMITIVE_STRING;
        case PRIMITIVE_INFO_USIZE: return PRIMITIVE_U32; // Map to u32 for now
        case PRIMITIVE_INFO_ISIZE: return PRIMITIVE_I32; // Map to i32 for now
        default: return PRIMITIVE_VOID; // fallback
    }
}

// =============================================================================
// SEMANTIC ANALYSIS INTEGRATION
// =============================================================================

TypeInfo *type_info_from_type_descriptor(const TypeDescriptor *descriptor) {
    if (!descriptor) return NULL;
    
    TypeInfo *type_info = NULL;
    
    switch (descriptor->category) {
        case TYPE_PRIMITIVE: {
            const char *name = descriptor->name ? descriptor->name : "unknown";
            int mapped_kind = map_descriptor_to_info_primitive(descriptor->data.primitive.primitive_kind);
            type_info = type_info_create_primitive(name, mapped_kind, descriptor->size);
            break;
        }
        
        case TYPE_SLICE: {
            TypeInfo *element_type_info = type_info_from_type_descriptor(descriptor->data.slice.element_type);
            if (element_type_info) {
                type_info = type_info_create_slice(element_type_info, descriptor->flags.is_mutable);
                type_info_release(element_type_info); // create_slice retains it
            }
            break;
        }
        
        case TYPE_RESULT: {
            TypeInfo *ok_type_info = type_info_from_type_descriptor(descriptor->data.result.ok_type);
            TypeInfo *err_type_info = type_info_from_type_descriptor(descriptor->data.result.err_type);
            if (ok_type_info && err_type_info) {
                type_info = type_info_create_result(ok_type_info, err_type_info);
                type_info_release(ok_type_info);   // create_result retains it
                type_info_release(err_type_info);  // create_result retains it
            } else {
                if (ok_type_info) type_info_release(ok_type_info);
                if (err_type_info) type_info_release(err_type_info);
            }
            break;
        }
        
        case TYPE_STRUCT: {
            const char *name = descriptor->name ? descriptor->name : "unknown_struct";
            type_info = type_info_create(name, 0);
            if (type_info) {
                type_info->category = (int)TYPE_INFO_STRUCT;
                type_info->size = descriptor->size;
                type_info->alignment = descriptor->alignment;
                type_info->flags.is_mutable = descriptor->flags.is_mutable;
                type_info->flags.is_ffi_compatible = descriptor->flags.is_ffi_compatible;
                
                // Copy struct field information if available
                if (descriptor->data.struct_type.fields) {
                    type_info->data.struct_info.field_count = descriptor->data.struct_type.field_count;
                    // Note: We don't copy the fields here as they're managed by the symbol table
                    // The fields pointer will be set up during struct analysis
                }
            }
            break;
        }
        
        case TYPE_POINTER: {
            TypeInfo *pointee_type_info = type_info_from_type_descriptor(descriptor->data.pointer.pointee_type);
            if (pointee_type_info) {
                type_info = type_info_create_pointer(pointee_type_info, descriptor->flags.is_mutable);
                type_info_release(pointee_type_info); // create_pointer retains it
            }
            break;
        }
        
        case TYPE_FUNCTION: {
            // Convert parameter types
            TypeInfo **param_type_infos = NULL;
            if (descriptor->data.function.param_count > 0) {
                param_type_infos = malloc(descriptor->data.function.param_count * sizeof(TypeInfo*));
                if (!param_type_infos) return NULL;
                
                for (size_t i = 0; i < descriptor->data.function.param_count; i++) {
                    param_type_infos[i] = type_info_from_type_descriptor(descriptor->data.function.param_types[i]);
                    if (!param_type_infos[i]) {
                        // Cleanup on failure
                        for (size_t j = 0; j < i; j++) {
                            type_info_release(param_type_infos[j]);
                        }
                        free(param_type_infos);
                        return NULL;
                    }
                }
            }
            
            // Convert return type
            TypeInfo *return_type_info = type_info_from_type_descriptor(descriptor->data.function.return_type);
            if (!return_type_info && descriptor->data.function.return_type) {
                // Cleanup on failure
                if (param_type_infos) {
                    for (size_t i = 0; i < descriptor->data.function.param_count; i++) {
                        type_info_release(param_type_infos[i]);
                    }
                    free(param_type_infos);
                }
                return NULL;
            }
            
            type_info = type_info_create_function(param_type_infos, descriptor->data.function.param_count,
                                                 return_type_info);
            
            // Cleanup temporary arrays (create_function makes its own copies)
            if (param_type_infos) {
                for (size_t i = 0; i < descriptor->data.function.param_count; i++) {
                    type_info_release(param_type_infos[i]);
                }
                free(param_type_infos);
            }
            if (return_type_info) {
                type_info_release(return_type_info);
            }
            break;
        }
        
        default:
            // For other types, create a basic descriptor
            type_info = type_info_create(descriptor->name ? descriptor->name : "unknown", 0);
            if (type_info) {
                type_info->category = (int)TYPE_INFO_UNKNOWN;
                type_info->size = descriptor->size;
                type_info->alignment = descriptor->alignment;
                
                // Check if strdup failed
                if (!type_info->name) {
                    free(type_info);
                    type_info = NULL;
                }
            }
            break;
    }
    
    if (type_info) {
        // Copy common flags
        type_info->flags.is_mutable = descriptor->flags.is_mutable;
        type_info->flags.is_owned = descriptor->flags.is_owned;
        type_info->flags.is_borrowed = descriptor->flags.is_borrowed;
        type_info->flags.is_constant = descriptor->flags.is_constant;
        type_info->flags.is_volatile = descriptor->flags.is_volatile;
        type_info->flags.is_atomic = descriptor->flags.is_atomic;
        type_info->flags.is_ffi_compatible = descriptor->flags.is_ffi_compatible;
        
        // Link back to the TypeDescriptor
        type_info->type_descriptor = descriptor;
    }
    
    return type_info;
}

TypeDescriptor *type_descriptor_from_type_info(const TypeInfo *type_info) {
    if (!type_info) return NULL;
    
    // If we already have a linked TypeDescriptor, return it
    if (type_info->type_descriptor) {
        type_descriptor_retain((TypeDescriptor *)type_info->type_descriptor);
        return (TypeDescriptor *)type_info->type_descriptor;
    }
    
    // Create a new TypeDescriptor based on TypeInfo
    TypeDescriptor *descriptor = NULL;
    
    switch (type_info->category) {
        case TYPE_INFO_PRIMITIVE:
            {
                int mapped_kind = map_info_to_descriptor_primitive(type_info->data.primitive.kind);
                descriptor = type_descriptor_create_primitive(mapped_kind);
            }
            break;
            
        case TYPE_INFO_SLICE:
            if (type_info->data.slice.element_type) {
                TypeDescriptor *element_descriptor = type_descriptor_from_type_info(type_info->data.slice.element_type);
                if (element_descriptor) {
                    descriptor = type_descriptor_create_slice(element_descriptor);
                    type_descriptor_release(element_descriptor); // create_slice retains it
                }
            }
            break;
            
        case TYPE_INFO_RESULT:
            if (type_info->data.result.ok_type && type_info->data.result.err_type) {
                TypeDescriptor *ok_descriptor = type_descriptor_from_type_info(type_info->data.result.ok_type);
                TypeDescriptor *err_descriptor = type_descriptor_from_type_info(type_info->data.result.err_type);
                if (ok_descriptor && err_descriptor) {
                    descriptor = type_descriptor_create_result(ok_descriptor, err_descriptor);
                    type_descriptor_release(ok_descriptor);   // create_result retains it
                    type_descriptor_release(err_descriptor);  // create_result retains it
                } else {
                    if (ok_descriptor) type_descriptor_release(ok_descriptor);
                    if (err_descriptor) type_descriptor_release(err_descriptor);
                }
            }
            break;
            
        case TYPE_INFO_POINTER:
            if (type_info->data.pointer.pointee_type) {
                TypeDescriptor *pointee_descriptor = type_descriptor_from_type_info(type_info->data.pointer.pointee_type);
                if (pointee_descriptor) {
                    descriptor = type_descriptor_create_pointer(pointee_descriptor);
                    type_descriptor_release(pointee_descriptor); // create_pointer retains it
                }
            }
            break;
            
        case TYPE_INFO_STRUCT:
            descriptor = type_descriptor_create_struct(type_info->name, type_info->data.struct_info.field_count);
            break;
            
        default:
            // For other types, create a basic descriptor
            descriptor = malloc(sizeof(TypeDescriptor));
            if (descriptor) {
                memset(descriptor, 0, sizeof(TypeDescriptor));
                descriptor->category = (int)TYPE_UNKNOWN;
                descriptor->size = type_info->size;
                descriptor->alignment = type_info->alignment;
                descriptor->name = strdup(type_info->name);
                atomic_init(&descriptor->ref_count, 1);
                
                // Check if strdup failed
                if (!descriptor->name) {
                    free(descriptor);
                    descriptor = NULL;
                }
            }
            break;
    }
    
    if (descriptor) {
        // Copy common properties
        descriptor->flags.is_mutable = type_info->flags.is_mutable;
        descriptor->flags.is_owned = type_info->flags.is_owned;
        descriptor->flags.is_borrowed = type_info->flags.is_borrowed;
        descriptor->flags.is_constant = type_info->flags.is_constant;
        descriptor->flags.is_volatile = type_info->flags.is_volatile;
        descriptor->flags.is_atomic = type_info->flags.is_atomic;
        descriptor->flags.is_ffi_compatible = type_info->flags.is_ffi_compatible;
    }
    
    return descriptor;
}

bool type_info_sync_with_descriptor(TypeInfo *type_info, TypeDescriptor *descriptor) {
    if (!type_info || !descriptor) return false;
    
    // Update TypeInfo from TypeDescriptor
    type_info->size = descriptor->size;
    type_info->alignment = descriptor->alignment;
    type_info->flags.is_mutable = descriptor->flags.is_mutable;
    type_info->flags.is_owned = descriptor->flags.is_owned;
    type_info->flags.is_borrowed = descriptor->flags.is_borrowed;
    type_info->flags.is_constant = descriptor->flags.is_constant;
    type_info->flags.is_volatile = descriptor->flags.is_volatile;
    type_info->flags.is_atomic = descriptor->flags.is_atomic;
    type_info->flags.is_ffi_compatible = descriptor->flags.is_ffi_compatible;
    
    // Link them together
    type_info->type_descriptor = descriptor;
    type_descriptor_retain(descriptor);
    
    return true;
} 
