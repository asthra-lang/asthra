// This file provides stub implementations for performance profiling functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "performance_profiler.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

// =============================================================================
// PERFORMANCE PROFILING
// =============================================================================

PerformanceProfile* performance_profiler_create(void) {
    return calloc(1, sizeof(PerformanceProfile));
}

void performance_profiler_destroy(PerformanceProfile *profile) {
    free(profile);
}

void performance_timer_start(PerformanceTimer *timer) {
    if (timer) {
        clock_gettime(CLOCK_MONOTONIC, &timer->start_time);
        timer->is_active = true;
    }
}

void performance_timer_stop(PerformanceTimer *timer) {
    if (timer && timer->is_active) {
        clock_gettime(CLOCK_MONOTONIC, &timer->end_time);
        double start_ms = timer->start_time.tv_sec * 1000.0 + timer->start_time.tv_nsec / 1000000.0;
        double end_ms = timer->end_time.tv_sec * 1000.0 + timer->end_time.tv_nsec / 1000000.0;
        timer->duration_ms = end_ms - start_ms;
        timer->is_active = false;
    }
}

double performance_timer_get_duration_ms(const PerformanceTimer *timer) {
    return timer ? timer->duration_ms : 0.0;
}

void performance_track_memory_allocation(PerformanceProfile *profile, size_t bytes) {
    if (profile) {
        profile->memory_stats.allocated_objects++;
        profile->memory_stats.current_memory_bytes += bytes;
    }
}

void performance_record_cache_hit(PerformanceProfile *profile, double duration) {
    if (profile) {
        profile->cache_stats.cache_hits++;
        profile->cache_stats.total_requests++;
    }
}

void performance_record_cache_miss(PerformanceProfile *profile, double duration) {
    if (profile) {
        profile->cache_stats.cache_misses++;
        profile->cache_stats.total_requests++;
    }
}

void performance_record_file_start(PerformanceProfile *profile, const char *filepath) {
    // Stub implementation
}

void performance_record_file_complete(PerformanceProfile *profile, const char *filename, 
                                     uint32_t lines, uint32_t symbols, double duration_ms) {
    if (profile) {
        profile->file_stats.files_processed++;
        profile->file_stats.lines_analyzed += lines;
    }
}

void performance_print_summary(const PerformanceProfile *profile) {
    if (profile) {
        printf("Performance Summary: %u files processed\n", profile->file_stats.files_processed);
    }
}

void performance_print_detailed_stats(const PerformanceProfile *profile) {
    if (profile) {
        printf("Cache hits: %llu, misses: %llu\n", 
               (unsigned long long)profile->cache_stats.cache_hits, 
               (unsigned long long)profile->cache_stats.cache_misses);
    }
}

void performance_print_bottleneck_analysis(const PerformanceProfile *profile) {
    printf("Bottleneck analysis not implemented in stub\n");
}

void performance_print_optimization_recommendations(const PerformanceProfile *profile) {
    printf("Optimization recommendations not implemented in stub\n");
}

void performance_track_memory_deallocation(PerformanceProfile *profile, size_t bytes) {
    if (profile) {
        profile->memory_stats.current_memory_bytes = 
            (profile->memory_stats.current_memory_bytes > bytes) ? 
            profile->memory_stats.current_memory_bytes - bytes : 0;
    }
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_profiler_dummy = 0;

#endif // FAST_CHECK_USE_STUBS