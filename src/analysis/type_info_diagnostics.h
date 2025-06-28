#ifndef ASTHRA_TYPE_INFO_DIAGNOSTICS_H
#define ASTHRA_TYPE_INFO_DIAGNOSTICS_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// DEBUG AND INTROSPECTION
// =============================================================================

/**
 * Print TypeInfo for debugging
 */
void type_info_print(const TypeInfo *type_info, int indent);

/**
 * Get string representation of TypeInfo
 */
char *type_info_to_string(const TypeInfo *type_info);

/**
 * Validate TypeInfo structure
 */
bool type_info_validate(const TypeInfo *type_info);

/**
 * Get TypeInfo statistics
 */
TypeInfoStats type_info_get_stats(void);

/**
 * Update statistics when a type is created
 */
void type_info_stats_increment(int category);

/**
 * Update statistics when a type is destroyed
 */
void type_info_stats_decrement(int category);

/**
 * Get statistics for a type category
 */
int type_info_stats_get(int category);

/**
 * Reset all statistics counters
 */
void type_info_stats_reset(void);

/**
 * Update statistics after category is set
 */
void type_info_update_stats(TypeInfo *type_info);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_DIAGNOSTICS_H