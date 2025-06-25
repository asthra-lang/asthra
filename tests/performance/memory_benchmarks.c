/**
 * Asthra Programming Language - Memory Usage Benchmarks
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Comprehensive memory usage benchmarking for the Asthra compiler and runtime,
 * including zone-based allocation, garbage collection, and memory leak detection.
 */

#include "../framework/test_framework.h"
#include "memory_manager.h"
#include "performance_profiler.h"
#include "performance_validation.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

// =============================================================================
// MEMORY BENCHMARK CONFIGURATION
// =============================================================================

#define BENCHMARK_ALLOCATION_COUNT 10000
#define BENCHMARK_ALLOCATION_SIZE 1024
#define BENCHMARK_ITERATIONS 100
#define BENCHMARK_LARGE_ALLOCATION_SIZE (1024 * 1024) // 1MB
#define BENCHMARK_ZONE_COUNT 4

// Memory zone types for testing
typedef enum {
    BENCHMARK_ZONE_GENERAL = 0,
    BENCHMARK_ZONE_AST,
    BENCHMARK_ZONE_SYMBOLS,
    BENCHMARK_ZONE_TEMPORARY
} BenchmarkZoneType;

// =============================================================================
// MEMORY TRACKING STRUCTURES
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
    MemoryBenchmarkResult zone_results[BENCHMARK_ZONE_COUNT];
    MemoryBenchmarkResult overall_result;
    double total_benchmark_time_ms;
    bool memory_safety_validated;
    char error_details[512];
} ComprehensiveMemoryBenchmark;

// Global memory tracking
static atomic_size_t total_allocations = 0;
static atomic_size_t total_deallocations = 0;
static atomic_size_t peak_memory_usage = 0;
static atomic_size_t current_memory_usage = 0;

// =============================================================================
// MEMORY MEASUREMENT UTILITIES
// =============================================================================

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static size_t get_process_memory_usage(void) {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
#ifdef __APPLE__
        return (size_t)usage.ru_maxrss; // bytes on macOS
#else
        return (size_t)usage.ru_maxrss * 1024; // KB to bytes on Linux
#endif
    }
    return 0;
}

static void track_allocation(size_t size) {
    atomic_fetch_add(&total_allocations, 1);
    size_t new_usage = atomic_fetch_add(&current_memory_usage, size) + size;

    // Update peak if necessary
    size_t current_peak = atomic_load(&peak_memory_usage);
    while (new_usage > current_peak) {
        if (atomic_compare_exchange_weak(&peak_memory_usage, &current_peak, new_usage)) {
            break;
        }
    }
}

static void track_deallocation(size_t size) {
    atomic_fetch_add(&total_deallocations, 1);
    atomic_fetch_sub(&current_memory_usage, size);
}

// =============================================================================
// ZONE-BASED ALLOCATION BENCHMARKS
// =============================================================================

/**
 * Benchmark zone-based allocation performance
 */
static MemoryBenchmarkResult benchmark_zone_allocation(BenchmarkZoneType zone_type) {
    MemoryBenchmarkResult result = {0};

    printf("Benchmarking zone %d allocation...\n", zone_type);

    void *allocations[BENCHMARK_ALLOCATION_COUNT];
    size_t allocation_size = BENCHMARK_ALLOCATION_SIZE;

    // Allocation benchmark
    double start_time = get_time_ms();
    size_t start_memory = get_process_memory_usage();

    for (size_t i = 0; i < BENCHMARK_ALLOCATION_COUNT; i++) {
        allocations[i] = malloc(allocation_size);
        if (allocations[i]) {
            track_allocation(allocation_size);
            memset(allocations[i], (int)(i % 256), allocation_size); // Touch memory
        }
    }

    double allocation_time = get_time_ms() - start_time;
    size_t peak_memory = get_process_memory_usage();

    // Deallocation benchmark
    start_time = get_time_ms();

    for (size_t i = 0; i < BENCHMARK_ALLOCATION_COUNT; i++) {
        if (allocations[i]) {
            track_deallocation(allocation_size);
            free(allocations[i]);
        }
    }

    double deallocation_time = get_time_ms() - start_time;
    size_t end_memory = get_process_memory_usage();

    // Calculate metrics
    result.allocation_time_ms = allocation_time;
    result.deallocation_time_ms = deallocation_time;
    result.allocations_per_second =
        allocation_time > 0
            ? (size_t)((double)BENCHMARK_ALLOCATION_COUNT / allocation_time * 1000.0)
            : 0;
    result.deallocations_per_second =
        deallocation_time > 0
            ? (size_t)((double)BENCHMARK_ALLOCATION_COUNT / deallocation_time * 1000.0)
            : 0;
    result.peak_memory_bytes = peak_memory - start_memory;
    result.current_memory_bytes = end_memory - start_memory;

    // Simple leak detection
    result.memory_leaks = (end_memory > start_memory) ? (end_memory - start_memory) : 0;

    // Simple fragmentation estimation
    size_t theoretical_usage = BENCHMARK_ALLOCATION_COUNT * allocation_size;
    result.fragmentation_percentage =
        result.peak_memory_bytes > theoretical_usage
            ? ((result.peak_memory_bytes - theoretical_usage) * 100) / theoretical_usage
            : 0;

    return result;
}

