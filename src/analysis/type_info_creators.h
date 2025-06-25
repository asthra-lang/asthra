#ifndef ASTHRA_TYPE_INFO_CREATORS_H
#define ASTHRA_TYPE_INFO_CREATORS_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE INFO CREATION HELPERS
// =============================================================================

/**
 * Create primitive type info
 */
TypeInfo *type_info_create_primitive(const char *name, int primitive_kind, size_t size);

/**
 * Create struct type info
 */
TypeInfo *type_info_create_struct(const char *name, SymbolEntry **fields, size_t field_count);

/**
 * Create slice type info
 */
TypeInfo *type_info_create_slice(TypeInfo *element_type, bool is_mutable);

/**
 * Create pointer type info
 */
TypeInfo *type_info_create_pointer(TypeInfo *pointee_type, bool is_mutable);

/**
 * Create Result type info
 */
TypeInfo *type_info_create_result(TypeInfo *ok_type, TypeInfo *err_type);

/**
 * Create Option type info
 */
TypeInfo *type_info_create_option(TypeInfo *value_type);

/**
 * Create function type info
 */
TypeInfo *type_info_create_function(TypeInfo **param_types, size_t param_count,
                                    TypeInfo *return_type);

/**
 * Create module type info
 */
TypeInfo *type_info_create_module(const char *module_name, SymbolEntry **exports,
                                  size_t export_count);

/**
 * Create tuple type info
 */
TypeInfo *type_info_create_tuple(TypeInfo **element_types, size_t element_count);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_CREATORS_H