/*
 * Asthra Fast Check Performance Profiler Analysis Implementation
 * Bottleneck analysis and optimization recommendations
 */

#include "performance_profiler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// Bottleneck Analysis
// =============================================================================

BottleneckAnalysis* performance_analyze_bottlenecks(const PerformanceProfile *profile, size_t *bottleneck_count) {
    if (!profile || !bottleneck_count) return NULL;
    
    BottleneckAnalysis *bottlenecks = calloc(8, sizeof(BottleneckAnalysis));  // Max 8 bottleneck types
    if (!bottlenecks) return NULL;
    
    size_t count = 0;
    
    // Memory allocation bottleneck
    if (profile->memory_stats.peak_memory_bytes > 100 * 1024 * 1024) {  // > 100MB
        bottlenecks[count].type = BOTTLENECK_MEMORY_ALLOCATION;
        bottlenecks[count].impact_percentage = 25.0;
        snprintf(bottlenecks[count].description, sizeof(bottlenecks[count].description),
                "High memory usage detected: %.1f MB peak", 
                (double)profile->memory_stats.peak_memory_bytes / (1024.0 * 1024.0));
        snprintf(bottlenecks[count].suggested_optimization, sizeof(bottlenecks[count].suggested_optimization),
                "Implement memory pooling and reduce object allocations");
        count++;
    }
    
    // Cache miss bottleneck
    if (profile->cache_stats.hit_rate_percentage < 70.0) {
        bottlenecks[count].type = BOTTLENECK_CACHE_MISSES;
        bottlenecks[count].impact_percentage = 30.0;
        snprintf(bottlenecks[count].description, sizeof(bottlenecks[count].description),
                "Low cache hit rate: %.1f%%", profile->cache_stats.hit_rate_percentage);
        snprintf(bottlenecks[count].suggested_optimization, sizeof(bottlenecks[count].suggested_optimization),
                "Increase cache size or improve cache key generation");
        count++;
    }
    
    // Thread contention bottleneck
    if (profile->parallel_stats.thread_utilization_percentage < 50.0 && profile->parallel_stats.thread_pool_size > 1) {
        bottlenecks[count].type = BOTTLENECK_THREAD_CONTENTION;
        bottlenecks[count].impact_percentage = 20.0;
        snprintf(bottlenecks[count].description, sizeof(bottlenecks[count].description),
                "Low thread utilization: %.1f%%", profile->parallel_stats.thread_utilization_percentage);
        snprintf(bottlenecks[count].suggested_optimization, sizeof(bottlenecks[count].suggested_optimization),
                "Reduce lock contention and improve work distribution");
        count++;
    }
    
    // Semantic analysis bottleneck (heuristic based on average file time)
    if (profile->file_stats.average_file_time_ms > 50.0) {
        bottlenecks[count].type = BOTTLENECK_SEMANTIC_ANALYSIS;
        bottlenecks[count].impact_percentage = 35.0;
        snprintf(bottlenecks[count].description, sizeof(bottlenecks[count].description),
                "Slow file processing: %.1f ms average", profile->file_stats.average_file_time_ms);
        snprintf(bottlenecks[count].suggested_optimization, sizeof(bottlenecks[count].suggested_optimization),
                "Optimize symbol table operations and type checking");
        count++;
    }
    
    *bottleneck_count = count;
    return bottlenecks;
}

void performance_bottleneck_analysis_destroy(BottleneckAnalysis *analysis, size_t count) {
    (void)count;  // Suppress unused parameter warning
    free(analysis);
}

// =============================================================================
// Optimization Recommendations
// =============================================================================

OptimizationRecommendation* performance_get_optimization_recommendations(
    const PerformanceProfile *profile, size_t *recommendation_count) {
    if (!profile || !recommendation_count) return NULL;
    
    OptimizationRecommendation *recommendations = calloc(10, sizeof(OptimizationRecommendation));
    if (!recommendations) return NULL;
    
    size_t count = 0;
    
    // Memory optimization
    if (profile->memory_stats.peak_memory_bytes > 50 * 1024 * 1024) {
        strncpy(recommendations[count].optimization_type, "Memory Pool", sizeof(recommendations[count].optimization_type) - 1);
        snprintf(recommendations[count].description, sizeof(recommendations[count].description),
                "Implement object pooling to reduce allocation overhead");
        recommendations[count].expected_improvement_percentage = 15.0;
        recommendations[count].requires_code_changes = true;
        snprintf(recommendations[count].implementation_steps, sizeof(recommendations[count].implementation_steps),
                "1. Create memory pools for AST nodes\n2. Implement pool-based allocation\n3. Add cleanup mechanisms");
        count++;
    }
    
    // Cache optimization
    if (profile->cache_stats.hit_rate_percentage < 80.0) {
        strncpy(recommendations[count].optimization_type, "Cache Tuning", sizeof(recommendations[count].optimization_type) - 1);
        snprintf(recommendations[count].description, sizeof(recommendations[count].description),
                "Improve cache configuration and key generation");
        recommendations[count].expected_improvement_percentage = 25.0;
        recommendations[count].requires_code_changes = true;
        snprintf(recommendations[count].implementation_steps, sizeof(recommendations[count].implementation_steps),
                "1. Increase cache size\n2. Improve hash function\n3. Add cache warming");
        count++;
    }
    
    // Parallel processing optimization
    if (profile->parallel_stats.thread_utilization_percentage < 70.0) {
        strncpy(recommendations[count].optimization_type, "Parallelization", sizeof(recommendations[count].optimization_type) - 1);
        snprintf(recommendations[count].description, sizeof(recommendations[count].description),
                "Improve parallel processing efficiency");
        recommendations[count].expected_improvement_percentage = 20.0;
        recommendations[count].requires_code_changes = true;
        snprintf(recommendations[count].implementation_steps, sizeof(recommendations[count].implementation_steps),
                "1. Reduce lock granularity\n2. Implement lock-free data structures\n3. Improve work stealing");
        count++;
    }
    
    *recommendation_count = count;
    return recommendations;
}

void performance_optimization_recommendations_destroy(OptimizationRecommendation *recommendations, size_t count) {
    (void)count;  // Suppress unused parameter warning
    free(recommendations);
}