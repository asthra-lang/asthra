#include "fast_check_engine.h"
#include "dependency_tracker.h"
#include "fast_semantic_cache.h"
#include "performance_profiler.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h> // For usleep

// =============================================================================
// STUB IMPLEMENTATIONS FOR MISSING FUNCTIONS
// =============================================================================

// Stub implementation for performance_profiler_reset_file_stats
static void performance_profiler_reset_file_stats(PerformanceProfile *profiler) {
    (void)profiler;
    // TODO: Implement performance profiler file stats reset
}

// =============================================================================
// ORIGINAL IMPLEMENTATION
// =============================================================================

// Create default configuration
FastCheckConfig *fast_check_config_create_default(void) {
    FastCheckConfig *config = malloc(sizeof(FastCheckConfig));
    if (!config)
        return NULL;

    config->max_cache_memory_mb = 256; // 256MB cache
    config->cache_ttl_seconds = 300;   // 5 minutes
    config->enable_dependency_tracking = true;
    config->enable_incremental_parsing = true;
    config->enable_parallel_analysis = false; // Disabled for now
    config->max_analysis_time_ms = 5000;      // 5 seconds max
    config->max_files_per_batch = 10;

    return config;
}

void fast_check_config_destroy(FastCheckConfig *config) {
    free(config);
}

// Create fast check engine with provided config
FastCheckEngine *fast_check_engine_create_with_config(FastCheckConfig *config) {
    if (!config)
        return NULL;

    FastCheckEngine *engine = calloc(1, sizeof(FastCheckEngine));
    if (!engine) {
        return NULL;
    }

    engine->config = config;

    engine->semantic_cache = fast_semantic_cache_create(config->max_cache_memory_mb);
    if (!engine->semantic_cache) {
        free(engine);
        return NULL;
    }

    if (pthread_mutex_init(&engine->engine_lock, NULL) != 0) {
        fast_semantic_cache_destroy(engine->semantic_cache);
        free(engine);
        return NULL;
    }

    // Initialize performance profiler
    engine->profiler = performance_profiler_create();
    if (!engine->profiler) {
        pthread_mutex_destroy(&engine->engine_lock);
        fast_semantic_cache_destroy(engine->semantic_cache);
        free(engine);
        return NULL;
    }

    return engine;
}

// Create fast check engine
FastCheckEngine *fast_check_engine_create(void) {
    FastCheckConfig *config = fast_check_config_create_default();
    if (!config)
        return NULL;

    FastCheckEngine *engine = fast_check_engine_create_with_config(config);
    if (!engine) {
        fast_check_config_destroy(config);
        return NULL;
    }

    return engine;
}

void fast_check_engine_destroy(FastCheckEngine *engine) {
    if (!engine)
        return;

    pthread_mutex_destroy(&engine->engine_lock);
    fast_semantic_cache_destroy(engine->semantic_cache);
    performance_profiler_destroy(engine->profiler); // Destroy profiler
    fast_check_config_destroy(engine->config);
    free(engine);
}

// Create fast check result
FastCheckResult *fast_check_result_create(void) {
    FastCheckResult *result = malloc(sizeof(FastCheckResult));
    if (!result)
        return NULL;

    result->success = false;
    result->file_path = NULL;
    result->errors = NULL;
    result->error_count = 0;
    result->warnings = NULL;
    result->warning_count = 0;
    result->suggestions = NULL;
    result->suggestion_count = 0;
    result->check_time_ms = 0.0;
    result->was_cached = false;
    result->memory_used_bytes = 0;
    result->files_analyzed = 0;
    result->cache_hits = 0;

    return result;
}

void fast_check_result_destroy(FastCheckResult *result) {
    if (!result)
        return;

    free(result->file_path);
    free(result->errors);
    free(result->warnings);
    free(result->suggestions);
    free(result);
}

