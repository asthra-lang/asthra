#ifndef ASTHRA_AI_AST_NAVIGATION_H
#define ASTHRA_AI_AST_NAVIGATION_H

#include "../parser/ast.h"
#include "semantic_api_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CODE LOCATION AND CONTEXT
// =============================================================================

// Forward declaration - full definition in semantic_api_core.h
// typedef struct AICodeLocation AICodeLocation;

/**
 * Symbol usage information
 */
typedef struct {
    char *symbol_name;       // Name of the symbol
    char *usage_type;        // "declaration", "definition", "reference", "assignment"
    AICodeLocation location; // Location of the usage
    char *scope_name;        // Name of the containing scope
} AISymbolUsage;

// =============================================================================
// AST NAVIGATION AND SEARCH
// =============================================================================

/**
 * Find the declaration node for a symbol
 * @param api The semantic API instance
 * @param symbol_name The symbol name to find
 * @return AST node of the declaration, or NULL if not found
 * @note The returned node is owned by the AST and should not be freed
 */
ASTNode *asthra_ai_find_declaration(AsthraSemanticsAPI *api, const char *symbol_name);

/**
 * Find all usage locations of a symbol
 * @param api The semantic API instance
 * @param symbol_name The symbol name to find usages for
 * @param count Output parameter for array size
 * @return Array of symbol usage information, or NULL if none found
 * @note Caller must free using asthra_ai_free_symbol_usage_array
 */
AISymbolUsage **asthra_ai_find_symbol_usages(AsthraSemanticsAPI *api, const char *symbol_name,
                                             size_t *count);

/**
 * Get documentation for a symbol (from comments)
 * @param api The semantic API instance
 * @param symbol_name The symbol name to get documentation for
 * @return Documentation string, or NULL if none found
 * @note Caller must free the returned string
 */
char *asthra_ai_get_symbol_documentation(AsthraSemanticsAPI *api, const char *symbol_name);

/**
 * Get all symbol locations (declarations and usages)
 * @param api The semantic API instance
 * @param symbol_name The symbol name to find locations for
 * @param count Output parameter for array size
 * @return Array of code locations, or NULL if none found
 * @note Caller must free using asthra_ai_free_code_location_array
 */
AICodeLocation **asthra_ai_get_symbol_locations(AsthraSemanticsAPI *api, const char *symbol_name,
                                                size_t *count);

// =============================================================================
// AST TRAVERSAL AND ANALYSIS
// =============================================================================

/**
 * Get the parent node of a given AST node
 * @param api The semantic API instance
 * @param node The AST node to find parent for
 * @return Parent AST node, or NULL if root or not found
 * @note The returned node is owned by the AST and should not be freed
 */
ASTNode *asthra_ai_get_parent_node(AsthraSemanticsAPI *api, ASTNode *node);

/**
 * Get all child nodes of a given AST node
 * @param api The semantic API instance
 * @param node The AST node to get children for
 * @param count Output parameter for array size
 * @return Array of child AST nodes, or NULL if none
 * @note Caller must free the returned array (but not the nodes themselves)
 */
ASTNode **asthra_ai_get_child_nodes(AsthraSemanticsAPI *api, ASTNode *node, size_t *count);

/**
 * Find nodes of a specific type in the AST
 * @param api The semantic API instance
 * @param node_type The AST node type to search for
 * @param count Output parameter for array size
 * @return Array of matching AST nodes, or NULL if none found
 * @note Caller must free the returned array (but not the nodes themselves)
 */
ASTNode **asthra_ai_find_nodes_by_type(AsthraSemanticsAPI *api, ASTNodeType node_type,
                                       size_t *count);

/**
 * Get the scope containing a given AST node
 * @param api The semantic API instance
 * @param node The AST node to find scope for
 * @return Scope name as string, or NULL if not found
 * @note Caller must free the returned string
 */
char *asthra_ai_get_node_scope(AsthraSemanticsAPI *api, ASTNode *node);

// =============================================================================
// CODE CONTEXT AND ANALYSIS
// =============================================================================

/**
 * Get the source code context around a location
 * @param api The semantic API instance
 * @param file_path The source file path
 * @param line The line number (1-based)
 * @param context_lines Number of lines before/after to include
 * @return Code context string, or NULL if not found
 * @note Caller must free the returned string
 */
char *asthra_ai_get_code_context(AsthraSemanticsAPI *api, const char *file_path, size_t line,
                                 size_t context_lines);

/**
 * Get all symbols visible at a given location
 * @param api The semantic API instance
 * @param file_path The source file path
 * @param line The line number (1-based)
 * @param column The column number (1-based)
 * @param count Output parameter for array size
 * @return Array of visible symbol names, or NULL if none
 * @note Caller must free using asthra_ai_free_string_array
 */
char **asthra_ai_get_visible_symbols(AsthraSemanticsAPI *api, const char *file_path, size_t line,
                                     size_t column, size_t *count);

/**
 * Check if a symbol is accessible from a given location
 * @param api The semantic API instance
 * @param symbol_name The symbol name to check
 * @param file_path The source file path
 * @param line The line number (1-based)
 * @param column The column number (1-based)
 * @return true if symbol is accessible, false otherwise
 */
bool asthra_ai_is_symbol_accessible(AsthraSemanticsAPI *api, const char *symbol_name,
                                    const char *file_path, size_t line, size_t column);

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

/**
 * Free array of code locations
 * @param locations The array to free
 * @param count Number of elements in the array
 */
void asthra_ai_free_code_location_array(AICodeLocation **locations, size_t count);

/**
 * Free array of symbol usages
 * @param usages The array to free
 * @param count Number of elements in the array
 */
void asthra_ai_free_symbol_usage_array(AISymbolUsage **usages, size_t count);

/**
 * Free a single code location
 * @param location The location to free
 */
void asthra_ai_free_code_location(AICodeLocation *location);

/**
 * Free a single symbol usage
 * @param usage The usage to free
 */
void asthra_ai_free_symbol_usage(AISymbolUsage *usage);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_AI_AST_NAVIGATION_H
