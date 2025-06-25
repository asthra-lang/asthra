#ifndef ASTHRA_TYPE_INFO_LIFECYCLE_H
#define ASTHRA_TYPE_INFO_LIFECYCLE_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE INFO LIFECYCLE MANAGEMENT
// =============================================================================

/**
 * Create a new TypeInfo structure
 */
TypeInfo *type_info_create(const char *name, uint32_t type_id);

/**
 * Create TypeInfo from TypeDescriptor (semantic analysis integration)
 */
TypeInfo *type_info_from_descriptor(TypeDescriptor *descriptor);

/**
 * Create TypeInfo from AST type node
 */
TypeInfo *type_info_from_ast_node(ASTNode *type_node);

/**
 * Retain a TypeInfo (increment reference count)
 */
void type_info_retain(TypeInfo *type_info);

/**
 * Release a TypeInfo (decrement reference count, free if zero)
 */
void type_info_release(TypeInfo *type_info);

/**
 * Clone a TypeInfo (deep copy)
 */
TypeInfo *type_info_clone(const TypeInfo *type_info);

// =============================================================================
// TYPE ID ALLOCATION
// =============================================================================

/**
 * Allocate a new unique type ID
 */
uint32_t type_info_allocate_id(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_LIFECYCLE_H