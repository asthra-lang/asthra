#include "memory_pool_internal.h"

// =============================================================================
// PERFORMANCE MONITORING AND REPORTING
// =============================================================================

AsthraPoolPerformanceMetrics
asthra_pool_get_performance_metrics(const AsthraPoolStatistics *stats) {
    AsthraPoolPerformanceMetrics metrics = {0};

    if (!stats)
        return metrics;

    metrics.allocation_time_ns = (uint64_t)stats->average_allocation_time_ns;
    metrics.deallocation_time_ns = (uint64_t)stats->average_deallocation_time_ns;
    metrics.memory_usage_bytes = atomic_load(&stats->current_memory_used);

    // Calculate fragmentation ratio
    uint64_t total_allocs = atomic_load(&stats->total_allocations);
    if (total_allocs > 0) {
        metrics.fragmentation_ratio =
            (double)atomic_load(&stats->fragmentation_events) / (double)total_allocs;
    }

    // Calculate cache hit ratio (simplified)
    if (total_allocs > 0) {
        uint64_t failures = atomic_load(&stats->allocation_failures);
        metrics.cache_hit_ratio = 1.0 - ((double)failures / (double)total_allocs);
    }

    return metrics;
}

void asthra_pool_print_statistics(const AsthraPoolStatistics *stats, const char *pool_name) {
    if (!stats || !pool_name)
        return;

    printf("\n=== Memory Pool Statistics: %s ===\n", pool_name);
    printf("Total allocations: %llu\n", atomic_load(&stats->total_allocations));
    printf("Total deallocations: %llu\n", atomic_load(&stats->total_deallocations));
    printf("Current allocations: %llu\n", atomic_load(&stats->current_allocations));
    printf("Peak allocations: %llu\n", atomic_load(&stats->peak_allocations));
    printf("Current memory used: %zu bytes\n", atomic_load(&stats->current_memory_used));
    printf("Peak memory used: %zu bytes\n", atomic_load(&stats->peak_memory_used));
    printf("Allocation failures: %llu\n", atomic_load(&stats->allocation_failures));
    printf("Average allocation time: %.2f ns\n", stats->average_allocation_time_ns);
    printf("Average deallocation time: %.2f ns\n", stats->average_deallocation_time_ns);

    AsthraPoolPerformanceMetrics metrics = asthra_pool_get_performance_metrics(stats);
    printf("Fragmentation ratio: %.2f%%\n", metrics.fragmentation_ratio * 100.0);
    printf("Cache hit ratio: %.2f%%\n", metrics.cache_hit_ratio * 100.0);
    printf("=====================================\n");
}