// Create error result
static FastCheckResult *create_error_result(const char *error_message, const char *file_path) {
    FastCheckResult *result = fast_check_result_create();
    if (!result)
        return NULL;

    result->success = false;
    result->file_path = file_path ? strdup(file_path) : NULL;

    // Create a single error diagnostic
    result->errors = malloc(sizeof(EnhancedDiagnostic));
    if (result->errors) {
        result->error_count = 1;
        // Note: This is a simplified error creation
        // In a full implementation, we'd properly initialize EnhancedDiagnostic
        result->errors[0].level = DIAGNOSTIC_ERROR;
        result->errors[0].message = strdup(error_message ? error_message : "Unknown error");
        result->errors[0].code = strdup("FAST_CHECK_ERROR");
        result->errors[0].spans = NULL;
        result->errors[0].span_count = 0;
        result->errors[0].suggestions = NULL;
        result->errors[0].suggestion_count = 0;
        result->errors[0].metadata = NULL;
        result->errors[0].related_info = NULL;
        result->errors[0].related_count = 0;
    }

    return result;
}

// Create result from cache
static FastCheckResult *create_fast_check_result_from_cache(SemanticAnalysisCache *cache) {
    if (!cache)
        return NULL;

    FastCheckResult *result = fast_check_result_create();
    if (!result)
        return NULL;

    result->success = cache->is_valid;
    result->was_cached = true;
    result->memory_used_bytes = cache->memory_used_bytes;
    result->check_time_ms = cache->analysis_time_ms;

    // Copy diagnostics from cache
    if (cache->diagnostics && cache->diagnostic_count > 0) {
        // Note: This is simplified - in a full implementation we'd
        // properly categorize diagnostics into errors/warnings/suggestions
        result->error_count = cache->diagnostic_count;
        result->errors = malloc(sizeof(EnhancedDiagnostic) * cache->diagnostic_count);
        if (result->errors) {
            memcpy(result->errors, cache->diagnostics,
                   sizeof(EnhancedDiagnostic) * cache->diagnostic_count);
        }
    }

    return result;
}

// Core fast check implementation
FastCheckResult *fast_check_file(FastCheckEngine *engine, const char *filepath) {
    if (!engine || !filepath) {
        FastCheckResult *result = calloc(1, sizeof(FastCheckResult));
        // Note: error_message field doesn't exist in FastCheckResult
        return result;
    }

    pthread_mutex_lock(&engine->engine_lock);
    performance_timer_start(&engine->profiler->overall_timer); // Start overall timer
    performance_record_file_start(engine->profiler, filepath); // Record file start

    FastCheckResult *result = calloc(1, sizeof(FastCheckResult));
    if (!result) {
        pthread_mutex_unlock(&engine->engine_lock);
        return NULL;
    }
    result->success = false;
    result->was_cached = false;

    // Try to get from cache
    SemanticAnalysisCache *cached_analysis =
        fast_semantic_cache_get_file(engine->semantic_cache, filepath);
    if (cached_analysis) {
        result->success = cached_analysis->is_valid;
        result->was_cached = true;
        result->check_time_ms = cached_analysis->analysis_time_ms;             // Use cached time
        performance_record_cache_hit(engine->profiler, result->check_time_ms); // Record cache hit
        // In a real scenario, you'd return the analysis results here
        // Note: fast_semantic_cache_release_file is not declared, so we'll skip it
        // fast_semantic_cache_release_file(engine->semantic_cache, cached_analysis); // Release
        // reference
    } else {
        performance_record_cache_miss(engine->profiler, 0.0); // Record cache miss

        // Simulate work (replace with actual parsing/analysis)
        usleep(50000); // Simulate 50ms work

        // Simulate result
        result->success = true;
        result->check_time_ms = 50.0; // Simulated time

        // Store result in cache (simulated)
        SemanticAnalysisCache *new_cache_entry = semantic_analysis_cache_create();
        if (new_cache_entry) {
            new_cache_entry->is_valid = true;
            new_cache_entry->analysis_time_ms = result->check_time_ms;
            // Simulate some memory usage
            new_cache_entry->memory_used_bytes = 1024 * 10; // 10KB
            fast_semantic_cache_store_file(engine->semantic_cache, filepath, new_cache_entry);
        }
    }

    performance_timer_stop(&engine->profiler->overall_timer); // Stop overall timer
    performance_record_file_complete(engine->profiler, filepath, 100, 50,
                                     result->check_time_ms); // Simulate lines/symbols
    performance_update_peak_memory(engine->profiler);        // Update peak memory

    pthread_mutex_unlock(&engine->engine_lock);
    return result;
}

