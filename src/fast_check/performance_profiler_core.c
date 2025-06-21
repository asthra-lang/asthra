/*
 * Asthra Fast Check Performance Profiler Core Implementation
 * Core profile management and utility functions
 */

#include "performance_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/resource.h>

// =============================================================================
// Internal Utilities
// =============================================================================

double timespec_to_ms(const struct timespec *ts) {
    return (double)ts->tv_sec * 1000.0 + (double)ts->tv_nsec / 1000000.0;
}

double timespec_diff_ms(const struct timespec *start, const struct timespec *end) {
    return timespec_to_ms(end) - timespec_to_ms(start);
}

size_t get_memory_usage_kb(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return (size_t)usage.ru_maxrss;
    }
    return 0;
}

// =============================================================================
// Performance Profile Management
// =============================================================================

PerformanceProfile* performance_profiler_create(void) {
    PerformanceProfile *profile = calloc(1, sizeof(PerformanceProfile));
    if (!profile) return NULL;
    
    // Initialize target metrics (from Phase 5 requirements)
    profile->target_single_file_ms = 100.0;     // < 100ms
    profile->target_medium_project_ms = 500.0;  // < 500ms
    profile->target_large_project_ms = 2000.0;  // < 2s
    
    // Initialize performance flags
    profile->meets_single_file_target = false;
    profile->meets_medium_project_target = false;
    profile->meets_large_project_target = false;
    profile->overall_performance_acceptable = false;
    
    // Initialize historical data
    for (int i = 0; i < 10; i++) {
        profile->historical_total_times[i] = 0.0;
        profile->historical_avg_file_times[i] = 0.0;
        profile->historical_cache_hit_rates[i] = 0.0;
        profile->historical_peak_memory[i] = 0;
    }
    profile->history_count = 0;
    profile->history_index = 0;
    
    // Initialize mutex
    if (pthread_mutex_init(&profile->stats_mutex, NULL) != 0) {
        free(profile);
        return NULL;
    }
    
    // Initialize timers
    profile->overall_timer.is_active = false;
    
    return profile;
}

void performance_profiler_destroy(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_destroy(&profile->stats_mutex);
    free(profile);
}

void performance_profiler_reset(PerformanceProfile *profile) {
    if (!profile) return;
    
    pthread_mutex_lock(&profile->stats_mutex);
    
    // Reset all stats except targets
    memset(&profile->memory_stats, 0, sizeof(MemoryUsageStats));
    memset(&profile->cache_stats, 0, sizeof(CachePerformanceStats));
    memset(&profile->file_stats, 0, sizeof(FileProcessingStats));
    memset(&profile->parallel_stats, 0, sizeof(ParallelProcessingStats));
    
    profile->overall_timer.is_active = false;
    profile->meets_single_file_target = false;
    profile->meets_medium_project_target = false;
    profile->meets_large_project_target = false;
    profile->overall_performance_acceptable = false;
    
    // Reset historical data
    for (int i = 0; i < 10; i++) {
        profile->historical_total_times[i] = 0.0;
        profile->historical_avg_file_times[i] = 0.0;
        profile->historical_cache_hit_rates[i] = 0.0;
        profile->historical_peak_memory[i] = 0;
    }
    profile->history_count = 0;
    profile->history_index = 0;
    
    pthread_mutex_unlock(&profile->stats_mutex);
}

void performance_profiler_record_history(PerformanceProfile *profile) {
    if (!profile) return;

    pthread_mutex_lock(&profile->stats_mutex);

    // Record current performance metrics into the circular buffer
    profile->historical_total_times[profile->history_index] = performance_timer_get_duration_ms(&profile->overall_timer);
    profile->historical_avg_file_times[profile->history_index] = profile->file_stats.average_file_time_ms;
    profile->historical_cache_hit_rates[profile->history_index] = profile->cache_stats.hit_rate_percentage;
    profile->historical_peak_memory[profile->history_index] = profile->memory_stats.peak_memory_bytes;

    profile->history_index = (profile->history_index + 1) % 10;
    if (profile->history_count < 10) {
        profile->history_count++;
    }

    pthread_mutex_unlock(&profile->stats_mutex);
}

