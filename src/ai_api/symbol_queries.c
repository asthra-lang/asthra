#include "../analysis/semantic_symbols.h"
#include "../analysis/semantic_types.h"
#include "semantic_api_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

// Helper function to get current time in milliseconds
static double get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}

// Helper function to check if API is valid and initialized
static bool is_api_valid(AsthraSemanticsAPI *api) {
    return api != NULL && api->analyzer != NULL && api->is_initialized;
}

// Helper function to convert symbol kind to string
static const char *symbol_kind_to_string(SymbolKind kind) {
    switch (kind) {
    case SYMBOL_VARIABLE:
        return "variable";
    case SYMBOL_FUNCTION:
        return "function";
    case SYMBOL_TYPE:
        return "type";
    case SYMBOL_FIELD:
        return "field";
    case SYMBOL_ENUM_VARIANT:
        return "enum_variant";
    case SYMBOL_PARAMETER:
        return "parameter";
    default:
        return "unknown";
    }
}

// Context structures for symbol table iteration
typedef struct {
    AIFieldInfo *fields;
    size_t current_index;
    size_t max_count;
} FieldExtractionContext;

typedef struct {
    char **method_names;
    size_t current_index;
    size_t max_count;
} MethodExtractionContext;

// Callback function for field extraction
static bool extract_field_callback(const char *name, SymbolEntry *entry, void *user_data) {
    FieldExtractionContext *ctx = (FieldExtractionContext *)user_data;

    if (!ctx || !entry || ctx->current_index >= ctx->max_count) {
        return false; // Stop iteration
    }

    // Only process field symbols
    if (entry->kind != SYMBOL_FIELD) {
        return true; // Continue iteration
    }

    AIFieldInfo *field = &ctx->fields[ctx->current_index];

    // Extract field information
    field->name = strdup(name ? name : "");
    field->type_name =
        entry->type ? strdup(entry->type->name ? entry->type->name : "unknown") : strdup("unknown");
    field->is_public = (entry->visibility == VISIBILITY_PUBLIC);
    field->is_mutable = entry->flags.is_mutable;
    field->default_value = strdup(""); // TODO: Extract default value if available

    ctx->current_index++;
    return true; // Continue iteration
}

// Callback function for method extraction
static bool extract_method_callback(const char *name, SymbolEntry *entry, void *user_data) {
    MethodExtractionContext *ctx = (MethodExtractionContext *)user_data;

    if (!ctx || !entry || ctx->current_index >= ctx->max_count) {
        return false; // Stop iteration
    }

    // Only process function symbols (methods)
    if (entry->kind != SYMBOL_FUNCTION) {
        return true; // Continue iteration
    }

    // Extract method name
    ctx->method_names[ctx->current_index] = strdup(name ? name : "");

    ctx->current_index++;
    return true; // Continue iteration
}

