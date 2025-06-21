/*
 * Asthra Fast Check Performance Profiler Parallel Implementation
 * Parallel processing metrics tracking
 */

#include "performance_profiler.h"
#include <pthread.h>

// =============================================================================
// Parallel Processing Metrics
// =============================================================================

void performance_record_thread_start(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->parallel_stats.active_threads++;
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_record_thread_complete(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    if (profile->parallel_stats.active_threads > 0) {
        profile->parallel_stats.active_threads--;
    }
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_record_task_queued(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->parallel_stats.tasks_queued++;
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_record_task_completed(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    profile->parallel_stats.tasks_completed++;
    
    // Calculate utilization and efficiency
    if (profile->parallel_stats.thread_pool_size > 0) {
        profile->parallel_stats.thread_utilization_percentage = 
            (double)profile->parallel_stats.active_threads / profile->parallel_stats.thread_pool_size * 100.0;
    }
    
    if (profile->parallel_stats.tasks_queued > 0) {
        profile->parallel_stats.parallel_efficiency = 
            (double)profile->parallel_stats.tasks_completed / (double)profile->parallel_stats.tasks_queued * 100.0;
    }
    
    pthread_mutex_unlock(&profile->stats_mutex);
}