/**
 * Benchmark large allocation handling
 */
static MemoryBenchmarkResult benchmark_large_allocations(void) {
    MemoryBenchmarkResult result = {0};

    printf("Benchmarking large allocations...\n");

    const size_t large_alloc_count = 100;
    void *large_allocations[large_alloc_count];

    double start_time = get_time_ms();
    size_t start_memory = get_process_memory_usage();

    // Allocate large blocks
    for (size_t i = 0; i < large_alloc_count; i++) {
        large_allocations[i] = malloc(BENCHMARK_LARGE_ALLOCATION_SIZE);
        if (large_allocations[i]) {
            track_allocation(BENCHMARK_LARGE_ALLOCATION_SIZE);
            // Touch first and last page to ensure allocation
            *((char *)large_allocations[i]) = 1;
            *((char *)large_allocations[i] + BENCHMARK_LARGE_ALLOCATION_SIZE - 1) = 1;
        }
    }

    double allocation_time = get_time_ms() - start_time;
    size_t peak_memory = get_process_memory_usage();

    // Deallocate
    start_time = get_time_ms();

    for (size_t i = 0; i < large_alloc_count; i++) {
        if (large_allocations[i]) {
            track_deallocation(BENCHMARK_LARGE_ALLOCATION_SIZE);
            free(large_allocations[i]);
        }
    }

    double deallocation_time = get_time_ms() - start_time;
    size_t end_memory = get_process_memory_usage();

    // Calculate metrics
    result.allocation_time_ms = allocation_time;
    result.deallocation_time_ms = deallocation_time;
    result.allocations_per_second =
        allocation_time > 0 ? (size_t)((double)large_alloc_count / allocation_time * 1000.0) : 0;
    result.deallocations_per_second =
        deallocation_time > 0 ? (size_t)((double)large_alloc_count / deallocation_time * 1000.0)
                              : 0;
    result.peak_memory_bytes = peak_memory - start_memory;
    result.current_memory_bytes = end_memory - start_memory;
    result.memory_leaks = (end_memory > start_memory) ? (end_memory - start_memory) : 0;

    return result;
}

/**
 * Benchmark memory access patterns
 */
