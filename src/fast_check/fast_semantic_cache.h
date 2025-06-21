#ifndef FAST_SEMANTIC_CACHE_H
#define FAST_SEMANTIC_CACHE_H

#include <stdbool.h>
#include <stddef.h>
#include <time.h>
#include <pthread.h>

// Forward declarations
typedef struct FastSemanticCacheManager FastSemanticCacheManager;
typedef struct SemanticAnalysisCache SemanticAnalysisCache;
typedef struct EnhancedDiagnostic EnhancedDiagnostic;
typedef struct CacheConfig CacheConfig;

// Include the actual type definitions to avoid conflicts
#ifndef SEMANTIC_TYPES_DEFS_H
#include "../analysis/semantic_types_defs.h"
#endif

#ifndef SEMANTIC_SYMBOLS_DEFS_H  
#include "../analysis/semantic_symbols_defs.h"
#endif

// Diagnostic level enum
typedef enum {
    DIAGNOSTIC_ERROR,
    DIAGNOSTIC_WARNING,
    DIAGNOSTIC_INFO,
    DIAGNOSTIC_HINT
} DiagnosticLevel;

// Enhanced diagnostic structure
struct EnhancedDiagnostic {
    DiagnosticLevel level;
    char *message;
    char *code;
    void *spans;
    size_t span_count;
    void *suggestions;
    size_t suggestion_count;
    void *metadata;
    void *related_info;
    size_t related_count;
};

// Semantic analysis cache structure
struct SemanticAnalysisCache {
    bool is_valid;
    size_t memory_used_bytes;
    double analysis_time_ms;
    EnhancedDiagnostic *diagnostics;
    size_t diagnostic_count;
    time_t cache_timestamp;
    
    // Additional fields used by cache implementation
    void *symbols;
    void *types;
    size_t type_count;
    size_t file_content_hash;
};

// Cache statistics structure
typedef struct {
    size_t total_files_cached;
    size_t cache_hits;
    size_t cache_misses;
    size_t memory_usage_bytes;
    double hit_rate;
    double cache_efficiency;
    double average_analysis_time_ms;
} FastCacheStatistics;

// Cache manager structure
struct FastSemanticCacheManager {
    void *base_cache; // Underlying semantic cache
    SemanticAnalysisCache **file_entries;
    size_t file_entry_count;
    size_t file_capacity;
    size_t max_memory_usage;
    size_t current_memory_usage;
    time_t default_ttl;
    
    // Statistics
    size_t total_file_checks;
    size_t cache_hits;
    size_t cache_misses;
    double total_analysis_time;
    
    // Thread safety
    pthread_rwlock_t fast_cache_lock;
};

// Stub cache config structure
struct CacheConfig {
    size_t max_memory_mb;
};

// Note: SymbolTable and TypeDescriptor are forward declared above
// and defined in semantic_symbols_defs.h and semantic_types_defs.h respectively

// Function declarations
FastSemanticCacheManager* fast_semantic_cache_create(size_t max_memory_mb);
void fast_semantic_cache_destroy(FastSemanticCacheManager *cache);
SemanticAnalysisCache* fast_semantic_cache_get_file(FastSemanticCacheManager *cache, const char *file_path);
bool fast_semantic_cache_store_file(FastSemanticCacheManager *cache, const char *file_path, SemanticAnalysisCache *result);
void fast_semantic_cache_invalidate_file(FastSemanticCacheManager *cache, const char *file_path);
FastCacheStatistics fast_semantic_cache_get_statistics(FastSemanticCacheManager *cache);
void fast_semantic_cache_reset_statistics(FastSemanticCacheManager *cache);
void fast_semantic_cache_cleanup_expired(FastSemanticCacheManager *cache);

// Semantic analysis cache functions
SemanticAnalysisCache* semantic_analysis_cache_create(void);
void semantic_analysis_cache_destroy(SemanticAnalysisCache *cache);

// Cache utility functions
size_t fast_semantic_cache_calculate_file_hash(const char *file_path);
CacheConfig semantic_cache_default_config(void);
void* semantic_cache_create(CacheConfig *config);
void semantic_cache_destroy(void *cache);

#endif // FAST_SEMANTIC_CACHE_H 