AISymbolInfo *asthra_ai_get_symbol_info(AsthraSemanticsAPI *api, const char *symbol_name) {
    if (!is_api_valid(api) || !symbol_name) {
        return NULL;
    }

    double start_time = get_current_time_ms();
    pthread_mutex_lock(&api->api_mutex);

    // Update performance counters
    api->total_queries++;

    // For now, skip cache lookup until proper serialization is implemented
    // This avoids the unsafe pointer caching issue
    api->cache_misses++;

    // Use existing symbol table lookup
    SymbolEntry *symbol = symbol_table_lookup_safe(api->analyzer->global_scope, symbol_name);
    if (!symbol) {
        // Try current scope if global lookup fails
        if (api->analyzer->current_scope) {
            symbol = symbol_table_lookup_safe(api->analyzer->current_scope, symbol_name);
        }
    }

    if (!symbol) {
        double end_time = get_current_time_ms();
        api->total_query_time_ms += (end_time - start_time);
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }

    // Convert to AI-friendly format
    AISymbolInfo *info = malloc(sizeof(AISymbolInfo));
    if (!info) {
        double end_time = get_current_time_ms();
        api->total_query_time_ms += (end_time - start_time);
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }

    info->name = strdup(symbol->name ? symbol->name : "");
    info->type_name = symbol->type ? strdup(symbol->type->name ? symbol->type->name : "unknown")
                                   : strdup("unknown");
    info->kind = strdup(symbol_kind_to_string(symbol->kind));
    info->is_public = (symbol->visibility == VISIBILITY_PUBLIC);
    info->is_mutable = symbol->flags.is_mutable;
    info->documentation = strdup(""); // TODO: Extract from comments when available

    if (symbol->declaration) {
        info->location = symbol->declaration->location;
    } else {
        memset(&info->location, 0, sizeof(SourceLocation));
    }

    // Cache the result for future use
    // Note: We don't cache the actual pointers, as that would be unsafe
    // For now, we'll skip caching complex structures and rely on symbol table lookup
    // TODO: Implement proper serialization for caching

    double end_time = get_current_time_ms();
    api->total_query_time_ms += (end_time - start_time);

    pthread_mutex_unlock(&api->api_mutex);
    return info;
}

AIStructInfo *asthra_ai_get_struct_info(AsthraSemanticsAPI *api, const char *struct_name) {
    if (!is_api_valid(api) || !struct_name) {
        return NULL;
    }

    double start_time = get_current_time_ms();
    pthread_mutex_lock(&api->api_mutex);

    // Update performance counters
    api->total_queries++;

    // Check cache first
    void *cached_data = NULL;
    size_t cached_size = 0;
    char cache_key[256];
    snprintf(cache_key, sizeof(cache_key), "struct_info_%s", struct_name);

    bool cache_hit = false;
    if (api->performance_cache &&
        semantic_cache_get(api->performance_cache, cache_key, &cached_data, &cached_size)) {
        api->cache_hits++;
        cache_hit = true;
        // For struct info, we'll skip complex cache restoration for now
        // and just count it as a cache hit but still do the work
        free(cached_data);
    }

    if (!cache_hit) {
        api->cache_misses++;
    }

    // Look up struct symbol using existing infrastructure
    SymbolEntry *struct_symbol = symbol_table_lookup_safe(api->analyzer->global_scope, struct_name);
    if (!struct_symbol || struct_symbol->kind != SYMBOL_TYPE || !struct_symbol->type ||
        struct_symbol->type->category != TYPE_STRUCT) {
        double end_time = get_current_time_ms();
        api->total_query_time_ms += (end_time - start_time);
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }

    TypeDescriptor *struct_type = struct_symbol->type;
    AIStructInfo *info = malloc(sizeof(AIStructInfo));
    if (!info) {
        double end_time = get_current_time_ms();
        api->total_query_time_ms += (end_time - start_time);
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }

    info->struct_name = strdup(struct_name);
    info->is_generic = struct_symbol->is_generic;
    info->type_param_count = struct_symbol->type_param_count;

    // Initialize arrays
    info->fields = NULL;
    info->field_count = 0;
    info->method_names = NULL;
    info->method_count = 0;
    info->type_parameters = NULL;

    // Extract field information using existing symbol table
    if (struct_type->data.struct_type.fields) {
        SymbolTable *fields = struct_type->data.struct_type.fields;
        size_t field_count = symbol_table_size(fields);

        if (field_count > 0) {
            info->fields = malloc(sizeof(AIFieldInfo) * field_count);
            if (info->fields) {
                // Use symbol table iteration to extract fields
                FieldExtractionContext field_ctx = {
                    .fields = info->fields, .current_index = 0, .max_count = field_count};

                symbol_table_iterate(fields, extract_field_callback, &field_ctx);
                info->field_count = field_ctx.current_index;
            }
        }
    }

    // Extract method information using existing method table
    if (struct_type->data.struct_type.methods) {
        SymbolTable *methods = struct_type->data.struct_type.methods;
        size_t method_count = symbol_table_size(methods);

        if (method_count > 0) {
            info->method_names = malloc(sizeof(char *) * method_count);
            if (info->method_names) {
                // Use symbol table iteration to extract methods
                MethodExtractionContext method_ctx = {.method_names = info->method_names,
                                                      .current_index = 0,
                                                      .max_count = method_count};

                symbol_table_iterate(methods, extract_method_callback, &method_ctx);
                info->method_count = method_ctx.current_index;
            }
        }
    }

    // Cache the result
    if (api->performance_cache && !cache_hit) {
        semantic_cache_put(api->performance_cache, cache_key, info, sizeof(AIStructInfo));
    }

    double end_time = get_current_time_ms();
    api->total_query_time_ms += (end_time - start_time);

    pthread_mutex_unlock(&api->api_mutex);
    return info;
}