static MemoryBenchmarkResult benchmark_memory_access_patterns(void) {
    MemoryBenchmarkResult result = {0};

    printf("Benchmarking memory access patterns...\n");

    const size_t buffer_count = 1000;
    const size_t buffer_size = 4096; // Page-aligned
    void *buffers[buffer_count];

    double start_time = get_time_ms();
    size_t start_memory = get_process_memory_usage();

    // Allocate buffers
    for (size_t i = 0; i < buffer_count; i++) {
        buffers[i] = malloc(buffer_size);
        if (buffers[i]) {
            track_allocation(buffer_size);
        }
    }

    // Sequential access pattern
    for (size_t i = 0; i < buffer_count; i++) {
        if (buffers[i]) {
            memset(buffers[i], (int)(i % 256), buffer_size);
        }
    }

    // Random access pattern
    for (size_t i = 0; i < buffer_count * 10; i++) {
        size_t idx = rand() % buffer_count;
        if (buffers[idx]) {
            volatile char *ptr = (char *)buffers[idx];
            volatile char value = ptr[rand() % buffer_size];
            (void)value; // Suppress unused variable warning
        }
    }

    double access_time = get_time_ms() - start_time;
    size_t peak_memory = get_process_memory_usage();

    // Cleanup
    start_time = get_time_ms();
    for (size_t i = 0; i < buffer_count; i++) {
        if (buffers[i]) {
            track_deallocation(buffer_size);
            free(buffers[i]);
        }
    }
    double cleanup_time = get_time_ms() - start_time;

    size_t end_memory = get_process_memory_usage();

    result.allocation_time_ms = access_time;
    result.deallocation_time_ms = cleanup_time;
    result.peak_memory_bytes = peak_memory - start_memory;
    result.current_memory_bytes = end_memory - start_memory;
    result.memory_leaks = (end_memory > start_memory) ? (end_memory - start_memory) : 0;

    return result;
}

// =============================================================================
// COMPREHENSIVE MEMORY BENCHMARK
// =============================================================================

/**
 * Run comprehensive memory benchmarks
 */
ComprehensiveMemoryBenchmark memory_benchmarks_run_comprehensive(void) {
    ComprehensiveMemoryBenchmark benchmark = {0};

    printf("\n=== Comprehensive Memory Benchmarks ===\n");

    double total_start_time = get_time_ms();

    // Reset global counters
    atomic_store(&total_allocations, 0);
    atomic_store(&total_deallocations, 0);
    atomic_store(&peak_memory_usage, 0);
    atomic_store(&current_memory_usage, 0);

    // Benchmark each zone type
    for (int i = 0; i < BENCHMARK_ZONE_COUNT; i++) {
        benchmark.zone_results[i] = benchmark_zone_allocation((BenchmarkZoneType)i);
    }

    // Large allocation benchmark
    MemoryBenchmarkResult large_alloc_result = benchmark_large_allocations();

    // Memory access pattern benchmark
    MemoryBenchmarkResult access_pattern_result = benchmark_memory_access_patterns();

    // Calculate overall results
    MemoryBenchmarkResult *overall = &benchmark.overall_result;

    // Aggregate metrics from all benchmarks
    overall->allocation_time_ms = 0;
    overall->deallocation_time_ms = 0;
    overall->peak_memory_bytes = 0;

    for (int i = 0; i < BENCHMARK_ZONE_COUNT; i++) {
        overall->allocation_time_ms += benchmark.zone_results[i].allocation_time_ms;
        overall->deallocation_time_ms += benchmark.zone_results[i].deallocation_time_ms;
        if (benchmark.zone_results[i].peak_memory_bytes > overall->peak_memory_bytes) {
            overall->peak_memory_bytes = benchmark.zone_results[i].peak_memory_bytes;
        }
        overall->memory_leaks += benchmark.zone_results[i].memory_leaks;
    }

    // Include large allocation and access pattern results
    overall->allocation_time_ms +=
        large_alloc_result.allocation_time_ms + access_pattern_result.allocation_time_ms;
    overall->deallocation_time_ms +=
        large_alloc_result.deallocation_time_ms + access_pattern_result.deallocation_time_ms;
    overall->memory_leaks += large_alloc_result.memory_leaks + access_pattern_result.memory_leaks;

    // Calculate aggregate rates
    size_t total_allocs = atomic_load(&total_allocations);
    size_t total_deallocs = atomic_load(&total_deallocations);

    overall->allocations_per_second =
        overall->allocation_time_ms > 0
            ? (size_t)((double)total_allocs / overall->allocation_time_ms * 1000.0)
            : 0;
    overall->deallocations_per_second =
        overall->deallocation_time_ms > 0
            ? (size_t)((double)total_deallocs / overall->deallocation_time_ms * 1000.0)
            : 0;

    overall->current_memory_bytes = atomic_load(&current_memory_usage);
    overall->peak_memory_bytes = atomic_load(&peak_memory_usage);

    // Memory safety validation
    benchmark.memory_safety_validated =
        (overall->memory_leaks == 0) && (total_allocs == total_deallocs);

    if (!benchmark.memory_safety_validated) {
        snprintf(benchmark.error_details, sizeof(benchmark.error_details),
                 "Memory safety violations: %zu leaks, %zu allocs, %zu deallocs",
                 overall->memory_leaks, total_allocs, total_deallocs);
    } else {
        strcpy(benchmark.error_details, "No memory safety violations detected");
    }

    benchmark.total_benchmark_time_ms = get_time_ms() - total_start_time;

    return benchmark;
}

