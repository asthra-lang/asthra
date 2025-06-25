#include "semantic_api_core.h"
#include "../analysis/semantic_analyzer.h"
#include "../analysis/semantic_symbols.h"
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

AsthraSemanticsAPI *asthra_ai_create_api(const char *source_file) {
    if (!source_file) {
        return NULL;
    }

    AsthraSemanticsAPI *api = malloc(sizeof(AsthraSemanticsAPI));
    if (!api) {
        return NULL;
    }

    // Initialize using existing semantic analyzer
    api->analyzer = semantic_analyzer_create();
    if (!api->analyzer) {
        free(api);
        return NULL;
    }

    // Create cached symbol table for performance
    api->cached_symbols = symbol_table_create(64);
    if (!api->cached_symbols) {
        semantic_analyzer_destroy(api->analyzer);
        free(api);
        return NULL;
    }

    // Week 7: Initialize performance cache
    CacheConfig cache_config = semantic_cache_default_config();
    api->performance_cache = semantic_cache_create(&cache_config);
    if (!api->performance_cache) {
        symbol_table_destroy(api->cached_symbols);
        semantic_analyzer_destroy(api->analyzer);
        free(api);
        return NULL;
    }

    api->source_file_path = strdup(source_file);
    if (!api->source_file_path) {
        semantic_cache_destroy(api->performance_cache);
        symbol_table_destroy(api->cached_symbols);
        semantic_analyzer_destroy(api->analyzer);
        free(api);
        return NULL;
    }

    api->is_initialized = false;

    // Week 7: Initialize performance monitoring
    api->total_queries = 0;
    api->total_query_time_ms = 0.0;
    api->cache_hits = 0;
    api->cache_misses = 0;

    // Initialize mutex for thread safety
    if (pthread_mutex_init(&api->api_mutex, NULL) != 0) {
        free(api->source_file_path);
        semantic_cache_destroy(api->performance_cache);
        symbol_table_destroy(api->cached_symbols);
        semantic_analyzer_destroy(api->analyzer);
        free(api);
        return NULL;
    }

    return api;
}

bool asthra_ai_initialize_from_source(AsthraSemanticsAPI *api, const char *source_code) {
    if (!api || !source_code) {
        return false;
    }

    pthread_mutex_lock(&api->api_mutex);

    // Create lexer from source code
    Lexer *lexer = lexer_create(source_code, strlen(source_code), api->source_file_path);
    if (!lexer) {
        pthread_mutex_unlock(&api->api_mutex);
        return false;
    }

    // Create parser with lexer
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        pthread_mutex_unlock(&api->api_mutex);
        return false;
    }

    // Parse the program
    ASTNode *program = parser_parse_program(parser);
    if (!program) {
        parser_destroy(parser);
        lexer_destroy(lexer);
        pthread_mutex_unlock(&api->api_mutex);
        return false;
    }

    // Perform semantic analysis using existing analyzer
    bool success = semantic_analyze_program(api->analyzer, program);

    if (success) {
        api->is_initialized = true;
    }

    // Clean up parser and lexer (AST is now owned by semantic analyzer)
    parser_destroy(parser);
    lexer_destroy(lexer);
    pthread_mutex_unlock(&api->api_mutex);

    return success;
}

void asthra_ai_destroy_api(AsthraSemanticsAPI *api) {
    if (!api) {
        return;
    }

    // Clean up mutex
    pthread_mutex_destroy(&api->api_mutex);

    // Week 7: Clean up performance cache
    if (api->performance_cache) {
        semantic_cache_destroy(api->performance_cache);
    }

    // Clean up semantic analyzer
    if (api->analyzer) {
        semantic_analyzer_destroy(api->analyzer);
    }

    // Clean up cached symbols
    if (api->cached_symbols) {
        symbol_table_destroy(api->cached_symbols);
    }

    // Clean up source file path
    if (api->source_file_path) {
        free(api->source_file_path);
    }

    free(api);
}

