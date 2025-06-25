#include "memory_pool_internal.h"

// =============================================================================
// STACK ALLOCATOR IMPLEMENTATION
// =============================================================================

AsthraStackAllocator *asthra_stack_allocator_create(const char *name, size_t total_size,
                                                    size_t alignment) {
    if (!name || total_size == 0)
        return NULL;

    AsthraStackAllocator *allocator =
        aligned_alloc(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE, sizeof(AsthraStackAllocator));
    if (!allocator)
        return NULL;

    // Initialize allocator structure
    memset(allocator, 0, sizeof(AsthraStackAllocator));
    strncpy(allocator->name, name, sizeof(allocator->name) - 1);
    allocator->total_size = total_size;
    allocator->alignment = alignment;

    // Allocate memory region
    allocator->memory_region = aligned_alloc(alignment, total_size);
    if (!allocator->memory_region) {
        free(allocator);
        return NULL;
    }

    // Initialize atomic counters
    atomic_store(&allocator->current_offset, 0);
    atomic_store(&allocator->peak_offset, 0);

    // Initialize statistics
    asthra_pool_statistics_reset(&allocator->stats);

    return allocator;
}

void asthra_stack_allocator_destroy(AsthraStackAllocator *allocator) {
    if (!allocator)
        return;

    free(allocator->memory_region);
    free(allocator);
}

void *asthra_stack_alloc(AsthraStackAllocator *allocator, size_t size) {
    return asthra_stack_alloc_aligned(allocator, size, allocator->alignment);
}

void *asthra_stack_alloc_aligned(AsthraStackAllocator *allocator, size_t size, size_t alignment) {
    if (!allocator || size == 0)
        return NULL;

    uint64_t start_time = asthra_benchmark_get_time_ns();

    // Align the size
    size = asthra_align_up(size, alignment);

    // Get current offset and calculate new offset
    size_t current_offset = atomic_load(&allocator->current_offset);
    size_t aligned_offset = asthra_align_up(current_offset, alignment);
    size_t new_offset = aligned_offset + size;

    // Check if we have enough space
    if (new_offset > allocator->total_size) {
        atomic_fetch_add(&allocator->stats.allocation_failures, 1);
        return NULL;
    }

    // Update offset atomically
    if (!atomic_compare_exchange_strong(&allocator->current_offset, &current_offset, new_offset)) {
        // Another thread modified the offset, retry
        return asthra_stack_alloc_aligned(allocator, size, alignment);
    }

    // Update peak offset
    size_t peak_offset = atomic_load(&allocator->peak_offset);
    if (new_offset > peak_offset) {
        atomic_store(&allocator->peak_offset, new_offset);
    }

    // Update statistics
    atomic_fetch_add(&allocator->stats.total_allocations, 1);
    atomic_fetch_add(&allocator->stats.current_allocations, 1);
    atomic_fetch_add(&allocator->stats.total_memory_allocated, size);
    atomic_fetch_add(&allocator->stats.current_memory_used, size);

    // Calculate allocation time
    uint64_t end_time = asthra_benchmark_get_time_ns();
    double allocation_time = (double)(end_time - start_time);
    allocator->stats.average_allocation_time_ns =
        (allocator->stats.average_allocation_time_ns + allocation_time) / 2.0;

    return (char *)allocator->memory_region + aligned_offset;
}

AsthraStackCheckpoint asthra_stack_checkpoint(AsthraStackAllocator *allocator) {
    AsthraStackCheckpoint checkpoint = {0};
    if (allocator) {
        checkpoint.offset = atomic_load(&allocator->current_offset);
        checkpoint.timestamp = asthra_benchmark_get_time_ns();
    }
    return checkpoint;
}

void asthra_stack_restore(AsthraStackAllocator *allocator, AsthraStackCheckpoint checkpoint) {
    if (!allocator)
        return;

    size_t current_offset = atomic_load(&allocator->current_offset);
    if (checkpoint.offset <= current_offset) {
        atomic_store(&allocator->current_offset, checkpoint.offset);

        // Update statistics for bulk deallocation
        size_t freed_memory = current_offset - checkpoint.offset;
        atomic_fetch_add(&allocator->stats.total_deallocations, 1);
        atomic_fetch_sub(&allocator->stats.current_memory_used, freed_memory);
    }
}

void asthra_stack_reset(AsthraStackAllocator *allocator) {
    if (!allocator)
        return;

    atomic_store(&allocator->current_offset, 0);
    atomic_store(&allocator->stats.current_allocations, 0);
    atomic_store(&allocator->stats.current_memory_used, 0);
}
