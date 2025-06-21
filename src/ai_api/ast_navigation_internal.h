#ifndef ASTHRA_AI_AST_NAVIGATION_INTERNAL_H
#define ASTHRA_AI_AST_NAVIGATION_INTERNAL_H

#include "ast_navigation.h"
#include "../analysis/semantic_symbols.h"
#include "../parser/ast_operations.h"

// Helper structure for collecting symbol usages
typedef struct {
    const char *target_symbol;
    AISymbolUsage **usages;
    size_t usage_count;
    size_t usage_capacity;
    const char *source_file_path;
} SymbolUsageCollector;

// Helper functions shared across ast_navigation implementation files
bool is_api_valid(AsthraSemanticsAPI *api);
const char* usage_type_to_string(ASTNodeType node_type);
void add_usage_to_collector(SymbolUsageCollector *collector, ASTNode *node, const char *usage_type);
void collect_symbol_names(SymbolTable *table, char ***symbols, size_t *count, size_t *capacity);

// Internal function for recursive symbol usage collection
void collect_symbol_usages_recursive(ASTNode *node, SymbolUsageCollector *collector);

#endif // ASTHRA_AI_AST_NAVIGATION_INTERNAL_H