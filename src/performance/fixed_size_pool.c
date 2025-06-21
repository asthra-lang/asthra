#include "memory_pool_internal.h"

// =============================================================================
// FIXED-SIZE POOL IMPLEMENTATION
// =============================================================================

AsthraFixedSizePool* asthra_fixed_pool_create(
    const char *name,
    size_t block_size,
    size_t block_count,
    size_t alignment,
    AsthraPoolThreadSafety thread_safety) {
    
    if (!name || block_size < ASTHRA_MEMORY_POOL_MIN_BLOCK_SIZE || 
        block_size > ASTHRA_MEMORY_POOL_MAX_BLOCK_SIZE || block_count == 0) {
        return NULL;
    }
    
    // Align block size
    block_size = asthra_align_up(block_size, alignment);
    
    AsthraFixedSizePool *pool = aligned_alloc(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE, 
                                              sizeof(AsthraFixedSizePool));
    if (!pool) {
        return NULL;
    }
    
    // Initialize pool structure
    memset(pool, 0, sizeof(AsthraFixedSizePool));
    strncpy(pool->name, name, sizeof(pool->name) - 1);
    pool->block_size = block_size;
    pool->block_count = block_count;
    pool->alignment = alignment;
    pool->thread_safety = thread_safety;
    
    // Allocate memory region
    size_t total_size = block_size * block_count;
    pool->memory_region = aligned_alloc(alignment, total_size);
    if (!pool->memory_region) {
        free(pool);
        return NULL;
    }
    
    // Initialize free bitmap
    size_t bitmap_size = (block_count + 7) / 8;
    pool->free_bitmap = calloc(bitmap_size, 1);
    if (!pool->free_bitmap) {
        free(pool->memory_region);
        free(pool);
        return NULL;
    }
    
    // Initialize atomic counters
    atomic_store(&pool->next_free_index, 0);
    atomic_store(&pool->allocated_count, 0);
    
    // Initialize statistics
    asthra_pool_statistics_reset(&pool->stats);
    
    return pool;
}

void asthra_fixed_pool_destroy(AsthraFixedSizePool *pool) {
    if (!pool) return;
    
    free(pool->memory_region);
    free(pool->free_bitmap);
    free(pool);
}

void* asthra_fixed_pool_alloc(AsthraFixedSizePool *pool) {
    if (!pool) return NULL;
    
    uint64_t start_time = asthra_benchmark_get_time_ns();
    
    // Find next free block
    size_t block_count = pool->block_count;
    size_t start_index = atomic_load(&pool->next_free_index);
    
    for (size_t i = 0; i < block_count; i++) {
        size_t index = (start_index + i) % block_count;
        size_t byte_index = index / 8;
        size_t bit_index = index % 8;
        
        // Check if block is free (bit is 0)
        if ((pool->free_bitmap[byte_index] & (1 << bit_index)) == 0) {
            // Try to claim this block atomically
            uint8_t old_byte = pool->free_bitmap[byte_index];
            uint8_t new_byte = (uint8_t)(old_byte | (1U << bit_index));
            
            // For thread-safe pools, use atomic compare-exchange
            if (pool->thread_safety == ASTHRA_POOL_THREAD_SAFE_ATOMIC) {
                if (__sync_bool_compare_and_swap(&pool->free_bitmap[byte_index], old_byte, new_byte)) {
                    // Successfully claimed the block
                    atomic_fetch_add(&pool->allocated_count, 1);
                    atomic_store(&pool->next_free_index, (index + 1) % block_count);
                    
                    // Update statistics
                    atomic_fetch_add(&pool->stats.total_allocations, 1);
                    atomic_fetch_add(&pool->stats.current_allocations, 1);
                    atomic_fetch_add(&pool->stats.total_memory_allocated, pool->block_size);
                    atomic_fetch_add(&pool->stats.current_memory_used, pool->block_size);
                    
                    // Update peak allocations
                    uint64_t current_allocs = atomic_load(&pool->stats.current_allocations);
                    uint64_t peak_allocs = atomic_load(&pool->stats.peak_allocations);
                    if (current_allocs > peak_allocs) {
                        atomic_store(&pool->stats.peak_allocations, current_allocs);
                    }
                    
                    // Update peak memory usage
                    size_t current_memory = atomic_load(&pool->stats.current_memory_used);
                    size_t peak_memory = atomic_load(&pool->stats.peak_memory_used);
                    if (current_memory > peak_memory) {
                        atomic_store(&pool->stats.peak_memory_used, current_memory);
                    }
                    
                    // Calculate allocation time
                    uint64_t end_time = asthra_benchmark_get_time_ns();
                    double allocation_time = (double)(end_time - start_time);
                    pool->stats.average_allocation_time_ns = 
                        (pool->stats.average_allocation_time_ns + allocation_time) / 2.0;
                    
                    // Return pointer to the allocated block
                    return (char*)pool->memory_region + (index * pool->block_size);
                }
            } else {
                // Non-thread-safe version
                pool->free_bitmap[byte_index] = new_byte;
                atomic_fetch_add(&pool->allocated_count, 1);
                atomic_store(&pool->next_free_index, (index + 1) % block_count);
                
                // Update statistics (non-atomic for performance)
                atomic_fetch_add(&pool->stats.total_allocations, 1);
                atomic_fetch_add(&pool->stats.current_allocations, 1);
                
                return (char*)pool->memory_region + (index * pool->block_size);
            }
        }
    }
    
    // No free blocks available
    atomic_fetch_add(&pool->stats.allocation_failures, 1);
    return NULL;
}

