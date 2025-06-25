#ifndef ASTHRA_TYPE_INFO_QUERIES_H
#define ASTHRA_TYPE_INFO_QUERIES_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE INFO QUERIES AND OPERATIONS
// =============================================================================

/**
 * Check if two TypeInfo structures represent the same type
 */
bool type_info_equals(const TypeInfo *a, const TypeInfo *b);

/**
 * Check if type A is compatible with type B (for assignment)
 */
bool type_info_is_compatible(const TypeInfo *a, const TypeInfo *b);

/**
 * Check if type A can be cast to type B
 */
bool type_info_can_cast(const TypeInfo *from, const TypeInfo *to);

/**
 * Get the size of a type in bytes
 */
size_t type_info_get_size(const TypeInfo *type_info);

/**
 * Get the alignment of a type in bytes
 */
size_t type_info_get_alignment(const TypeInfo *type_info);

/**
 * Check if type is a numeric type
 */
bool type_info_is_numeric(const TypeInfo *type_info);

/**
 * Check if type is an integer type
 */
bool type_info_is_integer(const TypeInfo *type_info);

/**
 * Check if type is a floating-point type
 */
bool type_info_is_float(const TypeInfo *type_info);

/**
 * Check if type is a signed type
 */
bool type_info_is_signed(const TypeInfo *type_info);

/**
 * Check if type supports comparison operations
 */
bool type_info_is_comparable(const TypeInfo *type_info);

/**
 * Check if type supports logical operations
 */
bool type_info_is_logical(const TypeInfo *type_info);

/**
 * Check if type is FFI-compatible
 */
bool type_info_is_ffi_compatible(const TypeInfo *type_info);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_QUERIES_H