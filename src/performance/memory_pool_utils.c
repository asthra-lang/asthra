#include "memory_pool_internal.h"

// =============================================================================
// GLOBAL VARIABLES
// =============================================================================

// Thread-local storage for performance tracking
ASTHRA_MEMORY_POOL_THREAD_LOCAL AsthraPoolStatistics *g_thread_local_stats __attribute__((unused)) =
    NULL;

// Global atomic counters for memory tracking (reserved for future use)
_Atomic(uint64_t) g_global_allocations __attribute__((unused)) = 0;
_Atomic(uint64_t) g_global_deallocations __attribute__((unused)) = 0;
_Atomic(size_t) g_global_memory_usage __attribute__((unused)) = 0;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Alignment utilities
size_t asthra_align_up(size_t value, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return value; // Invalid alignment, return unchanged
    }
    return (value + alignment - 1) & ~(alignment - 1);
}

size_t asthra_align_down(size_t value, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return value; // Invalid alignment, return unchanged
    }
    return value & ~(alignment - 1);
}

bool asthra_is_aligned(const void *ptr, size_t alignment) {
    if (alignment == 0 || (alignment & (alignment - 1)) != 0) {
        return false; // Invalid alignment
    }
    return ((uintptr_t)ptr & (alignment - 1)) == 0;
}

// Cache-friendly memory operations with prefetching
void asthra_memory_copy_prefetch(void *dest, const void *src, size_t size) {
    const char *src_ptr = (const char *)src;
    char *dest_ptr = (char *)dest;

    // Prefetch source data
    for (size_t i = 0; i < size; i += ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE) {
        asthra_prefetch_read(src_ptr + i, 3);
    }

    // Prefetch destination for write
    for (size_t i = 0; i < size; i += ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE) {
        asthra_prefetch_write(dest_ptr + i, 3);
    }

    // Perform the copy
    memcpy(dest, src, size);
}

void asthra_memory_set_prefetch(void *dest, int value, size_t size) {
    char *dest_ptr = (char *)dest;

    // Prefetch destination for write
    for (size_t i = 0; i < size; i += ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE) {
        asthra_prefetch_write(dest_ptr + i, 3);
    }

    // Perform the set
    memset(dest, value, size);
}

// Statistics management
void asthra_pool_statistics_reset(AsthraPoolStatistics *stats) {
    if (!stats)
        return;

    atomic_store(&stats->total_allocations, 0);
    atomic_store(&stats->total_deallocations, 0);
    atomic_store(&stats->current_allocations, 0);
    atomic_store(&stats->peak_allocations, 0);
    atomic_store(&stats->total_memory_allocated, 0);
    atomic_store(&stats->current_memory_used, 0);
    atomic_store(&stats->peak_memory_used, 0);
    atomic_store(&stats->allocation_failures, 0);
    atomic_store(&stats->fragmentation_events, 0);
    stats->average_allocation_time_ns = 0.0;
    stats->average_deallocation_time_ns = 0.0;
}

void asthra_pool_statistics_merge(AsthraPoolStatistics *dest, const AsthraPoolStatistics *src) {
    if (!dest || !src)
        return;

    atomic_fetch_add(&dest->total_allocations, atomic_load(&src->total_allocations));
    atomic_fetch_add(&dest->total_deallocations, atomic_load(&src->total_deallocations));
    atomic_fetch_add(&dest->allocation_failures, atomic_load(&src->allocation_failures));
    atomic_fetch_add(&dest->fragmentation_events, atomic_load(&src->fragmentation_events));

    // Update peak values if necessary
    size_t src_peak_allocs = atomic_load(&src->peak_allocations);
    size_t dest_peak_allocs = atomic_load(&dest->peak_allocations);
    if (src_peak_allocs > dest_peak_allocs) {
        atomic_store(&dest->peak_allocations, src_peak_allocs);
    }

    size_t src_peak_memory = atomic_load(&src->peak_memory_used);
    size_t dest_peak_memory = atomic_load(&dest->peak_memory_used);
    if (src_peak_memory > dest_peak_memory) {
        atomic_store(&dest->peak_memory_used, src_peak_memory);
    }
}
