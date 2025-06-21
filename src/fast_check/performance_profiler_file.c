/*
 * Asthra Fast Check Performance Profiler File Implementation
 * File processing metrics tracking
 */

#include "performance_profiler.h"
#include <string.h>
#include <pthread.h>

// =============================================================================
// File Processing Metrics
// =============================================================================

void performance_record_file_start(PerformanceProfile *profile, const char *filename) {
    if (!profile || !filename) return;
    
    // Just increment counter - timing is handled elsewhere
    pthread_mutex_lock(&profile->stats_mutex);
    profile->file_stats.files_processed++;
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_record_file_complete(PerformanceProfile *profile, const char *filename, 
                                     uint32_t lines, uint32_t symbols, double duration_ms) {
    if (!profile || !filename) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    
    profile->file_stats.lines_analyzed += lines;
    profile->file_stats.symbols_resolved += symbols;
    profile->file_stats.total_processing_time_ms += duration_ms;
    
    // Update average
    if (profile->file_stats.files_processed > 0) {
        profile->file_stats.average_file_time_ms = 
            profile->file_stats.total_processing_time_ms / profile->file_stats.files_processed;
    }
    
    // Track fastest/slowest files
    if (profile->file_stats.files_processed == 1 || duration_ms < profile->file_stats.fastest_time_ms) {
        strncpy(profile->file_stats.fastest_file, filename, sizeof(profile->file_stats.fastest_file) - 1);
        profile->file_stats.fastest_time_ms = duration_ms;
    }
    
    if (profile->file_stats.files_processed == 1 || duration_ms > profile->file_stats.slowest_time_ms) {
        strncpy(profile->file_stats.slowest_file, filename, sizeof(profile->file_stats.slowest_file) - 1);
        profile->file_stats.slowest_time_ms = duration_ms;
    }
    
    pthread_mutex_unlock(&profile->stats_mutex);
}