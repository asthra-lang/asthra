#include "type_info.h"
#include <string.h>

// =============================================================================
// TYPE INFO QUERIES AND OPERATIONS
// =============================================================================

bool type_info_equals(const TypeInfo *a, const TypeInfo *b) {
    if (a == b)
        return true;
    if (!a || !b)
        return false;

    if (a->category != b->category)
        return false;

    switch (a->category) {
    case TYPE_INFO_PRIMITIVE:
        return a->data.primitive.kind == b->data.primitive.kind;
    case TYPE_INFO_SLICE:
        return type_info_equals(a->data.slice.element_type, b->data.slice.element_type) &&
               a->data.slice.is_mutable == b->data.slice.is_mutable;
    case TYPE_INFO_RESULT:
        return type_info_equals(a->data.result.ok_type, b->data.result.ok_type) &&
               type_info_equals(a->data.result.err_type, b->data.result.err_type);
    case TYPE_INFO_STRUCT:
        return strcmp(a->name, b->name) == 0; // Struct types are nominal
    default:
        return false;
    }
}

bool type_info_is_numeric(const TypeInfo *type_info) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE)
        return false;

    enum PrimitiveInfoKind kind = type_info->data.primitive.kind;
    return (kind >= PRIMITIVE_INFO_I8 && kind <= PRIMITIVE_INFO_F64);
}

bool type_info_is_integer(const TypeInfo *type_info) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE)
        return false;
    return type_info->data.primitive.is_integer;
}

bool type_info_is_float(const TypeInfo *type_info) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE)
        return false;

    enum PrimitiveInfoKind kind = type_info->data.primitive.kind;
    return (kind == PRIMITIVE_INFO_F32 || kind == PRIMITIVE_INFO_F64);
}

bool type_info_is_signed(const TypeInfo *type_info) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE)
        return false;
    return type_info->data.primitive.is_signed;
}

bool type_info_is_comparable(const TypeInfo *type_info) {
    if (!type_info)
        return false;

    switch (type_info->category) {
    case TYPE_INFO_PRIMITIVE:
        // All primitives except void are comparable
        return type_info->data.primitive.kind != PRIMITIVE_INFO_VOID;
    case TYPE_INFO_POINTER:
        return true; // Pointers are comparable
    default:
        return false;
    }
}

bool type_info_is_logical(const TypeInfo *type_info) {
    if (!type_info || type_info->category != TYPE_INFO_PRIMITIVE)
        return false;
    return type_info->data.primitive.kind == PRIMITIVE_INFO_BOOL;
}

bool type_info_is_ffi_compatible(const TypeInfo *type_info) {
    if (!type_info)
        return false;
    return type_info->flags.is_ffi_compatible;
}

size_t type_info_get_size(const TypeInfo *type_info) {
    return type_info ? type_info->size : 0;
}

size_t type_info_get_alignment(const TypeInfo *type_info) {
    return type_info ? type_info->alignment : 1;
}

bool type_info_can_cast(const TypeInfo *from, const TypeInfo *to) {
    if (!from || !to)
        return false;
    if (type_info_equals(from, to))
        return true;

    // Allow casting between numeric types
    if (type_info_is_numeric(from) && type_info_is_numeric(to)) {
        return true;
    }

    // Allow casting between pointer types
    if (from->category == TYPE_INFO_POINTER && to->category == TYPE_INFO_POINTER) {
        return true;
    }

    return false;
}

bool type_info_is_compatible(const TypeInfo *a, const TypeInfo *b) {
    if (!a || !b)
        return false;

    // Exact match
    if (type_info_equals(a, b))
        return true;

    // Compatible numeric types
    if (type_info_is_numeric(a) && type_info_is_numeric(b)) {
        // Allow implicit conversions between compatible numeric types
        if (type_info_is_integer(a) && type_info_is_integer(b)) {
            return true; // Integer types are compatible
        }
        if (type_info_is_float(a) && type_info_is_float(b)) {
            return true; // Float types are compatible
        }
    }

    return false;
}