bool asthra_fixed_pool_free(AsthraFixedSizePool *pool, void *ptr) {
    if (!pool || !ptr) return false;
    
    uint64_t start_time = asthra_benchmark_get_time_ns();
    
    // Calculate block index
    char *pool_start = (char*)pool->memory_region;
    char *block_ptr = (char*)ptr;
    
    if (block_ptr < pool_start) return false;
    
    size_t offset = (size_t)(block_ptr - pool_start);
    if (offset % pool->block_size != 0) return false;
    
    size_t index = offset / pool->block_size;
    if (index >= pool->block_count) return false;
    
    // Free the block
    size_t byte_index = index / 8;
    size_t bit_index = index % 8;
    
    if (pool->thread_safety == ASTHRA_POOL_THREAD_SAFE_ATOMIC) {
        // Atomic version
        uint8_t old_byte, new_byte;
        do {
            old_byte = pool->free_bitmap[byte_index];
            new_byte = old_byte & ~(1 << bit_index);
        } while (!__sync_bool_compare_and_swap(&pool->free_bitmap[byte_index], old_byte, new_byte));
    } else {
        // Non-atomic version
        pool->free_bitmap[byte_index] &= ~(1 << bit_index);
    }
    
    atomic_fetch_sub(&pool->allocated_count, 1);
    atomic_fetch_add(&pool->stats.total_deallocations, 1);
    atomic_fetch_sub(&pool->stats.current_allocations, 1);
    atomic_fetch_sub(&pool->stats.current_memory_used, pool->block_size);
    
    // Calculate deallocation time
    uint64_t end_time = asthra_benchmark_get_time_ns();
    double deallocation_time = (double)(end_time - start_time);
    pool->stats.average_deallocation_time_ns = 
        (pool->stats.average_deallocation_time_ns + deallocation_time) / 2.0;
    
    return true;
}

bool asthra_fixed_pool_validate(const AsthraFixedSizePool *pool) {
    if (!pool || !pool->memory_region || !pool->free_bitmap) {
        return false;
    }
    
    if (pool->block_size < ASTHRA_MEMORY_POOL_MIN_BLOCK_SIZE ||
        pool->block_size > ASTHRA_MEMORY_POOL_MAX_BLOCK_SIZE) {
        return false;
    }
    
    if (pool->block_count == 0) {
        return false;
    }
    
    // Validate alignment
    if (!asthra_is_aligned(pool->memory_region, pool->alignment)) {
        return false;
    }
    
    return true;
} 
