#include "benchmark.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// REPORTING AND OUTPUT
// =============================================================================

void asthra_benchmark_print_result(const AsthraBenchmarkResult *result) {
    if (!result) {
        return;
    }
    
    printf("\n=== Benchmark Result: %s ===\n", result->config.name);
    printf("Status: %s\n", 
           result->status == ASTHRA_BENCHMARK_STATUS_SUCCESS ? "SUCCESS" : "FAILED");
    
    if (result->status != ASTHRA_BENCHMARK_STATUS_SUCCESS) {
        printf("Error: %s\n", result->error_message);
        return;
    }
    
    const AsthraBenchmarkStatistics *stats = &result->stats;
    
    printf("Iterations: %llu\n", stats->iterations);
    printf("Total time: %.3f ms\n", asthra_benchmark_ns_to_milliseconds(stats->total_ns));
    printf("Mean time: %.3f μs\n", asthra_benchmark_ns_to_microseconds(stats->mean_ns));
    printf("Median time: %.3f μs\n", asthra_benchmark_ns_to_microseconds(stats->median_ns));
    printf("Min time: %.3f μs\n", asthra_benchmark_ns_to_microseconds(stats->min_ns));
    printf("Max time: %.3f μs\n", asthra_benchmark_ns_to_microseconds(stats->max_ns));
    printf("Std dev: %.3f μs\n", asthra_benchmark_ns_to_microseconds(stats->std_dev_ns));
    printf("Throughput: %.2f ops/sec\n", stats->throughput_ops_per_sec);
    
    if (result->config.track_memory) {
        const AsthraBenchmarkMemoryStats *mem = &result->memory_stats;
        printf("\nMemory Statistics:\n");
        printf("Peak memory: %zu bytes\n", mem->peak_memory_bytes);
        printf("Total allocations: %zu\n", mem->total_allocations);
        printf("Total deallocations: %zu\n", mem->total_deallocations);
    }
    
    printf("================================\n");
}

void asthra_benchmark_print_suite_results(
    const AsthraBenchmarkResult *results,
    size_t count) {
    
    if (!results || count == 0) {
        return;
    }
    
    printf("\n=== Benchmark Suite Results ===\n");
    
    for (size_t i = 0; i < count; i++) {
        asthra_benchmark_print_result(&results[i]);
    }
    
    // Summary statistics
    size_t successful = 0;
    uint64_t total_time = 0;
    double total_throughput = 0.0;
    
    for (size_t i = 0; i < count; i++) {
        if (results[i].status == ASTHRA_BENCHMARK_STATUS_SUCCESS) {
            successful++;
            total_time += results[i].stats.total_ns;
            total_throughput += results[i].stats.throughput_ops_per_sec;
        }
    }
    
    printf("\n=== Suite Summary ===\n");
    printf("Total benchmarks: %zu\n", count);
    printf("Successful: %zu\n", successful);
    printf("Failed: %zu\n", count - successful);
    printf("Total execution time: %.3f ms\n", asthra_benchmark_ns_to_milliseconds(total_time));
    if (successful > 0) {
        printf("Average throughput: %.2f ops/sec\n", total_throughput / (double)successful);
    }
    printf("======================\n");
} 