// Code snippet checking
FastCheckResult *fast_check_code_snippet(FastCheckEngine *engine, const char *code_snippet,
                                         const char *file_path) {
    if (!engine || !code_snippet) {
        FastCheckResult *result = calloc(1, sizeof(FastCheckResult));
        // Note: error_message field doesn't exist in FastCheckResult
        return result;
    }

    pthread_mutex_lock(&engine->engine_lock);
    performance_timer_start(&engine->profiler->overall_timer); // Start overall timer
    performance_record_file_start(engine->profiler,
                                  file_path ? file_path : "<snippet>"); // Record snippet start

    FastCheckResult *result = calloc(1, sizeof(FastCheckResult));
    if (!result) {
        pthread_mutex_unlock(&engine->engine_lock);
        return NULL;
    }
    result->success = false;
    result->was_cached = false;

    // For code snippets, we generally don't cache unless explicitly configured.
    // Simulate work
    usleep(20000); // Simulate 20ms work for snippet

    result->success = true;
    result->check_time_ms = 20.0; // Simulated time

    performance_timer_stop(&engine->profiler->overall_timer); // Stop overall timer
    performance_record_file_complete(engine->profiler, file_path ? file_path : "<snippet>",
                                     (uint32_t)(strlen(code_snippet) / 20),
                                     (uint32_t)(strlen(code_snippet) / 10),
                                     result->check_time_ms); // Simulate lines/symbols
    performance_update_peak_memory(engine->profiler);        // Update peak memory

    pthread_mutex_unlock(&engine->engine_lock);
    return result;
}

// Cache invalidation
bool fast_check_invalidate_cache(FastCheckEngine *engine, const char *filepath) {
    if (!engine)
        return false;

    pthread_mutex_lock(&engine->engine_lock);
    // Note: fast_semantic_cache_invalidate_file returns void, not bool
    fast_semantic_cache_invalidate_file(engine->semantic_cache, filepath);
    pthread_mutex_unlock(&engine->engine_lock);
    return true; // Assume success
}

// Create incremental analyzer
IncrementalAnalyzer *incremental_analyzer_create(void) {
    IncrementalAnalyzer *analyzer = malloc(sizeof(IncrementalAnalyzer));
    if (!analyzer)
        return NULL;

    // Create semantic analyzer
    analyzer->analyzer = semantic_analyzer_create();
    if (!analyzer->analyzer) {
        free(analyzer);
        return NULL;
    }

    // Create fast cache manager
    analyzer->cache = fast_semantic_cache_create(128); // 128MB for incremental cache
    if (!analyzer->cache) {
        semantic_analyzer_destroy(analyzer->analyzer);
        free(analyzer);
        return NULL;
    }

    // Create dependency graph
    analyzer->dependencies = dependency_graph_create();
    if (!analyzer->dependencies) {
        fast_semantic_cache_destroy(analyzer->cache);
        semantic_analyzer_destroy(analyzer->analyzer);
        free(analyzer);
        return NULL;
    }

    // Initialize state
    analyzer->has_global_context = false;
    analyzer->global_symbols = NULL;
    analyzer->builtin_types = NULL;
    analyzer->builtin_type_count = 0;

    return analyzer;
}

void incremental_analyzer_destroy(IncrementalAnalyzer *analyzer) {
    if (!analyzer)
        return;

    dependency_graph_destroy(analyzer->dependencies);
    fast_semantic_cache_destroy(analyzer->cache);
    semantic_analyzer_destroy(analyzer->analyzer);

    free(analyzer);
}

