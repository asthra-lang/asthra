#include "ast_navigation_internal.h"
#include <stdlib.h>
#include <string.h>

// Helper function to check if API is valid and initialized
bool is_api_valid(AsthraSemanticsAPI *api) {
    return api != NULL && api->analyzer != NULL && api->is_initialized;
}

// Helper function to convert usage type to string
const char* usage_type_to_string(ASTNodeType node_type) {
    switch (node_type) {
        case AST_FUNCTION_DECL:
        case AST_STRUCT_DECL:
        case AST_ENUM_DECL:
            return "declaration";
        case AST_ASSIGNMENT:
            return "assignment";
        case AST_IDENTIFIER:
            return "reference";
        default:
            return "usage";
    }
}

// Helper function to add a usage to the collector
void add_usage_to_collector(SymbolUsageCollector *collector, ASTNode *node, const char *usage_type) {
    if (collector->usage_count >= collector->usage_capacity) {
        size_t new_capacity = collector->usage_capacity * 2;
        if (new_capacity == 0) new_capacity = 8;
        
        AISymbolUsage **new_usages = realloc(collector->usages, sizeof(AISymbolUsage*) * new_capacity);
        if (!new_usages) return;
        
        collector->usages = new_usages;
        collector->usage_capacity = new_capacity;
    }
    
    AISymbolUsage *usage = malloc(sizeof(AISymbolUsage));
    if (!usage) return;
    
    usage->symbol_name = strdup(collector->target_symbol);
    usage->usage_type = strdup(usage_type);
    usage->scope_name = strdup("unknown"); // Could be enhanced to track actual scope
    
    // Fill in location information
    usage->location.file_path = strdup(collector->source_file_path ? collector->source_file_path : "unknown");
    usage->location.line = (size_t)node->location.line;
    usage->location.column = (size_t)node->location.column;
    usage->location.context_code = strdup(""); // Could be enhanced to extract actual context
    
    collector->usages[collector->usage_count++] = usage;
}

// Helper function to collect all symbol names from a symbol table
void collect_symbol_names(SymbolTable *table, char ***symbols, size_t *count, size_t *capacity) {
    if (!table) return;
    
    // This is a simplified implementation - in reality we'd need to iterate through the symbol table
    // For now, just add some common symbols
    const char *common_symbols[] = {"log", "range", "len", "print", "panic", "assert"};
    size_t common_count = sizeof(common_symbols) / sizeof(common_symbols[0]);
    
    for (size_t i = 0; i < common_count; i++) {
        if (*count >= *capacity) {
            size_t new_capacity = (*capacity == 0) ? 8 : (*capacity * 2);
            char **new_symbols = realloc(*symbols, sizeof(char*) * new_capacity);
            if (!new_symbols) return;
            *symbols = new_symbols;
            *capacity = new_capacity;
        }
        
        (*symbols)[*count] = strdup(common_symbols[i]);
        if ((*symbols)[*count]) {
            (*count)++;
        }
    }
}