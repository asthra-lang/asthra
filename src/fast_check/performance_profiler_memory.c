/*
 * Asthra Fast Check Performance Profiler Memory Implementation
 * Memory tracking and management functions
 */

#include "performance_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// =============================================================================
// Memory Tracking
// =============================================================================

void performance_track_memory_allocation(PerformanceProfile *profile, size_t bytes) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->memory_stats.current_memory_bytes += bytes;
    profile->memory_stats.allocated_objects++;
    
    if (profile->memory_stats.current_memory_bytes > profile->memory_stats.peak_memory_bytes) {
        profile->memory_stats.peak_memory_bytes = profile->memory_stats.current_memory_bytes;
    }
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_track_memory_deallocation(PerformanceProfile *profile, size_t bytes) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    if (profile->memory_stats.current_memory_bytes >= bytes) {
        profile->memory_stats.current_memory_bytes -= bytes;
    }
    profile->memory_stats.deallocated_objects++;
    
    // Calculate potential memory leaks
    profile->memory_stats.memory_leaks = 
        profile->memory_stats.allocated_objects - profile->memory_stats.deallocated_objects;
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_update_peak_memory(PerformanceProfile *profile) {
    if (!profile) return;
    
    size_t current_kb = get_memory_usage_kb();
    size_t current_bytes = current_kb * 1024;
    
    pthread_mutex_lock(&profile->stats_mutex);
    if (current_bytes > profile->memory_stats.peak_memory_bytes) {
        profile->memory_stats.peak_memory_bytes = current_bytes;
    }
    profile->memory_stats.current_memory_bytes = current_bytes;
    pthread_mutex_unlock(&profile->stats_mutex);
}