// Simplified incremental analysis implementation
FastCheckResult *incremental_analyzer_check_file(IncrementalAnalyzer *analyzer,
                                                 const char *file_path) {
    if (!analyzer || !file_path)
        return NULL;

    // For now, this is a simplified implementation
    // In a full implementation, this would:
    // 1. Parse the file if needed
    // 2. Perform semantic analysis
    // 3. Cache the results
    // 4. Return diagnostics

    FastCheckResult *result = fast_check_result_create();
    if (!result)
        return NULL;

    result->success = true;
    result->file_path = strdup(file_path);
    result->files_analyzed = 1;

    return result;
}

FastCheckResult *incremental_analyzer_check_snippet(IncrementalAnalyzer *analyzer, const char *code,
                                                    const char *context_file) {
    if (!analyzer || !code)
        return NULL;

    // Simplified implementation for code snippets
    FastCheckResult *result = fast_check_result_create();
    if (!result)
        return NULL;

    result->success = true;
    result->file_path = context_file ? strdup(context_file) : strdup("<snippet>");
    result->files_analyzed = 1;

    return result;
}

// Engine statistics
FastCheckEngineStats fast_check_engine_get_stats(FastCheckEngine *engine) {
    FastCheckEngineStats stats = {0};

    if (!engine)
        return stats;

    pthread_mutex_lock(&engine->engine_lock);

    // Retrieve statistics from the profiler
    PerformanceProfile *profile = engine->profiler;
    if (profile) {
        // Use mutex for profiler access if needed, or assume it's handled internally
        // (performance_profiler functions already handle their own mutexes)
        stats.total_checks = profile->file_stats.files_processed;
        stats.cache_hits = profile->cache_stats.cache_hits;
        stats.cache_misses = profile->cache_stats.cache_misses;
        stats.hit_rate = (profile->cache_stats.total_requests > 0)
                             ? (double)profile->cache_stats.cache_hits /
                                   (double)profile->cache_stats.total_requests
                             : 0.0;
        stats.average_check_time_ms = profile->file_stats.average_file_time_ms;
        stats.memory_usage_bytes = profile->memory_stats.peak_memory_bytes;
        // For files cached, we need to adapt from semantic_cache's count
        FastCacheStatistics cache_stats =
            fast_semantic_cache_get_statistics(engine->semantic_cache);
        stats.files_cached = cache_stats.total_files_cached;
    }

    pthread_mutex_unlock(&engine->engine_lock);

    return stats;
}

void fast_check_engine_reset_stats(FastCheckEngine *engine) {
    if (!engine)
        return;

    pthread_mutex_lock(&engine->engine_lock);

    // Reset internal engine stats (no longer explicitly tracked here, rely on profiler)
    engine->total_checks = 0;
    engine->cache_hits = 0;
    engine->cache_misses = 0;
    engine->total_check_time = 0.0;

    // Reset semantic cache stats
    fast_semantic_cache_reset_statistics(engine->semantic_cache);

    // Reset profiler stats
    performance_profiler_reset(engine->profiler);

    pthread_mutex_unlock(&engine->engine_lock);
}

// Utility functions
void fast_check_engine_clear_cache(FastCheckEngine *engine) {
    if (!engine)
        return;

    pthread_mutex_lock(&engine->engine_lock);

    // Clear the semantic cache and recreate it
    fast_semantic_cache_destroy(engine->semantic_cache);
    engine->semantic_cache = fast_semantic_cache_create(engine->config->max_cache_memory_mb);
    // Reset profiler file stats as well since cache is cleared
    performance_profiler_reset_file_stats(engine->profiler);

    pthread_mutex_unlock(&engine->engine_lock);
}

size_t fast_check_engine_get_cache_size(FastCheckEngine *engine) {
    if (!engine)
        return 0;

    pthread_mutex_lock(&engine->engine_lock);

    FastCacheStatistics stats = fast_semantic_cache_get_statistics(engine->semantic_cache);
    size_t size = stats.memory_usage_bytes;

    pthread_mutex_unlock(&engine->engine_lock);

    return size;
}
