#include "benchmark.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// MEMORY TRACKING
// =============================================================================

// Thread-local storage for memory tracking
static _Thread_local AsthraBenchmarkMemoryStats *g_current_memory_stats = NULL;

// Global atomic counters for memory tracking
static _Atomic(size_t) g_total_allocated = 0;
static _Atomic(size_t) g_total_deallocated = 0;
static _Atomic(size_t) g_peak_memory = 0;
static _Atomic(size_t) g_current_memory = 0;

void asthra_benchmark_memory_tracking_start(AsthraBenchmarkMemoryStats *stats) {
    if (!stats) {
        return;
    }

    memset(stats, 0, sizeof(AsthraBenchmarkMemoryStats));
    g_current_memory_stats = stats;

    // Reset global counters
    atomic_store(&g_total_allocated, 0);
    atomic_store(&g_total_deallocated, 0);
    atomic_store(&g_peak_memory, 0);
    atomic_store(&g_current_memory, 0);
}

void asthra_benchmark_memory_tracking_stop(AsthraBenchmarkMemoryStats *stats) {
    if (!stats || g_current_memory_stats != stats) {
        return;
    }

    stats->total_allocations = atomic_load(&g_total_allocated);
    stats->total_deallocations = atomic_load(&g_total_deallocated);
    stats->peak_memory_bytes = atomic_load(&g_peak_memory);
    stats->current_memory_bytes = atomic_load(&g_current_memory);

    g_current_memory_stats = NULL;
}

void asthra_benchmark_memory_record_allocation(size_t size) {
    atomic_fetch_add(&g_total_allocated, 1);

    size_t current = atomic_fetch_add(&g_current_memory, size) + size;

    // Update peak memory atomically
    size_t peak = atomic_load(&g_peak_memory);
    while (current > peak && !atomic_compare_exchange_weak(&g_peak_memory, &peak, current)) {
        // Retry if another thread updated peak
    }
}

void asthra_benchmark_memory_record_deallocation(size_t size) {
    atomic_fetch_add(&g_total_deallocated, 1);
    atomic_fetch_sub(&g_current_memory, size);
}
