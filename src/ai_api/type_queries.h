#ifndef ASTHRA_AI_TYPE_QUERIES_H
#define ASTHRA_AI_TYPE_QUERIES_H

#include "semantic_api_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE INFERENCE AND CHECKING
// =============================================================================

/**
 * Infer the type of an Asthra expression
 * @param api The semantic API instance
 * @param expression The expression string to analyze
 * @return Type name as string, or NULL if inference failed
 * @note Caller must free the returned string
 */
char *asthra_ai_infer_expression_type(AsthraSemanticsAPI *api, const char *expression);

/**
 * Check if two types are compatible
 * @param api The semantic API instance
 * @param expected The expected type name
 * @param actual The actual type name
 * @return true if types are compatible, false otherwise
 */
bool asthra_ai_check_type_compatibility(AsthraSemanticsAPI *api, const char *expected, const char *actual);

/**
 * Get list of types compatible with a given context
 * @param api The semantic API instance
 * @param context The context string (e.g., "assignment", "parameter")
 * @param count Output parameter for array size
 * @return Array of compatible type names, or NULL if none found
 * @note Caller must free the returned array using asthra_ai_free_string_array
 */
char **asthra_ai_get_compatible_types(AsthraSemanticsAPI *api, const char *context, size_t *count);

// =============================================================================
// TYPE INFORMATION QUERIES
// =============================================================================

/**
 * Detailed type information structure
 */
struct AITypeInfo {
    char *type_name;
    char *category;          // "primitive", "struct", "enum", "slice", "pointer", "function"
    bool is_mutable;
    bool is_generic;
    char **generic_params;   // Array of generic parameter names
    size_t generic_param_count;
    
    // Additional type metadata
    size_t size_bytes;       // Type size in bytes (0 if unknown)
    size_t alignment;        // Type alignment (0 if unknown)
    bool is_ffi_compatible;  // Can be used in FFI
    bool is_copy_type;       // Can be copied (vs moved)
};

/**
 * Get detailed information about a type
 * @param api The semantic API instance
 * @param type_name The type name to query
 * @return Type information structure, or NULL if type not found
 * @note Caller must free using asthra_ai_free_type_info
 */
AITypeInfo *asthra_ai_get_type_info(AsthraSemanticsAPI *api, const char *type_name);

/**
 * Get all available types in the current scope
 * @param api The semantic API instance
 * @param count Output parameter for array size
 * @return Array of type names, or NULL if none found
 * @note Caller must free using asthra_ai_free_string_array
 */
char **asthra_ai_get_available_types(AsthraSemanticsAPI *api, size_t *count);

/**
 * Check if a type is a primitive type
 * @param api The semantic API instance
 * @param type_name The type name to check
 * @return true if primitive, false otherwise
 */
bool asthra_ai_is_primitive_type(AsthraSemanticsAPI *api, const char *type_name);

/**
 * Get the underlying type for a type alias
 * @param api The semantic API instance
 * @param alias_name The type alias name
 * @return Underlying type name, or NULL if not an alias
 * @note Caller must free the returned string
 */
char *asthra_ai_resolve_type_alias(AsthraSemanticsAPI *api, const char *alias_name);

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

/**
 * Free type information structure
 * @param info The type info to free
 */
void asthra_ai_free_type_info(AITypeInfo *info);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_TYPE_QUERIES_H