// =============================================================================
// Performance Target Validation
// =============================================================================

bool performance_meets_single_file_target(const PerformanceProfile *profile, double actual_time_ms) {
    if (!profile) return false;
    return actual_time_ms <= profile->target_single_file_ms;
}

bool performance_meets_medium_project_target(const PerformanceProfile *profile, double actual_time_ms) {
    if (!profile) return false;
    return actual_time_ms <= profile->target_medium_project_ms;
}

bool performance_meets_large_project_target(const PerformanceProfile *profile, double actual_time_ms) {
    if (!profile) return false;
    return actual_time_ms <= profile->target_large_project_ms;
}

// =============================================================================
// Historical Data Helpers
// =============================================================================

double calculate_historical_average_double(const double history_array[], int count) {
    if (count == 0) return 0.0;
    double sum = 0.0;
    for (int i = 0; i < count; i++) {
        sum += history_array[i];
    }
    return sum / count;
}

size_t calculate_historical_average_size_t(const size_t history_array[], int count) {
    if (count == 0) return 0;
    size_t sum = 0;
    for (int i = 0; i < count; i++) {
        sum += history_array[i];
    }
    return sum / count;
}

// Detect performance regression
bool performance_detect_regression(const PerformanceProfile *profile, BottleneckAnalysis *bottleneck) {
    if (profile->history_count < 3) { // Need at least 3 historical runs to detect a trend
        return false;
    }

    double avg_total_time = calculate_historical_average_double(profile->historical_total_times, profile->history_count);
    double avg_avg_file_time = calculate_historical_average_double(profile->historical_avg_file_times, profile->history_count);
    double avg_cache_hit_rate = calculate_historical_average_double(profile->historical_cache_hit_rates, profile->history_count);
    size_t avg_peak_memory = calculate_historical_average_size_t(profile->historical_peak_memory, profile->history_count);

    // Get current metrics (from the last completed run recorded in the profile)
    double current_total_time = performance_timer_get_duration_ms(&profile->overall_timer);
    double current_avg_file_time = profile->file_stats.average_file_time_ms;
    double current_cache_hit_rate = profile->cache_stats.hit_rate_percentage;
    size_t current_peak_memory = profile->memory_stats.peak_memory_bytes;
    (void)avg_cache_hit_rate; // TODO: Use in cache hit rate comparison
    (void)avg_peak_memory; // TODO: Use in memory comparison
    (void)current_cache_hit_rate; // TODO: Use in cache hit rate comparison
    (void)current_peak_memory; // TODO: Use in memory comparison

    // Define regression thresholds (e.g., 10% degradation)
    const double REGRESSION_THRESHOLD_PERCENT = 10.0; // 10% worse

    bool regressed = false;
    char details[512] = {0};

    if (current_total_time > avg_total_time * (1.0 + REGRESSION_THRESHOLD_PERCENT / 100.0)) {
        snprintf(details + strlen(details), sizeof(details) - strlen(details),
                 "Total time degraded by %.1f%% (%.1fms vs avg %.1fms). ",
                 (current_total_time - avg_total_time) / avg_total_time * 100.0,
                 current_total_time, avg_total_time);
        regressed = true;
    }
    if (current_avg_file_time > avg_avg_file_time * (1.0 + REGRESSION_THRESHOLD_PERCENT / 100.0)) {
        snprintf(details + strlen(details), sizeof(details) - strlen(details),
                 "Average file time degraded by %.1f%% (%.1fms vs avg %.1fms). ",
                 (current_avg_file_time - avg_avg_file_time) / avg_avg_file_time * 100.0,
                 current_avg_file_time, avg_avg_file_time);
        regressed = true;
    }
    
    // Return the regression status and details would be handled by caller
    return regressed;
}