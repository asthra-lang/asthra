#include "ast_navigation_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// =============================================================================
// AST NAVIGATION AND SEARCH
// =============================================================================

ASTNode *asthra_ai_find_declaration(AsthraSemanticsAPI *api, const char *symbol_name) {
    if (!is_api_valid(api) || !symbol_name) {
        return NULL;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    // Look up the symbol in the symbol table
    SymbolEntry *symbol = symbol_table_lookup_safe(api->analyzer->global_scope, symbol_name);
    if (!symbol) {
        // Try current scope if global lookup fails
        if (api->analyzer->current_scope) {
            symbol = symbol_table_lookup_safe(api->analyzer->current_scope, symbol_name);
        }
    }
    
    ASTNode *declaration = NULL;
    if (symbol && symbol->declaration) {
        declaration = symbol->declaration;
    }
    
    pthread_mutex_unlock(&api->api_mutex);
    return declaration;
}

// Helper function to traverse AST and collect symbol usages
void collect_symbol_usages_recursive(ASTNode *node, SymbolUsageCollector *collector) {
    if (!node) return;
    
    // Check if this node references our target symbol
    switch (node->type) {
        case AST_IDENTIFIER:
            if (node->data.identifier.name && 
                strcmp(node->data.identifier.name, collector->target_symbol) == 0) {
                add_usage_to_collector(collector, node, "reference");
            }
            break;
            
        case AST_FUNCTION_DECL:
            if (node->data.function_decl.name && 
                strcmp(node->data.function_decl.name, collector->target_symbol) == 0) {
                add_usage_to_collector(collector, node, "declaration");
            }
            // Traverse children
            if (node->data.function_decl.params) {
                for (size_t i = 0; i < ast_node_list_size(node->data.function_decl.params); i++) {
                    collect_symbol_usages_recursive(ast_node_list_get(node->data.function_decl.params, i), collector);
                }
            }
            if (node->data.function_decl.body) {
                collect_symbol_usages_recursive(node->data.function_decl.body, collector);
            }
            break;
            
        case AST_STRUCT_DECL:
            if (node->data.struct_decl.name && 
                strcmp(node->data.struct_decl.name, collector->target_symbol) == 0) {
                add_usage_to_collector(collector, node, "declaration");
            }
            break;
            
        case AST_LET_STMT:
            if (node->data.let_stmt.name && 
                strcmp(node->data.let_stmt.name, collector->target_symbol) == 0) {
                add_usage_to_collector(collector, node, "declaration");
            }
            if (node->data.let_stmt.initializer) {
                collect_symbol_usages_recursive(node->data.let_stmt.initializer, collector);
            }
            break;
            
        case AST_ASSIGNMENT:
            if (node->data.assignment.target) {
                collect_symbol_usages_recursive(node->data.assignment.target, collector);
            }
            if (node->data.assignment.value) {
                collect_symbol_usages_recursive(node->data.assignment.value, collector);
            }
            break;
            
        case AST_CALL_EXPR:
            if (node->data.call_expr.function) {
                collect_symbol_usages_recursive(node->data.call_expr.function, collector);
            }
            if (node->data.call_expr.args) {
                for (size_t i = 0; i < ast_node_list_size(node->data.call_expr.args); i++) {
                    collect_symbol_usages_recursive(ast_node_list_get(node->data.call_expr.args, i), collector);
                }
            }
            break;
            
        case AST_BINARY_EXPR:
            if (node->data.binary_expr.left) {
                collect_symbol_usages_recursive(node->data.binary_expr.left, collector);
            }
            if (node->data.binary_expr.right) {
                collect_symbol_usages_recursive(node->data.binary_expr.right, collector);
            }
            break;
            
        case AST_BLOCK:
            if (node->data.block.statements) {
                for (size_t i = 0; i < ast_node_list_size(node->data.block.statements); i++) {
                    collect_symbol_usages_recursive(ast_node_list_get(node->data.block.statements, i), collector);
                }
            }
            break;
            
        case AST_PROGRAM:
            if (node->data.program.declarations) {
                for (size_t i = 0; i < ast_node_list_size(node->data.program.declarations); i++) {
                    collect_symbol_usages_recursive(ast_node_list_get(node->data.program.declarations, i), collector);
                }
            }
            break;
            
        default:
            // For other node types, we could add more cases as needed
            break;
    }
}

AISymbolUsage **asthra_ai_find_symbol_usages(AsthraSemanticsAPI *api, const char *symbol_name, size_t *count) {
    if (!is_api_valid(api) || !symbol_name || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = 0;
    pthread_mutex_lock(&api->api_mutex);
    
    // Initialize collector
    SymbolUsageCollector collector = {
        .target_symbol = symbol_name,
        .usages = NULL,
        .usage_count = 0,
        .usage_capacity = 0,
        .source_file_path = api->source_file_path
    };
    
    // TODO: Need access to root AST for comprehensive symbol usage tracking
    // For now, return just the declaration from symbol table
    SymbolEntry *symbol = symbol_table_lookup_safe(api->analyzer->global_scope, symbol_name);
    if (symbol && symbol->declaration) {
        collector.usages = malloc(sizeof(AISymbolUsage*) * 1);
        if (collector.usages) {
            add_usage_to_collector(&collector, symbol->declaration, "declaration");
        }
    }
    
    *count = collector.usage_count;
    
    pthread_mutex_unlock(&api->api_mutex);
    return collector.usages;
}

char *asthra_ai_get_symbol_documentation(AsthraSemanticsAPI *api, const char *symbol_name) {
    if (!is_api_valid(api) || !symbol_name) {
        return NULL;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    // Look up the symbol
    SymbolEntry *symbol = symbol_table_lookup_safe(api->analyzer->global_scope, symbol_name);
    if (!symbol) {
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    // For now, return a placeholder
    // A full implementation would extract documentation from comments
    char *doc = malloc(256);
    if (doc) {
        snprintf(doc, 256, "Documentation for %s (kind: %s)", 
                symbol_name, 
                symbol->kind == SYMBOL_FUNCTION ? "function" :
                symbol->kind == SYMBOL_TYPE ? "type" :
                symbol->kind == SYMBOL_VARIABLE ? "variable" : "symbol");
    }
    
    pthread_mutex_unlock(&api->api_mutex);
    return doc;
}

AICodeLocation **asthra_ai_get_symbol_locations(AsthraSemanticsAPI *api, const char *symbol_name, size_t *count) {
    if (!is_api_valid(api) || !symbol_name || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = 0;
    pthread_mutex_lock(&api->api_mutex);
    
    // Look up the symbol
    SymbolEntry *symbol = symbol_table_lookup_safe(api->analyzer->global_scope, symbol_name);
    if (!symbol || !symbol->declaration) {
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    AICodeLocation **locations = malloc(sizeof(AICodeLocation*) * 1);
    if (!locations) {
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    AICodeLocation *location = malloc(sizeof(AICodeLocation));
    if (!location) {
        free(locations);
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    location->file_path = strdup(api->source_file_path ? api->source_file_path : "unknown");
    location->line = (size_t)symbol->declaration->location.line;
    location->column = (size_t)symbol->declaration->location.column;
    location->context_code = strdup(""); // Would extract actual context
    
    locations[0] = location;
    *count = 1;
    
    pthread_mutex_unlock(&api->api_mutex);
    return locations;
}