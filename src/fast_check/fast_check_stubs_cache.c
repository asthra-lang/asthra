// This file provides stub implementations for cache management functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "fast_semantic_cache.h"
#include <stdlib.h>
#include <pthread.h>

// =============================================================================
// CACHE MANAGEMENT
// =============================================================================

FastSemanticCacheManager* fast_semantic_cache_create(size_t max_memory_mb) {
    FastSemanticCacheManager *cache = calloc(1, sizeof(FastSemanticCacheManager));
    if (!cache) return NULL;
    
    cache->max_memory_usage = max_memory_mb * 1024 * 1024; // Convert MB to bytes
    cache->current_memory_usage = 0;
    cache->total_file_checks = 0;
    cache->cache_hits = 0;
    cache->cache_misses = 0;
    cache->total_analysis_time = 0.0;
    pthread_rwlock_init(&cache->fast_cache_lock, NULL);
    return cache;
}

void fast_semantic_cache_destroy(FastSemanticCacheManager *cache) {
    if (cache) {
        pthread_rwlock_destroy(&cache->fast_cache_lock);
        free(cache);
    }
}

FastCacheStatistics fast_semantic_cache_get_statistics(FastSemanticCacheManager *cache) {
    FastCacheStatistics stats = {0};
    if (!cache) {
        return stats;
    }
    
    stats.total_files_cached = cache->file_entry_count;
    stats.cache_hits = cache->cache_hits;
    stats.cache_misses = cache->cache_misses;
    stats.memory_usage_bytes = cache->current_memory_usage;
    
    if (cache->cache_hits + cache->cache_misses > 0) {
        stats.hit_rate = (double)cache->cache_hits / (cache->cache_hits + cache->cache_misses);
    }
    
    if (cache->total_file_checks > 0) {
        stats.average_analysis_time_ms = cache->total_analysis_time / cache->total_file_checks;
        stats.cache_efficiency = stats.hit_rate;
    }
    
    return stats;
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_cache_dummy = 0;

#endif // FAST_CHECK_USE_STUBS