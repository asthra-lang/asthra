#ifndef ASTHRA_TYPE_INFO_INTEGRATION_H
#define ASTHRA_TYPE_INFO_INTEGRATION_H

#include "type_info_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// AST INTEGRATION
// =============================================================================

/**
 * Attach TypeInfo to an AST node
 */
void ast_node_set_type_info(ASTNode *node, TypeInfo *type_info);

/**
 * Get TypeInfo from an AST node
 */
TypeInfo *ast_node_get_type_info(const ASTNode *node);

/**
 * Propagate type information through AST
 */
bool type_info_propagate_ast(ASTNode *root);

// =============================================================================
// SEMANTIC ANALYSIS INTEGRATION
// =============================================================================

/**
 * Convert TypeDescriptor to TypeInfo
 */
TypeInfo *type_info_from_type_descriptor(const TypeDescriptor *descriptor);

/**
 * Convert TypeInfo to TypeDescriptor
 */
TypeDescriptor *type_descriptor_from_type_info(const TypeInfo *type_info);

/**
 * Synchronize TypeInfo with TypeDescriptor
 */
bool type_info_sync_with_descriptor(TypeInfo *type_info, TypeDescriptor *descriptor);

// =============================================================================
// RUNTIME INTEGRATION
// =============================================================================

/**
 * Get runtime type ID for TypeInfo
 */
uint32_t type_info_get_runtime_id(const TypeInfo *type_info);

/**
 * Register TypeInfo with runtime type system
 */
bool type_info_register_runtime(TypeInfo *type_info);

/**
 * Create runtime type descriptor from TypeInfo
 */
void *type_info_create_runtime_descriptor(const TypeInfo *type_info);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TYPE_INFO_INTEGRATION_H