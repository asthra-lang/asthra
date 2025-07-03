#include "../../runtime/asthra_runtime.h"
#include "../parser/ast.h"
#include "semantic_analyzer.h"
#include "semantic_symbols_defs.h"
#include "type_info.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

static uint64_t get_timestamp_ns(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

// Helper structure for collecting struct fields
typedef struct {
    SymbolEntry **fields;
    size_t index;
    size_t capacity;
} FieldCollector;

// Callback function for collecting struct fields from symbol table
static bool collect_struct_field(const char *name, SymbolEntry *entry, void *user_data) {
    FieldCollector *collector = (FieldCollector *)user_data;

    if (entry && entry->kind == SYMBOL_FIELD && collector->index < collector->capacity) {
        collector->fields[collector->index++] = entry;
    }
    return true; // Continue iteration
}

// =============================================================================
// TYPE INFO LIFECYCLE MANAGEMENT
// =============================================================================

TypeInfo *type_info_create(const char *name, uint32_t type_id) {
    if (!name)
        return NULL;

    TypeInfo *type_info = calloc(1, sizeof(TypeInfo));
    if (!type_info)
        return NULL;

    type_info->type_id = type_id == 0 ? type_info_allocate_id() : type_id;
    type_info->name = strdup(name);
    type_info->category = (int)TYPE_INFO_UNKNOWN;
    type_info->size = 0;
    type_info->alignment = 1;
    type_info->ownership = OWNERSHIP_INFO_GC;
    type_info->creation_timestamp = get_timestamp_ns();

    atomic_init(&type_info->ref_count, 1);

    if (!type_info->name) {
        free(type_info);
        return NULL;
    }

    return type_info;
}

/**
 * Update statistics after category is set
 */
void type_info_update_stats(TypeInfo *type_info) {
    if (!type_info)
        return;
    type_info_stats_increment((int)type_info->category);
}

void type_info_retain(TypeInfo *type_info) {
    if (!type_info)
        return;
    atomic_fetch_add(&type_info->ref_count, 1);
}

void type_info_release(TypeInfo *type_info) {
    if (!type_info)
        return;

    uint32_t old_count = atomic_fetch_sub(&type_info->ref_count, 1);
    if (old_count == 1) {
        // Last reference, free the type info

        // Update statistics before freeing
        type_info_stats_decrement((int)type_info->category);

        free(type_info->name);

        // Release nested type references
        switch (type_info->category) {
        case TYPE_INFO_SLICE:
            if (type_info->data.slice.element_type) {
                type_info_release(type_info->data.slice.element_type);
            }
            break;
        case TYPE_INFO_POINTER:
            if (type_info->data.pointer.pointee_type) {
                type_info_release(type_info->data.pointer.pointee_type);
            }
            break;
        case TYPE_INFO_RESULT:
            if (type_info->data.result.ok_type) {
                type_info_release(type_info->data.result.ok_type);
            }
            if (type_info->data.result.err_type) {
                type_info_release(type_info->data.result.err_type);
            }
            break;
        case TYPE_INFO_FUNCTION:
            if (type_info->data.function.param_types) {
                for (size_t i = 0; i < type_info->data.function.param_count; i++) {
                    if (type_info->data.function.param_types[i]) {
                        type_info_release(type_info->data.function.param_types[i]);
                    }
                }
                free(type_info->data.function.param_types);
            }
            if (type_info->data.function.return_type) {
                type_info_release(type_info->data.function.return_type);
            }
            break;
        case TYPE_INFO_STRUCT:
            if (type_info->data.struct_info.field_offsets) {
                free(type_info->data.struct_info.field_offsets);
            }
            break;
        case TYPE_INFO_MODULE:
            if (type_info->data.module.module_name) {
                free(type_info->data.module.module_name);
            }
            break;
        case TYPE_INFO_TASK_HANDLE:
            if (type_info->data.task_handle.result_type) {
                type_info_release(type_info->data.task_handle.result_type);
            }
            break;
        default:
            break;
        }

        free(type_info);
    }
}

// =============================================================================
// TYPE INFO STATISTICS FUNCTIONS
// =============================================================================

// Note: Statistics functions are implemented in type_info_diagnostics.c to avoid
// duplicate symbols. The debug module provides the complete implementation
// with atomic operations and proper statistics tracking.

// =============================================================================
// TYPE DESCRIPTOR CONVERSION
// =============================================================================

TypeInfo *type_info_from_descriptor(TypeDescriptor *descriptor) {
    if (!descriptor)
        return NULL;

    // Debug output - removed to clean up output

    // Create a basic TypeInfo from the TypeDescriptor
    TypeInfo *type_info = type_info_create(descriptor->name ? descriptor->name : "unknown", 0);
    if (!type_info)
        return NULL;

    // Map TypeDescriptor category to TypeInfo category
    switch (descriptor->category) {
    case TYPE_PRIMITIVE:
    case TYPE_BUILTIN:
    case TYPE_INTEGER:
    case TYPE_FLOAT:
    case TYPE_BOOL:
        type_info->category = TYPE_INFO_PRIMITIVE;
        // Set primitive kind based on the actual primitive_kind in the TypeDescriptor
        // instead of using string comparison which can miss types like usize
        switch (descriptor->data.primitive.primitive_kind) {
        case PRIMITIVE_VOID:
            type_info->data.primitive.kind = PRIMITIVE_INFO_VOID;
            break;
        case PRIMITIVE_BOOL:
            type_info->data.primitive.kind = PRIMITIVE_INFO_BOOL;
            break;
        case PRIMITIVE_I8:
            type_info->data.primitive.kind = PRIMITIVE_INFO_I8;
            break;
        case PRIMITIVE_I16:
            type_info->data.primitive.kind = PRIMITIVE_INFO_I16;
            break;
        case PRIMITIVE_I32:
            type_info->data.primitive.kind = PRIMITIVE_INFO_I32;
            break;
        case PRIMITIVE_I64:
            type_info->data.primitive.kind = PRIMITIVE_INFO_I64;
            break;
        case PRIMITIVE_I128:
            type_info->data.primitive.kind = PRIMITIVE_INFO_I128;
            break;
        case PRIMITIVE_ISIZE:
            type_info->data.primitive.kind = PRIMITIVE_INFO_ISIZE;
            break;
        case PRIMITIVE_U8:
            type_info->data.primitive.kind = PRIMITIVE_INFO_U8;
            break;
        case PRIMITIVE_U16:
            type_info->data.primitive.kind = PRIMITIVE_INFO_U16;
            break;
        case PRIMITIVE_U32:
            type_info->data.primitive.kind = PRIMITIVE_INFO_U32;
            break;
        case PRIMITIVE_U64:
            type_info->data.primitive.kind = PRIMITIVE_INFO_U64;
            break;
        case PRIMITIVE_U128:
            type_info->data.primitive.kind = PRIMITIVE_INFO_U128;
            break;
        case PRIMITIVE_USIZE:
            type_info->data.primitive.kind = PRIMITIVE_INFO_USIZE;
            break;
        case PRIMITIVE_F32:
            type_info->data.primitive.kind = PRIMITIVE_INFO_F32;
            break;
        case PRIMITIVE_F64:
            type_info->data.primitive.kind = PRIMITIVE_INFO_F64;
            break;
        case PRIMITIVE_CHAR:
            type_info->data.primitive.kind = PRIMITIVE_INFO_CHAR;
            break;
        case PRIMITIVE_STRING:
            type_info->data.primitive.kind = PRIMITIVE_INFO_STRING;
            break;
        case PRIMITIVE_NEVER:
            type_info->data.primitive.kind = PRIMITIVE_INFO_NEVER;
            break;
        default:
            // Fallback to VOID for unknown types
            type_info->data.primitive.kind = PRIMITIVE_INFO_VOID;
            break;
        }
        break;
    case TYPE_STRUCT:
        type_info->category = TYPE_INFO_STRUCT;
        type_info->data.struct_info.field_count = descriptor->data.struct_type.field_count;
        type_info->data.struct_info.is_packed = false; // Default to non-packed

        // Extract fields from the symbol table
        if (descriptor->data.struct_type.field_count > 0 && descriptor->data.struct_type.fields) {
            // Allocate array for field pointers
            type_info->data.struct_info.fields =
                calloc(descriptor->data.struct_type.field_count, sizeof(SymbolEntry *));

            if (type_info->data.struct_info.fields) {
                // Set up collector for fields
                FieldCollector collector = {.fields = type_info->data.struct_info.fields,
                                            .index = 0,
                                            .capacity = descriptor->data.struct_type.field_count};

                // Iterate through the symbol table to collect fields
                symbol_table_iterate(descriptor->data.struct_type.fields, collect_struct_field,
                                     &collector);
            }
        } else {
            type_info->data.struct_info.fields = NULL;
        }
        break;
    case TYPE_ENUM:
        type_info->category = TYPE_INFO_ENUM;
        break;
    case TYPE_SLICE:
        type_info->category = TYPE_INFO_SLICE;
        // Create TypeInfo for element type
        if (descriptor->data.slice.element_type) {
            type_info->data.slice.element_type =
                type_info_from_descriptor(descriptor->data.slice.element_type);
        }
        break;
    case TYPE_ARRAY:
        // Fixed-size arrays are treated as slices with known size
        type_info->category = TYPE_INFO_SLICE;
        // Store the array size in the size field (element count)
        type_info->size = descriptor->data.array.size;
        break;
    case TYPE_POINTER:
        type_info->category = TYPE_INFO_POINTER;
        break;
    case TYPE_RESULT:
        type_info->category = TYPE_INFO_RESULT;
        break;
    case TYPE_FUNCTION:
        type_info->category = TYPE_INFO_FUNCTION;
        // Populate function-specific data
        if (descriptor->data.function.return_type) {
            type_info->data.function.return_type =
                type_info_from_descriptor(descriptor->data.function.return_type);
        }
        type_info->data.function.param_count = descriptor->data.function.param_count;
        if (descriptor->data.function.param_count > 0 && descriptor->data.function.param_types) {
            type_info->data.function.param_types =
                malloc(descriptor->data.function.param_count * sizeof(TypeInfo *));
            for (size_t i = 0; i < descriptor->data.function.param_count; i++) {
                type_info->data.function.param_types[i] =
                    type_info_from_descriptor(descriptor->data.function.param_types[i]);
            }
        }
        break;
    case TYPE_GENERIC_INSTANCE:
        // Generic instances should map to their base type category
        if (descriptor->data.generic_instance.base_type &&
            descriptor->data.generic_instance.base_type->category == TYPE_STRUCT) {
            type_info->category = TYPE_INFO_STRUCT;

            // Extract fields from the base struct type
            TypeDescriptor *base_struct = descriptor->data.generic_instance.base_type;
            type_info->data.struct_info.field_count = base_struct->data.struct_type.field_count;
            type_info->data.struct_info.is_packed = false;

            if (base_struct->data.struct_type.field_count > 0 &&
                base_struct->data.struct_type.fields) {
                // Allocate array for field pointers
                type_info->data.struct_info.fields =
                    calloc(base_struct->data.struct_type.field_count, sizeof(SymbolEntry *));

                if (type_info->data.struct_info.fields) {
                    // Set up collector for fields
                    FieldCollector collector = {.fields = type_info->data.struct_info.fields,
                                                .index = 0,
                                                .capacity =
                                                    base_struct->data.struct_type.field_count};

                    // Iterate through the symbol table to collect fields
                    symbol_table_iterate(base_struct->data.struct_type.fields, collect_struct_field,
                                         &collector);
                }
            } else {
                type_info->data.struct_info.fields = NULL;
            }
        } else {
            type_info->category = TYPE_INFO_STRUCT; // Default fallback
        }
        break;
    case TYPE_TUPLE:
        type_info->category = TYPE_INFO_TUPLE;
        // Copy tuple-specific data
        type_info->data.tuple.element_count = descriptor->data.tuple.element_count;
        if (descriptor->data.tuple.element_count > 0 && descriptor->data.tuple.element_types) {
            type_info->data.tuple.element_types =
                malloc(descriptor->data.tuple.element_count * sizeof(TypeInfo *));
            if (type_info->data.tuple.element_types) {
                for (size_t i = 0; i < descriptor->data.tuple.element_count; i++) {
                    type_info->data.tuple.element_types[i] =
                        type_info_from_descriptor(descriptor->data.tuple.element_types[i]);
                }
            }
        }
        break;
    case TYPE_TASK_HANDLE:
        type_info->category = TYPE_INFO_TASK_HANDLE;
        // Copy task handle-specific data
        if (descriptor->data.task_handle.result_type) {
            type_info->data.task_handle.result_type =
                type_info_from_descriptor(descriptor->data.task_handle.result_type);
        }
        break;
    default:
        type_info->category = TYPE_INFO_UNKNOWN;
        break;
    }

    // Set basic properties
    type_info->size = descriptor->size;
    type_info->alignment = descriptor->alignment;

    // Set the type descriptor link
    type_info->type_descriptor = descriptor;

    // Update statistics
    type_info_update_stats(type_info);

    return type_info;
}
