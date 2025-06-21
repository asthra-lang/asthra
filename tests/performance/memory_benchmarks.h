/**
 * Asthra Programming Language - Memory Benchmarks Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Header for comprehensive memory usage benchmarking
 */

#ifndef ASTHRA_MEMORY_BENCHMARKS_H
#define ASTHRA_MEMORY_BENCHMARKS_H

#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MEMORY BENCHMARK STRUCTURES
// =============================================================================

typedef struct {
    size_t allocations_per_second;
    size_t deallocations_per_second;
    size_t peak_memory_bytes;
    size_t current_memory_bytes;
    double allocation_time_ms;
    double deallocation_time_ms;
    size_t memory_leaks;
    size_t fragmentation_percentage;
} MemoryBenchmarkResult;

typedef struct {
    MemoryBenchmarkResult zone_results[4]; // BENCHMARK_ZONE_COUNT
    MemoryBenchmarkResult overall_result;
    double total_benchmark_time_ms;
    bool memory_safety_validated;
    char error_details[512];
} ComprehensiveMemoryBenchmark;

// =============================================================================
// MEMORY BENCHMARK FUNCTIONS
// =============================================================================

/**
 * Run comprehensive memory benchmarks including zone-based allocation,
 * large allocations, and memory access pattern testing
 */
ComprehensiveMemoryBenchmark memory_benchmarks_run_comprehensive(void);

/**
 * Print detailed memory benchmark results
 */
void memory_benchmarks_print_results(ComprehensiveMemoryBenchmark benchmark);

/**
 * Validate memory performance against project targets
 * Returns true if all performance targets are met
 */
bool memory_benchmarks_validate_performance(ComprehensiveMemoryBenchmark benchmark);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_MEMORY_BENCHMARKS_H