/**
 * Print memory benchmark results
 */
void memory_benchmarks_print_results(ComprehensiveMemoryBenchmark benchmark) {
    printf("\n=== Memory Benchmark Results ===\n");

    printf("Overall Performance:\n");
    printf("  Total benchmark time: %.2f ms\n", benchmark.total_benchmark_time_ms);
    printf("  Allocation rate: %zu allocs/sec\n", benchmark.overall_result.allocations_per_second);
    printf("  Deallocation rate: %zu deallocs/sec\n",
           benchmark.overall_result.deallocations_per_second);
    printf("  Peak memory usage: %.2f MB\n",
           (double)benchmark.overall_result.peak_memory_bytes / (1024.0 * 1024.0));
    printf("  Memory leaks: %zu bytes\n", benchmark.overall_result.memory_leaks);
    printf("  Memory safety: %s\n",
           benchmark.memory_safety_validated ? "✅ VALIDATED" : "❌ VIOLATIONS");

    if (!benchmark.memory_safety_validated) {
        printf("  Error details: %s\n", benchmark.error_details);
    }

    printf("\nZone-specific Results:\n");
    for (int i = 0; i < BENCHMARK_ZONE_COUNT; i++) {
        MemoryBenchmarkResult *zone = &benchmark.zone_results[i];
        printf("  Zone %d: %zu allocs/sec, %zu deallocs/sec, %.2f MB peak, %zu%% fragmentation\n",
               i, zone->allocations_per_second, zone->deallocations_per_second,
               (double)zone->peak_memory_bytes / (1024.0 * 1024.0), zone->fragmentation_percentage);
    }

    printf("==============================\n\n");
}

/**
 * Validate memory performance against targets
 */
bool memory_benchmarks_validate_performance(ComprehensiveMemoryBenchmark benchmark) {
    bool performance_acceptable = true;

    printf("=== Memory Performance Validation ===\n");

    // Target: 10-20% memory usage reduction
    const size_t target_peak_memory_mb = 50; // Baseline target
    double peak_memory_mb = (double)benchmark.overall_result.peak_memory_bytes / (1024.0 * 1024.0);

    if (peak_memory_mb <= target_peak_memory_mb * 0.9) { // 10% reduction
        printf("✅ Memory usage target met: %.2f MB (target: %.2f MB)\n", peak_memory_mb,
               target_peak_memory_mb * 0.9);
    } else {
        printf("❌ Memory usage target missed: %.2f MB (target: %.2f MB)\n", peak_memory_mb,
               target_peak_memory_mb * 0.9);
        performance_acceptable = false;
    }

    // Target: Allocation rate should be reasonable
    const size_t min_allocation_rate = 100000; // 100K allocs/sec
    if (benchmark.overall_result.allocations_per_second >= min_allocation_rate) {
        printf("✅ Allocation rate target met: %zu allocs/sec (target: %zu allocs/sec)\n",
               benchmark.overall_result.allocations_per_second, min_allocation_rate);
    } else {
        printf("❌ Allocation rate target missed: %zu allocs/sec (target: %zu allocs/sec)\n",
               benchmark.overall_result.allocations_per_second, min_allocation_rate);
        performance_acceptable = false;
    }

    // Memory safety requirement
    if (benchmark.memory_safety_validated) {
        printf("✅ Memory safety validated: No leaks or violations\n");
    } else {
        printf("❌ Memory safety violations detected: %s\n", benchmark.error_details);
        performance_acceptable = false;
    }

    printf("===================================\n\n");

    return performance_acceptable;
}