// Memory management functions for AI data structures
void asthra_ai_free_symbol_info(AISymbolInfo *info) {
    if (!info)
        return;

    free(info->name);
    free(info->type_name);
    free(info->kind);
    free(info->documentation);
    free(info);
}

void asthra_ai_free_struct_info(AIStructInfo *info) {
    if (!info)
        return;

    free(info->struct_name);

    // Free fields array
    if (info->fields) {
        for (size_t i = 0; i < info->field_count; i++) {
            free(info->fields[i].name);
            free(info->fields[i].type_name);
            free(info->fields[i].default_value);
        }
        free(info->fields);
    }

    // Free method names array
    if (info->method_names) {
        for (size_t i = 0; i < info->method_count; i++) {
            free(info->method_names[i]);
        }
        free(info->method_names);
    }

    // Free type parameters array
    if (info->type_parameters) {
        for (size_t i = 0; i < info->type_param_count; i++) {
            free(info->type_parameters[i]);
        }
        free(info->type_parameters);
    }

    free(info);
}

void asthra_ai_free_field_info_array(AIFieldInfo **fields, size_t count) {
    if (!fields)
        return;

    for (size_t i = 0; i < count; i++) {
        if (fields[i]) {
            free(fields[i]->name);
            free(fields[i]->type_name);
            free(fields[i]->default_value);
            free(fields[i]);
        }
    }
    free(fields);
}

void asthra_ai_free_string_array(char **strings, size_t count) {
    if (!strings)
        return;

    for (size_t i = 0; i < count; i++) {
        free(strings[i]);
    }
    free(strings);
}

// asthra_ai_free_type_info moved to type_queries.c
// asthra_ai_free_code_location_array moved to ast_navigation.c

// Week 7: Performance monitoring and optimization functions
AIPerformanceStats asthra_ai_get_performance_stats(AsthraSemanticsAPI *api) {
    AIPerformanceStats stats = {0};

    if (!api)
        return stats;

    pthread_mutex_lock(&api->api_mutex);

    stats.total_queries = api->total_queries;
    if (api->total_queries > 0) {
        stats.average_query_time_ms = (double)api->total_query_time_ms / (double)api->total_queries;
    }

    if (api->cache_hits + api->cache_misses > 0) {
        stats.cache_hit_rate =
            (double)api->cache_hits / (double)(api->cache_hits + api->cache_misses);
    }

    if (api->performance_cache) {
        CacheStatistics cache_stats = semantic_cache_get_statistics(api->performance_cache);
        stats.memory_usage_bytes = cache_stats.memory_usage_bytes;
        stats.cache_entries = cache_stats.current_entries;
    }

    pthread_mutex_unlock(&api->api_mutex);

    return stats;
}

void asthra_ai_reset_performance_stats(AsthraSemanticsAPI *api) {
    if (!api)
        return;

    pthread_mutex_lock(&api->api_mutex);

    api->total_queries = 0;
    api->total_query_time_ms = 0.0;
    api->cache_hits = 0;
    api->cache_misses = 0;

    if (api->performance_cache) {
        semantic_cache_reset_statistics(api->performance_cache);
    }

    pthread_mutex_unlock(&api->api_mutex);
}

bool asthra_ai_configure_cache(AsthraSemanticsAPI *api, const CacheConfig *config) {
    if (!api || !config)
        return false;

    pthread_mutex_lock(&api->api_mutex);

    // Destroy old cache
    if (api->performance_cache) {
        semantic_cache_destroy(api->performance_cache);
    }

    // Create new cache with new configuration
    api->performance_cache = semantic_cache_create(config);
    bool success = (api->performance_cache != NULL);

    pthread_mutex_unlock(&api->api_mutex);

    return success;
}

void asthra_ai_clear_cache(AsthraSemanticsAPI *api) {
    if (!api)
        return;

    pthread_mutex_lock(&api->api_mutex);

    if (api->performance_cache) {
        semantic_cache_clear(api->performance_cache);
    }

    pthread_mutex_unlock(&api->api_mutex);
}
