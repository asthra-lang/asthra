#ifndef ASTHRA_TYPE_INFO_REGISTRY_H
#define ASTHRA_TYPE_INFO_REGISTRY_H

#include "type_info_types.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE REGISTRY INTEGRATION
// =============================================================================

/**
 * Register a TypeInfo with the runtime type system
 */
bool type_info_register(TypeInfo *type_info);

/**
 * Look up TypeInfo by type ID
 */
TypeInfo *type_info_lookup_by_id(uint32_t type_id);

/**
 * Look up TypeInfo by name
 */
TypeInfo *type_info_lookup_by_name(const char *name);

/**
 * Get all registered types
 */
TypeInfo **type_info_get_all_types(size_t *count);

// =============================================================================
// TYPE REGISTRY OPERATIONS
// =============================================================================

/**
 * Clean up the type registry
 */
void type_info_registry_cleanup(void);

// External registry variables (for internal use by type_info modules)
extern TypeInfo **g_type_registry;
extern size_t g_type_registry_size;
extern pthread_mutex_t g_type_registry_mutex;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_REGISTRY_H