char **asthra_ai_get_available_methods(AsthraSemanticsAPI *api, const char *type_name,
                                       size_t *count) {
    if (!is_api_valid(api) || !type_name || !count) {
        if (count)
            *count = 0;
        return NULL;
    }

    *count = 0;

    pthread_mutex_lock(&api->api_mutex);

    // Look up type symbol
    SymbolEntry *type_symbol = symbol_table_lookup_safe(api->analyzer->global_scope, type_name);
    if (!type_symbol || !type_symbol->type) {
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }

    // For now, return empty array - method extraction needs symbol table iteration
    pthread_mutex_unlock(&api->api_mutex);
    return NULL;
}

AIFieldInfo **asthra_ai_get_struct_fields(AsthraSemanticsAPI *api, const char *struct_name,
                                          size_t *count) {
    if (!is_api_valid(api) || !struct_name || !count) {
        if (count)
            *count = 0;
        return NULL;
    }

    *count = 0;

    // Use the struct info function and extract fields
    AIStructInfo *struct_info = asthra_ai_get_struct_info(api, struct_name);
    if (!struct_info) {
        return NULL;
    }

    if (struct_info->field_count == 0) {
        asthra_ai_free_struct_info(struct_info);
        return NULL;
    }

    // Allocate array of field info pointers
    AIFieldInfo **fields = malloc(sizeof(AIFieldInfo *) * struct_info->field_count);
    if (!fields) {
        asthra_ai_free_struct_info(struct_info);
        return NULL;
    }

    // Copy field information
    for (size_t i = 0; i < struct_info->field_count; i++) {
        fields[i] = malloc(sizeof(AIFieldInfo));
        if (fields[i]) {
            fields[i]->name = strdup(struct_info->fields[i].name);
            fields[i]->type_name = strdup(struct_info->fields[i].type_name);
            fields[i]->is_public = struct_info->fields[i].is_public;
            fields[i]->is_mutable = struct_info->fields[i].is_mutable;
            fields[i]->default_value = strdup(
                struct_info->fields[i].default_value ? struct_info->fields[i].default_value : "");
        }
    }

    *count = struct_info->field_count;
    asthra_ai_free_struct_info(struct_info);
    return fields;
}

char **asthra_ai_get_enum_variants(AsthraSemanticsAPI *api, const char *enum_name, size_t *count) {
    if (!is_api_valid(api) || !enum_name || !count) {
        if (count)
            *count = 0;
        return NULL;
    }

    *count = 0;

    pthread_mutex_lock(&api->api_mutex);

    // Look up enum symbol
    SymbolEntry *enum_symbol = symbol_table_lookup_safe(api->analyzer->global_scope, enum_name);
    if (!enum_symbol || enum_symbol->kind != SYMBOL_TYPE || !enum_symbol->type ||
        enum_symbol->type->category != TYPE_ENUM) {
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }

    // For now, return empty array - enum variant extraction needs implementation
    pthread_mutex_unlock(&api->api_mutex);
    return NULL;
}
