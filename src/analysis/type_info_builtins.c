#include "type_info.h"
#include <stdlib.h>

// =============================================================================
// BUILT-IN TYPE INSTANCES
// =============================================================================

// Built-in type instances
TypeInfo *TYPE_INFO_I8 = NULL;
TypeInfo *TYPE_INFO_I16 = NULL;
TypeInfo *TYPE_INFO_I32 = NULL;
TypeInfo *TYPE_INFO_I64 = NULL;
TypeInfo *TYPE_INFO_I128 = NULL;
TypeInfo *TYPE_INFO_U8 = NULL;
TypeInfo *TYPE_INFO_U16 = NULL;
TypeInfo *TYPE_INFO_U32 = NULL;
TypeInfo *TYPE_INFO_U64 = NULL;
TypeInfo *TYPE_INFO_U128 = NULL;
TypeInfo *TYPE_INFO_F32 = NULL;
TypeInfo *TYPE_INFO_F64 = NULL;
TypeInfo *TYPE_INFO_BOOL = NULL;
TypeInfo *TYPE_INFO_CHAR = NULL;
TypeInfo *TYPE_INFO_STRING = NULL;
TypeInfo *TYPE_INFO_VOID = NULL;
TypeInfo *TYPE_INFO_USIZE = NULL;
TypeInfo *TYPE_INFO_ISIZE = NULL;

// =============================================================================
// BUILT-IN TYPE INITIALIZATION
// =============================================================================

bool type_info_init_builtins(void) {
    // Check if already initialized
    if (TYPE_INFO_I32 != NULL) {
        return true; // Already initialized
    }

    // Initialize primitive types with fixed type IDs
    TYPE_INFO_I8 = type_info_create_primitive("i8", PRIMITIVE_INFO_I8, 1);
    TYPE_INFO_I16 = type_info_create_primitive("i16", PRIMITIVE_INFO_I16, 2);
    TYPE_INFO_I32 = type_info_create_primitive("i32", PRIMITIVE_INFO_I32, 4);
    TYPE_INFO_I64 = type_info_create_primitive("i64", PRIMITIVE_INFO_I64, 8);
    TYPE_INFO_I128 = type_info_create_primitive("i128", PRIMITIVE_INFO_I128, 16);
    TYPE_INFO_U8 = type_info_create_primitive("u8", PRIMITIVE_INFO_U8, 1);
    TYPE_INFO_U16 = type_info_create_primitive("u16", PRIMITIVE_INFO_U16, 2);
    TYPE_INFO_U32 = type_info_create_primitive("u32", PRIMITIVE_INFO_U32, 4);
    TYPE_INFO_U64 = type_info_create_primitive("u64", PRIMITIVE_INFO_U64, 8);
    TYPE_INFO_U128 = type_info_create_primitive("u128", PRIMITIVE_INFO_U128, 16);
    TYPE_INFO_F32 = type_info_create_primitive("f32", PRIMITIVE_INFO_F32, 4);
    TYPE_INFO_F64 = type_info_create_primitive("f64", PRIMITIVE_INFO_F64, 8);
    TYPE_INFO_BOOL = type_info_create_primitive("bool", PRIMITIVE_INFO_BOOL, 1);
    TYPE_INFO_CHAR = type_info_create_primitive("char", PRIMITIVE_INFO_CHAR, 4);
    TYPE_INFO_STRING = type_info_create_primitive("string", PRIMITIVE_INFO_STRING, sizeof(void *));
    TYPE_INFO_VOID = type_info_create_primitive("void", PRIMITIVE_INFO_VOID, 0);
    TYPE_INFO_USIZE = type_info_create_primitive("usize", PRIMITIVE_INFO_USIZE, sizeof(size_t));
    TYPE_INFO_ISIZE = type_info_create_primitive("isize", PRIMITIVE_INFO_ISIZE, sizeof(size_t));

    // Check if all types were created successfully
    TypeInfo *types[] = {TYPE_INFO_I8,     TYPE_INFO_I16,  TYPE_INFO_I32,   TYPE_INFO_I64,
                         TYPE_INFO_U8,     TYPE_INFO_U16,  TYPE_INFO_U32,   TYPE_INFO_U64,
                         TYPE_INFO_F32,    TYPE_INFO_F64,  TYPE_INFO_BOOL,  TYPE_INFO_CHAR,
                         TYPE_INFO_STRING, TYPE_INFO_VOID, TYPE_INFO_USIZE, TYPE_INFO_ISIZE};

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        if (!types[i])
            return false;
        if (!type_info_register(types[i]))
            return false;
    }

    return true;
}

void type_info_cleanup_builtins(void) {
    TypeInfo *types[] = {TYPE_INFO_I8,     TYPE_INFO_I16,  TYPE_INFO_I32,   TYPE_INFO_I64,
                         TYPE_INFO_U8,     TYPE_INFO_U16,  TYPE_INFO_U32,   TYPE_INFO_U64,
                         TYPE_INFO_F32,    TYPE_INFO_F64,  TYPE_INFO_BOOL,  TYPE_INFO_CHAR,
                         TYPE_INFO_STRING, TYPE_INFO_VOID, TYPE_INFO_USIZE, TYPE_INFO_ISIZE};

    for (size_t i = 0; i < sizeof(types) / sizeof(types[0]); i++) {
        if (types[i]) {
            type_info_release(types[i]);
        }
    }

    // Clear the global pointers
    TYPE_INFO_I8 = NULL;
    TYPE_INFO_I16 = NULL;
    TYPE_INFO_I32 = NULL;
    TYPE_INFO_I64 = NULL;
    TYPE_INFO_I128 = NULL;
    TYPE_INFO_U8 = NULL;
    TYPE_INFO_U16 = NULL;
    TYPE_INFO_U32 = NULL;
    TYPE_INFO_U64 = NULL;
    TYPE_INFO_U128 = NULL;
    TYPE_INFO_F32 = NULL;
    TYPE_INFO_F64 = NULL;
    TYPE_INFO_BOOL = NULL;
    TYPE_INFO_CHAR = NULL;
    TYPE_INFO_STRING = NULL;
    TYPE_INFO_VOID = NULL;
    TYPE_INFO_USIZE = NULL;
    TYPE_INFO_ISIZE = NULL;

    // Clean up the registry
    type_info_registry_cleanup();
}
