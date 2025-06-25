#ifndef FAST_CHECK_ENGINE_H
#define FAST_CHECK_ENGINE_H

#include "dependency_tracker.h"
#include "fast_semantic_cache.h"
#include "performance_profiler.h"
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

// Forward declarations
typedef struct FastCheckEngine FastCheckEngine;
typedef struct FastCheckConfig FastCheckConfig;
typedef struct FastCheckResult FastCheckResult;
typedef struct IncrementalAnalyzer IncrementalAnalyzer;

// Note: EnhancedDiagnostic, SemanticAnalysisCache are now defined in fast_semantic_cache.h

// Engine stats structure
typedef struct {
    size_t total_checks;
    size_t cache_hits;
    size_t cache_misses;
    double hit_rate;
    double average_check_time_ms;
    size_t memory_usage_bytes;
    size_t files_cached;
} FastCheckEngineStats;

// FastCacheStatistics is now defined in fast_semantic_cache.h
// PerformanceTimer and PerformanceProfile are defined in performance_profiler.h

// Stub structures
struct FastCheckEngine {
    FastCheckConfig *config;
    DependencyGraph *dependency_graph;
    FastSemanticCacheManager *semantic_cache;
    void *analyzer;
    FastCheckEngineStats stats;

    // Statistics fields used by fast_check_command.c
    size_t total_checks;
    double total_check_time;
    size_t cache_hits;
    size_t cache_misses;

    // Threading support
    pthread_mutex_t engine_lock;

    PerformanceProfile *profiler;
};

struct FastCheckConfig {
    size_t max_cache_memory_mb;
    time_t cache_ttl_seconds;
    bool enable_dependency_tracking;
    bool enable_incremental_parsing;
    bool enable_cache;
    bool watch_mode;
    bool syntax_only;
    uint64_t timeout_ms;
    bool clear_cache;
    char *output_format;
    char **include_patterns;
    char **exclude_patterns;
    size_t include_count;
    size_t exclude_count;
    bool enable_parallel_analysis;
    uint64_t max_analysis_time_ms;
    size_t max_files_per_batch;
    int max_errors;
};

struct FastCheckResult {
    bool success;
    double check_time_ms;
    int error_count;
    int warning_count;
    bool was_cached;

    // Additional fields used by engine implementation
    char *file_path;
    EnhancedDiagnostic *errors;
    EnhancedDiagnostic *warnings;
    EnhancedDiagnostic *suggestions;
    int suggestion_count;
    size_t memory_used_bytes;
    size_t files_analyzed;
    size_t cache_hits;
};

// FastSemanticCacheManager is now defined in fast_semantic_cache.h

// IncrementalAnalyzer structure
struct IncrementalAnalyzer {
    void *analyzer; // Semantic analyzer
    FastSemanticCacheManager *cache;
    DependencyGraph *dependencies;
    bool has_global_context;
    void *global_symbols;
    void *builtin_types;
    size_t builtin_type_count;
};

// Engine functions (stubs)
FastCheckEngine *fast_check_engine_create(void);
FastCheckEngine *fast_check_engine_create_with_config(FastCheckConfig *config);
void fast_check_engine_destroy(FastCheckEngine *engine);
FastCheckEngineStats fast_check_engine_get_stats(FastCheckEngine *engine);
void fast_check_engine_clear_cache(FastCheckEngine *engine);

// Config functions (stubs)
FastCheckConfig *fast_check_config_create_default(void);
void fast_check_config_destroy(FastCheckConfig *config);

// Result functions (stubs)
FastCheckResult *fast_check_file(FastCheckEngine *engine, const char *filepath);
FastCheckResult *fast_check_code_snippet(FastCheckEngine *engine, const char *code,
                                         const char *context);
void fast_check_result_destroy(FastCheckResult *result);
bool fast_check_invalidate_cache(FastCheckEngine *engine, const char *filepath);

// Dependency graph functions (stubs)
DependencyGraph *dependency_graph_create(void);
void dependency_graph_destroy(DependencyGraph *graph);
bool dependency_graph_add_file(DependencyGraph *graph, const char *filepath);
bool dependency_graph_add_dependency(DependencyGraph *graph, const char *from, const char *to);

// Cache functions (stubs)
FastSemanticCacheManager *fast_semantic_cache_create(size_t max_memory_mb);
void fast_semantic_cache_destroy(FastSemanticCacheManager *cache);
FastCacheStatistics fast_semantic_cache_get_statistics(FastSemanticCacheManager *cache);

// Performance profiler functions are declared in performance_profiler.h

// Additional engine functions used by implementation
FastCheckResult *fast_check_result_create(void);
IncrementalAnalyzer *incremental_analyzer_create(void);
void incremental_analyzer_destroy(IncrementalAnalyzer *analyzer);
FastCheckResult *incremental_analyzer_check_file(IncrementalAnalyzer *analyzer,
                                                 const char *file_path);
FastCheckResult *incremental_analyzer_check_snippet(IncrementalAnalyzer *analyzer, const char *code,
                                                    const char *context_file);

// Semantic cache functions used by implementation
SemanticAnalysisCache *fast_semantic_cache_get_file(FastSemanticCacheManager *cache,
                                                    const char *file_path);
void fast_semantic_cache_invalidate_file(FastSemanticCacheManager *cache, const char *file_path);
void fast_semantic_cache_reset_statistics(FastSemanticCacheManager *cache);

// Forward declaration for SemanticAnalyzer
typedef struct SemanticAnalyzer SemanticAnalyzer;

// Semantic analyzer functions used by implementation
SemanticAnalyzer *semantic_analyzer_create(void);
void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);

#endif // FAST_CHECK_ENGINE_H
