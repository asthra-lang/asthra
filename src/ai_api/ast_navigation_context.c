#include "ast_navigation_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

// =============================================================================
// CODE CONTEXT AND ANALYSIS
// =============================================================================

char *asthra_ai_get_code_context(AsthraSemanticsAPI *api, const char *file_path, size_t line, size_t context_lines) {
    if (!is_api_valid(api) || !file_path) {
        return NULL;
    }
    
    // Read the source file and extract context
    FILE *file = fopen(file_path, "r");
    if (!file) {
        return NULL;
    }
    
    // Count total lines first
    size_t total_lines = 0;
    char buffer[1024];
    while (fgets(buffer, sizeof(buffer), file)) {
        total_lines++;
    }
    rewind(file);
    
    // Calculate context range
    size_t start_line = (line > context_lines) ? line - context_lines : 1;
    size_t end_line = line + context_lines;
    if (end_line > total_lines) end_line = total_lines;
    
    // Read lines and build context
    size_t current_line = 1;
    size_t context_size = 0;
    char *context = malloc(4096); // Start with 4KB buffer
    if (!context) {
        fclose(file);
        return NULL;
    }
    context[0] = '\0';
    
    while (fgets(buffer, sizeof(buffer), file) && current_line <= end_line) {
        if (current_line >= start_line) {
            // Mark the target line with an indicator
            char line_prefix[32];
            if (current_line == line) {
                snprintf(line_prefix, sizeof(line_prefix), "→ %zu: ", current_line);
            } else {
                snprintf(line_prefix, sizeof(line_prefix), "  %zu: ", current_line);
            }
            
            size_t needed = strlen(line_prefix) + strlen(buffer) + 1;
            if (context_size + needed >= 4096) {
                // Expand buffer if needed
                char *new_context = realloc(context, context_size + needed + 1024);
                if (!new_context) {
                    fclose(file);
                    free(context);
                    return NULL;
                }
                context = new_context;
            }
            
            strcat(context, line_prefix);
            strcat(context, buffer);
            context_size += needed;
            
            // Ensure buffer ends with newline
            if (buffer[strlen(buffer)-1] != '\n') {
                strcat(context, "\n");
                context_size++;
            }
        }
        current_line++;
    }
    
    fclose(file);
    return context;
}

char **asthra_ai_get_visible_symbols(AsthraSemanticsAPI *api, const char *file_path, size_t line, size_t column, size_t *count) {
    if (!is_api_valid(api) || !file_path || !count) {
        if (count) *count = 0;
        return NULL;
    }
    
    *count = 0;
    (void)line; // Not used in this implementation
    (void)column; // Not used in this implementation
    
    pthread_mutex_lock(&api->api_mutex);
    
    char **symbols = NULL;
    size_t capacity = 0;
    
    // Collect symbols from global scope
    if (api->analyzer && api->analyzer->global_scope) {
        collect_symbol_names(api->analyzer->global_scope, &symbols, count, &capacity);
    }
    
    // Collect symbols from current scope if different from global
    if (api->analyzer && api->analyzer->current_scope && 
        api->analyzer->current_scope != api->analyzer->global_scope) {
        collect_symbol_names(api->analyzer->current_scope, &symbols, count, &capacity);
    }
    
    pthread_mutex_unlock(&api->api_mutex);
    return symbols;
}

bool asthra_ai_is_symbol_accessible(AsthraSemanticsAPI *api, const char *symbol_name, const char *file_path, size_t line, size_t column) {
    if (!is_api_valid(api) || !symbol_name || !file_path) {
        return false;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    // Simple check: if symbol exists in global scope, it's accessible
    SymbolEntry *symbol = symbol_table_lookup_safe(api->analyzer->global_scope, symbol_name);
    bool accessible = (symbol != NULL);
    
    pthread_mutex_unlock(&api->api_mutex);
    return accessible;
}