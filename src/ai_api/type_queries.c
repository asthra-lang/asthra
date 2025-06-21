#include "type_queries.h"
#include "../analysis/semantic_analyzer.h"
#include "../parser/parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/time.h>

// Helper function to get current time in milliseconds
static double get_current_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec * 1000.0 + (double)tv.tv_usec / 1000.0;
}


// =============================================================================
// TYPE INFERENCE IMPLEMENTATION
// =============================================================================

char *asthra_ai_infer_expression_type(AsthraSemanticsAPI *api, const char *expression) {
    if (!api || !expression || !api->is_initialized) {
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
    snprintf(cache_key, sizeof(cache_key), "expr_type_%s", expression);
    
    bool cache_hit = false;
    if (api->performance_cache && 
        semantic_cache_get(api->performance_cache, cache_key, &cached_data, &cached_size)) {
        // Cache hit - return cached data
        if (cached_data) {
            api->cache_hits++;
            cache_hit = true;
            
            char *cached_result = strdup((char*)cached_data);
            
            double end_time = get_current_time_ms();
            api->total_query_time_ms += (end_time - start_time);
            
            pthread_mutex_unlock(&api->api_mutex);
            free(cached_data);
            return cached_result;
        }
        free(cached_data);
    }
    
    // Cache miss - perform actual lookup
    if (!cache_hit) {
        api->cache_misses++;
    }
    
    // For now, implement basic literal type inference
    // This will be enhanced when semantic analyzer supports expression parsing
    
    char *result = NULL;
    
    // Integer literal
    if (strspn(expression, "0123456789") == strlen(expression)) {
        result = strdup("i32");
    }
    // Float literal
    else if (strchr(expression, '.') != NULL && 
             strspn(expression, "0123456789.") == strlen(expression)) {
        result = strdup("f64");
    }
    // String literal
    else if (expression[0] == '"' && expression[strlen(expression)-1] == '"') {
        result = strdup("string");
    }
    // Boolean literal
    else if (strcmp(expression, "true") == 0 || strcmp(expression, "false") == 0) {
        result = strdup("bool");
    }
    // Default fallback
    else {
        // Try to use semantic analyzer for complex expressions
        // For now, return unknown for unsupported expressions
        result = strdup("unknown");
    }
    
    // Cache the result for future use
    if (api->performance_cache && result && !cache_hit) {
        semantic_cache_put(api->performance_cache, cache_key, result, strlen(result) + 1);
    }
    
    double end_time = get_current_time_ms();
    api->total_query_time_ms += (end_time - start_time);
    
    pthread_mutex_unlock(&api->api_mutex);
    return result;
}

bool asthra_ai_check_type_compatibility(AsthraSemanticsAPI *api, const char *expected, const char *actual) {
    if (!api || !expected || !actual || !api->is_initialized) {
        return false;
    }
    
    double start_time = get_current_time_ms();
    pthread_mutex_lock(&api->api_mutex);
    
    // Update performance counters
    api->total_queries++;
    
    // Check cache first
    void *cached_data = NULL;
    size_t cached_size = 0;
    char cache_key[256];
    snprintf(cache_key, sizeof(cache_key), "type_compat_%s_%s", expected, actual);
    
    bool cache_hit = false;
    if (api->performance_cache && 
        semantic_cache_get(api->performance_cache, cache_key, &cached_data, &cached_size)) {
        // Cache hit - return cached data
        if (cached_data && cached_size == sizeof(bool)) {
            api->cache_hits++;
            cache_hit = true;
            
            bool cached_result = *((bool*)cached_data);
            
            double end_time = get_current_time_ms();
            api->total_query_time_ms += (end_time - start_time);
            
            pthread_mutex_unlock(&api->api_mutex);
            free(cached_data);
            return cached_result;
        }
        free(cached_data);
    }
    
    // Cache miss - perform actual lookup
    if (!cache_hit) {
        api->cache_misses++;
    }
    
    bool compatible = false;
    
    // Exact match
    if (strcmp(expected, actual) == 0) {
        compatible = true;
    }
    // Integer type promotion
    else if (strcmp(expected, "i32") == 0 && strcmp(actual, "i16") == 0) {
        compatible = true;
    }
    else if (strcmp(expected, "i64") == 0 && (strcmp(actual, "i32") == 0 || strcmp(actual, "i16") == 0)) {
        compatible = true;
    }
    // Float type promotion
    else if (strcmp(expected, "f64") == 0 && strcmp(actual, "f32") == 0) {
        compatible = true;
    }
    
    // Cache the result for future use
    if (api->performance_cache && !cache_hit) {
        semantic_cache_put(api->performance_cache, cache_key, &compatible, sizeof(bool));
    }
    
    double end_time = get_current_time_ms();
    api->total_query_time_ms += (end_time - start_time);
    
    pthread_mutex_unlock(&api->api_mutex);
    return compatible;
}

char **asthra_ai_get_compatible_types(AsthraSemanticsAPI *api, const char *context, size_t *count) {
    if (!api || !context || !count || !api->is_initialized) {
        if (count) *count = 0;
        return NULL;
    }
    
    double start_time = get_current_time_ms();
    pthread_mutex_lock(&api->api_mutex);
    
    // Update performance counters
    api->total_queries++;
    api->cache_misses++; // For now, don't cache complex arrays
    
    // Return basic types for now
    const char *basic_types[] = {"i32", "i64", "f32", "f64", "string", "bool", "void"};
    size_t basic_count = sizeof(basic_types) / sizeof(basic_types[0]);
    
    char **result = malloc(sizeof(char*) * basic_count);
    if (!result) {
        *count = 0;
        double end_time = get_current_time_ms();
        api->total_query_time_ms += (end_time - start_time);
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    for (size_t i = 0; i < basic_count; i++) {
        result[i] = strdup(basic_types[i]);
        if (!result[i]) {
            // Cleanup on failure
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *count = 0;
            double end_time = get_current_time_ms();
            api->total_query_time_ms += (end_time - start_time);
            pthread_mutex_unlock(&api->api_mutex);
            return NULL;
        }
    }
    
    *count = basic_count;
    
    double end_time = get_current_time_ms();
    api->total_query_time_ms += (end_time - start_time);
    
    pthread_mutex_unlock(&api->api_mutex);
    return result;
}

// =============================================================================
// TYPE INFORMATION QUERIES
// =============================================================================

AITypeInfo *asthra_ai_get_type_info(AsthraSemanticsAPI *api, const char *type_name) {
    if (!api || !type_name || !api->is_initialized) {
        return NULL;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    // For now, return basic type information
    // This will be enhanced when semantic analyzer provides detailed type info
    
    AITypeInfo *info = malloc(sizeof(AITypeInfo));
    if (!info) {
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    memset(info, 0, sizeof(AITypeInfo));
    
    info->type_name = strdup(type_name);
    
    // Determine category based on type name
    if (strcmp(type_name, "i32") == 0 || strcmp(type_name, "i64") == 0 ||
        strcmp(type_name, "f32") == 0 || strcmp(type_name, "f64") == 0 ||
        strcmp(type_name, "bool") == 0 || strcmp(type_name, "string") == 0) {
        info->category = strdup("primitive");
        info->is_ffi_compatible = true;
        info->is_copy_type = true;
    } else {
        info->category = strdup("struct");
        info->is_ffi_compatible = false;
        info->is_copy_type = false;
    }
    
    info->is_mutable = false;
    info->is_generic = false;
    info->generic_params = NULL;
    info->generic_param_count = 0;
    
    pthread_mutex_unlock(&api->api_mutex);
    return info;
}

char **asthra_ai_get_available_types(AsthraSemanticsAPI *api, size_t *count) {
    if (!api || !count || !api->is_initialized) {
        if (count) *count = 0;
        return NULL;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    // Return basic types plus void
    const char *available_types[] = {"i32", "i64", "i16", "i8", "f32", "f64", "string", "bool", "void"};
    size_t type_count = sizeof(available_types) / sizeof(available_types[0]);
    
    char **result = malloc(sizeof(char*) * type_count);
    if (!result) {
        *count = 0;
        pthread_mutex_unlock(&api->api_mutex);
        return NULL;
    }
    
    for (size_t i = 0; i < type_count; i++) {
        result[i] = strdup(available_types[i]);
        if (!result[i]) {
            // Cleanup on failure
            for (size_t j = 0; j < i; j++) {
                free(result[j]);
            }
            free(result);
            *count = 0;
            pthread_mutex_unlock(&api->api_mutex);
            return NULL;
        }
    }
    
    *count = type_count;
    pthread_mutex_unlock(&api->api_mutex);
    return result;
}

bool asthra_ai_is_primitive_type(AsthraSemanticsAPI *api, const char *type_name) {
    if (!api || !type_name || !api->is_initialized) {
        return false;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    const char *primitive_types[] = {"i32", "i64", "i16", "i8", "f32", "f64", "string", "bool"};
    size_t primitive_count = sizeof(primitive_types) / sizeof(primitive_types[0]);
    
    bool is_primitive = false;
    for (size_t i = 0; i < primitive_count; i++) {
        if (strcmp(type_name, primitive_types[i]) == 0) {
            is_primitive = true;
            break;
        }
    }
    
    pthread_mutex_unlock(&api->api_mutex);
    return is_primitive;
}

char *asthra_ai_resolve_type_alias(AsthraSemanticsAPI *api, const char *alias_name) {
    if (!api || !alias_name || !api->is_initialized) {
        return NULL;
    }
    
    pthread_mutex_lock(&api->api_mutex);
    
    // For now, no type aliases are supported
    // This will be enhanced when semantic analyzer supports type aliases
    char *result = NULL;
    
    pthread_mutex_unlock(&api->api_mutex);
    return result;
}

// =============================================================================
// MEMORY MANAGEMENT
// =============================================================================

void asthra_ai_free_type_info(AITypeInfo *info) {
    if (!info) return;
    
    free(info->type_name);
    free(info->category);
    
    if (info->generic_params) {
        for (size_t i = 0; i < info->generic_param_count; i++) {
            free(info->generic_params[i]);
        }
        free(info->generic_params);
    }
    
    free(info);
} 
