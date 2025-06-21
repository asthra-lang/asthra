/*
 * Asthra Fast Check Performance Profiler Output Implementation
 * Console output and reporting functions
 */

#include "performance_profiler.h"
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

// =============================================================================
// Console Output
// =============================================================================

void performance_print_summary(const PerformanceProfile *profile) {
    if (!profile) return;
    
    printf("📊 Fast Check Performance Summary\n");
    printf("=================================\n");
    printf("Overall Duration: %.1f ms\n", performance_timer_get_duration_ms(&profile->overall_timer));
    printf("Peak Memory: %.1f MB\n", (double)profile->memory_stats.peak_memory_bytes / (1024.0 * 1024.0));
    printf("Cache Hit Rate: %.1f%%\n", profile->cache_stats.hit_rate_percentage);
    printf("Files Processed: %u\n", profile->file_stats.files_processed);
    printf("Average File Time: %.1f ms\n", profile->file_stats.average_file_time_ms);
    printf("\n");
    
    // Performance targets
    printf("🎯 Performance Targets\n");
    printf("Single File (< %.0f ms): %s\n", profile->target_single_file_ms, 
           profile->meets_single_file_target ? "✅ PASS" : "❌ FAIL");
    printf("Medium Project (< %.0f ms): %s\n", profile->target_medium_project_ms,
           profile->meets_medium_project_target ? "✅ PASS" : "❌ FAIL");
    printf("Large Project (< %.0f ms): %s\n", profile->target_large_project_ms,
           profile->meets_large_project_target ? "✅ PASS" : "❌ FAIL");
    printf("\n");
}

void performance_print_detailed_stats(const PerformanceProfile *profile) {
    if (!profile) return;
    
    performance_print_summary(profile);
    
    printf("📈 Detailed Statistics\n");
    printf("======================\n");
    
    // Memory stats
    printf("Memory Usage:\n");
    printf("  Peak: %.1f MB\n", (double)profile->memory_stats.peak_memory_bytes / (1024.0 * 1024.0));
    printf("  Current: %.1f MB\n", (double)profile->memory_stats.current_memory_bytes / (1024.0 * 1024.0));
    printf("  Objects Allocated: %zu\n", profile->memory_stats.allocated_objects);
    printf("  Objects Deallocated: %zu\n", profile->memory_stats.deallocated_objects);
    printf("  Potential Leaks: %zu\n", profile->memory_stats.memory_leaks);
    printf("\n");
    
    // Cache stats
    printf("Cache Performance:\n");
    printf("  Total Requests: %" PRIu64 "\n", profile->cache_stats.total_requests);
    printf("  Cache Hits: %" PRIu64 "\n", profile->cache_stats.cache_hits);
    printf("  Cache Misses: %" PRIu64 "\n", profile->cache_stats.cache_misses);
    printf("  Hit Rate: %.1f%%\n", profile->cache_stats.hit_rate_percentage);
    printf("  Average Lookup Time: %.2f ms\n", profile->cache_stats.average_lookup_time_ms);
    printf("  Evictions: %" PRIu64 "\n", profile->cache_stats.cache_evictions);
    printf("\n");
    
    // File processing stats
    printf("File Processing:\n");
    printf("  Files Processed: %u\n", profile->file_stats.files_processed);
    printf("  Lines Analyzed: %u\n", profile->file_stats.lines_analyzed);
    printf("  Symbols Resolved: %u\n", profile->file_stats.symbols_resolved);
    printf("  Total Time: %.1f ms\n", profile->file_stats.total_processing_time_ms);
    printf("  Average per File: %.1f ms\n", profile->file_stats.average_file_time_ms);
    if (strlen(profile->file_stats.fastest_file) > 0) {
        printf("  Fastest File: %s (%.1f ms)\n", profile->file_stats.fastest_file, profile->file_stats.fastest_time_ms);
    }
    if (strlen(profile->file_stats.slowest_file) > 0) {
        printf("  Slowest File: %s (%.1f ms)\n", profile->file_stats.slowest_file, profile->file_stats.slowest_time_ms);
    }
    printf("\n");
    
    // Parallel processing stats
    if (profile->parallel_stats.thread_pool_size > 0) {
        printf("Parallel Processing:\n");
        printf("  Thread Pool Size: %u\n", profile->parallel_stats.thread_pool_size);
        printf("  Active Threads: %u\n", profile->parallel_stats.active_threads);
        printf("  Tasks Completed: %" PRIu64 "\n", profile->parallel_stats.tasks_completed);
        printf("  Tasks Queued: %" PRIu64 "\n", profile->parallel_stats.tasks_queued);
        printf("  Thread Utilization: %.1f%%\n", profile->parallel_stats.thread_utilization_percentage);
        printf("  Parallel Efficiency: %.1f%%\n", profile->parallel_stats.parallel_efficiency);
        printf("\n");
    }
}

void performance_print_bottleneck_analysis(const PerformanceProfile *profile) {
    if (!profile) return;
    
    size_t bottleneck_count;
    BottleneckAnalysis *bottlenecks = performance_analyze_bottlenecks(profile, &bottleneck_count);
    
    if (!bottlenecks || bottleneck_count == 0) {
        printf("🚀 No significant bottlenecks detected!\n\n");
        return;
    }
    
    printf("🔍 Bottleneck Analysis\n");
    printf("=====================\n");
    
    for (size_t i = 0; i < bottleneck_count; i++) {
        printf("Bottleneck %zu:\n", i + 1);
        printf("  Type: %s\n", 
               bottlenecks[i].type == BOTTLENECK_MEMORY_ALLOCATION ? "Memory Allocation" :
               bottlenecks[i].type == BOTTLENECK_CACHE_MISSES ? "Cache Misses" :
               bottlenecks[i].type == BOTTLENECK_THREAD_CONTENTION ? "Thread Contention" :
               bottlenecks[i].type == BOTTLENECK_SEMANTIC_ANALYSIS ? "Semantic Analysis" : "Unknown");
        printf("  Impact: %.1f%%\n", bottlenecks[i].impact_percentage);
        printf("  Description: %s\n", bottlenecks[i].description);
        printf("  Suggested Fix: %s\n", bottlenecks[i].suggested_optimization);
        printf("\n");
    }
    
    performance_bottleneck_analysis_destroy(bottlenecks, bottleneck_count);
}

void performance_print_optimization_recommendations(const PerformanceProfile *profile) {
    if (!profile) return;
    
    size_t recommendation_count;
    OptimizationRecommendation *recommendations = 
        performance_get_optimization_recommendations(profile, &recommendation_count);
    
    if (!recommendations || recommendation_count == 0) {
        printf("✅ No optimization recommendations needed!\n\n");
        return;
    }
    
    printf("💡 Optimization Recommendations\n");
    printf("===============================\n");
    
    for (size_t i = 0; i < recommendation_count; i++) {
        printf("Recommendation %zu: %s\n", i + 1, recommendations[i].optimization_type);
        printf("  Description: %s\n", recommendations[i].description);
        printf("  Expected Improvement: %.1f%%\n", recommendations[i].expected_improvement_percentage);
        printf("  Requires Code Changes: %s\n", recommendations[i].requires_code_changes ? "Yes" : "No");
        printf("  Implementation Steps:\n%s\n", recommendations[i].implementation_steps);
        printf("\n");
    }
    
    performance_optimization_recommendations_destroy(recommendations, recommendation_count);
}

// =============================================================================
// FastCheckEngine Integration (Stub Implementations)
// =============================================================================

// These functions are now integrated directly into fast_check_engine.c
// and are no longer needed as stubs here.
// They are being removed as the integration is complete. 