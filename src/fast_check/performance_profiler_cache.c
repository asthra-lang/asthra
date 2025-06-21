/*
 * Asthra Fast Check Performance Profiler Cache Implementation
 * Cache performance metrics tracking
 */

#include "performance_profiler.h"
#include <pthread.h>

// =============================================================================
// Cache Metrics
// =============================================================================

void performance_record_cache_hit(PerformanceProfile *profile, double lookup_time_ms) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->cache_stats.total_requests++;
    profile->cache_stats.cache_hits++;
    
    // Update average lookup time
    double total_time = profile->cache_stats.average_lookup_time_ms * (double)(profile->cache_stats.total_requests - 1);
    profile->cache_stats.average_lookup_time_ms = (total_time + lookup_time_ms) / (double)profile->cache_stats.total_requests;
    
    // Update hit rate
    profile->cache_stats.hit_rate_percentage = 
        (double)profile->cache_stats.cache_hits / (double)profile->cache_stats.total_requests * 100.0;
    
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_record_cache_miss(PerformanceProfile *profile, double lookup_time_ms) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->cache_stats.total_requests++;
    profile->cache_stats.cache_misses++;
    
    // Update average lookup time
    double total_time = profile->cache_stats.average_lookup_time_ms * (double)(profile->cache_stats.total_requests - 1);
    profile->cache_stats.average_lookup_time_ms = (total_time + lookup_time_ms) / (double)profile->cache_stats.total_requests;
    
    // Update hit rate
    profile->cache_stats.hit_rate_percentage = 
        (double)profile->cache_stats.cache_hits / (double)profile->cache_stats.total_requests * 100.0;
    
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_record_cache_eviction(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->cache_stats.cache_evictions++;
    pthread_mutex_unlock(&profile->stats_mutex);
}