/**
 * Asthra Programming Language
 * Performance Test Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of performance measurement and memory tracking functions
 */

#include "performance_test_utils.h"
#include "test_framework.h"
#include <stdatomic.h>
#include <stdio.h>

// =============================================================================
// GLOBAL TEST STATE
// =============================================================================

static _Atomic(size_t) g_memory_used = 0;
static _Atomic(size_t) g_peak_memory = 0;
static _Atomic(size_t) g_allocations_count = 0;
static _Atomic(size_t) g_deallocations_count = 0;

// =============================================================================
// PERFORMANCE TESTING UTILITIES
// =============================================================================

PerformanceMetrics* start_performance_measurement(void) {
    PerformanceMetrics* metrics = malloc(sizeof(PerformanceMetrics));
    if (!metrics) {
        return NULL;
    }

    metrics->start_time = asthra_test_get_time_ns();
    metrics->end_time = 0;
    metrics->memory_used = get_current_memory_usage();
    metrics->peak_memory = metrics->memory_used;
    metrics->allocations_count = atomic_load(&g_allocations_count);
    metrics->deallocations_count = atomic_load(&g_deallocations_count);

    track_memory_allocation(sizeof(PerformanceMetrics));
    return metrics;
}

void end_performance_measurement(PerformanceMetrics* metrics) {
    if (!metrics) {
        return;
    }

    metrics->end_time = asthra_test_get_time_ns();
    size_t current_memory = get_current_memory_usage();
    if (current_memory > metrics->peak_memory) {
        metrics->peak_memory = current_memory;
    }
}

void report_performance(const PerformanceMetrics* metrics, const char* test_name) {
    if (!metrics || !test_name) {
        return;
    }

    uint64_t duration = metrics->end_time - metrics->start_time;
    double duration_ms = asthra_test_ns_to_ms(duration);

    size_t allocations = atomic_load(&g_allocations_count) - metrics->allocations_count;
    size_t deallocations = atomic_load(&g_deallocations_count) - metrics->deallocations_count;

    printf("Performance Report for %s:\n", test_name);
    printf("  Duration: %.3f ms\n", duration_ms);
    printf("  Memory used: %zu bytes\n", metrics->memory_used);
    printf("  Peak memory: %zu bytes\n", metrics->peak_memory);
    printf("  Allocations: %zu\n", allocations);
    printf("  Deallocations: %zu\n", deallocations);
    printf("  Memory leaks: %zu\n", allocations > deallocations ? allocations - deallocations : 0);
}

void track_memory_allocation(size_t size) {
    atomic_fetch_add(&g_memory_used, size);
    atomic_fetch_add(&g_allocations_count, 1);

    size_t current = atomic_load(&g_memory_used);
    size_t peak = atomic_load(&g_peak_memory);

    while (current > peak) {
        if (atomic_compare_exchange_weak(&g_peak_memory, &peak, current)) {
            break;
        }
        peak = atomic_load(&g_peak_memory);
    }
}

void track_memory_deallocation(size_t size) {
    atomic_fetch_sub(&g_memory_used, size);
    atomic_fetch_add(&g_deallocations_count, 1);
}

size_t get_current_memory_usage(void) {
    return atomic_load(&g_memory_used);
}

void reset_memory_tracking(void) {
    atomic_store(&g_memory_used, 0);
    atomic_store(&g_peak_memory, 0);
    atomic_store(&g_allocations_count, 0);
    atomic_store(&g_deallocations_count, 0);
}
