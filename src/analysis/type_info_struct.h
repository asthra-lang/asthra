#ifndef ASTHRA_TYPE_INFO_STRUCT_H
#define ASTHRA_TYPE_INFO_STRUCT_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STRUCT TYPE OPERATIONS
// =============================================================================

/**
 * Add a field to a struct type
 */
bool type_info_struct_add_field(TypeInfo *struct_type, const char *field_name, TypeInfo *field_type,
                                size_t offset);

/**
 * Look up a field in a struct type
 */
SymbolEntry *type_info_struct_get_field(const TypeInfo *struct_type, const char *field_name);

/**
 * Get field offset in struct
 */
size_t type_info_struct_get_field_offset(const TypeInfo *struct_type, const char *field_name);

/**
 * Calculate struct layout and field offsets
 */
bool type_info_struct_calculate_layout(TypeInfo *struct_type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_STRUCT_H