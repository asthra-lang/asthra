#ifndef ASTHRA_MEMORY_POOL_H
#define ASTHRA_MEMORY_POOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ADVANCED MEMORY POOL IMPLEMENTATION - PHASE 6.2.2
// =============================================================================

// C17 Feature Detection for Memory Optimization
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
    #define ASTHRA_MEMORY_POOL_HAS_C17 1
    #define ASTHRA_MEMORY_POOL_ALIGNAS(x) _Alignas(x)
    #define ASTHRA_MEMORY_POOL_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
    #define ASTHRA_MEMORY_POOL_THREAD_LOCAL _Thread_local
#else
    #define ASTHRA_MEMORY_POOL_HAS_C17 0
    #define ASTHRA_MEMORY_POOL_ALIGNAS(x)
    #define ASTHRA_MEMORY_POOL_STATIC_ASSERT(cond, msg)
    #define ASTHRA_MEMORY_POOL_THREAD_LOCAL
#endif

// Memory alignment constants
#define ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE 64
#define ASTHRA_MEMORY_POOL_DEFAULT_ALIGNMENT 16
#define ASTHRA_MEMORY_POOL_MAX_ALIGNMENT 256

// Pool configuration constants
#define ASTHRA_MEMORY_POOL_MIN_BLOCK_SIZE 32
#define ASTHRA_MEMORY_POOL_MAX_BLOCK_SIZE (1024 * 1024)
#define ASTHRA_MEMORY_POOL_DEFAULT_CAPACITY 1024
#define ASTHRA_MEMORY_POOL_MAX_POOLS 64

// =============================================================================
// MEMORY POOL TYPES
// =============================================================================

// Pool allocation strategy
typedef enum {
    ASTHRA_POOL_STRATEGY_FIXED_SIZE,     // Fixed-size blocks
    ASTHRA_POOL_STRATEGY_POWER_OF_TWO,   // Power-of-two sized blocks
    ASTHRA_POOL_STRATEGY_BUDDY_SYSTEM,   // Buddy allocation system
    ASTHRA_POOL_STRATEGY_SLAB,           // Slab allocator
    ASTHRA_POOL_STRATEGY_STACK           // Stack allocator
} AsthraPoolStrategy;

// Pool memory layout optimization
typedef enum {
    ASTHRA_POOL_LAYOUT_AOS,              // Array of Structures
    ASTHRA_POOL_LAYOUT_SOA,              // Structure of Arrays
    ASTHRA_POOL_LAYOUT_AOSOA,            // Array of Structures of Arrays
    ASTHRA_POOL_LAYOUT_CACHE_FRIENDLY    // Cache-line optimized
} AsthraPoolLayout;

// Pool thread safety mode
typedef enum {
    ASTHRA_POOL_THREAD_UNSAFE,           // No thread safety
    ASTHRA_POOL_THREAD_LOCAL,            // Thread-local pools
    ASTHRA_POOL_THREAD_SAFE_ATOMIC,      // Atomic operations
    ASTHRA_POOL_THREAD_SAFE_LOCK_FREE    // Lock-free implementation
} AsthraPoolThreadSafety;

// Pool statistics for performance monitoring
typedef struct {
    ASTHRA_MEMORY_POOL_ALIGNAS(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE)
    _Atomic(uint64_t) total_allocations;
    _Atomic(uint64_t) total_deallocations;
    _Atomic(uint64_t) current_allocations;
    _Atomic(uint64_t) peak_allocations;
    _Atomic(size_t) total_memory_allocated;
    _Atomic(size_t) current_memory_used;
    _Atomic(size_t) peak_memory_used;
    _Atomic(uint64_t) allocation_failures;
    _Atomic(uint64_t) fragmentation_events;
    double average_allocation_time_ns;
    double average_deallocation_time_ns;
} AsthraPoolStatistics;

// Memory block header for tracking
typedef struct AsthraMemoryBlock {
    struct AsthraMemoryBlock *next;
    struct AsthraMemoryBlock *prev;
    size_t size;
    uint32_t magic;
    uint32_t pool_id;
    bool is_free;
    uint8_t alignment_padding;
    uint16_t flags;
} AsthraMemoryBlock;

// Fixed-size object pool
typedef struct {
    ASTHRA_MEMORY_POOL_ALIGNAS(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE)
    void *memory_region;
    size_t block_size;
    size_t block_count;
    size_t alignment;
    _Atomic(size_t) next_free_index;
    _Atomic(size_t) allocated_count;
    uint8_t *free_bitmap;
    AsthraPoolStatistics stats;
    AsthraPoolThreadSafety thread_safety;
    char name[64];
} AsthraFixedSizePool;

// Stack allocator for temporary objects
typedef struct {
    ASTHRA_MEMORY_POOL_ALIGNAS(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE)
    void *memory_region;
    size_t total_size;
    _Atomic(size_t) current_offset;
    _Atomic(size_t) peak_offset;
    size_t alignment;
    AsthraMemoryBlock *checkpoint_stack;
    AsthraPoolStatistics stats;
    char name[64];
} AsthraStackAllocator;

// Ring buffer for streaming data
typedef struct {
    ASTHRA_MEMORY_POOL_ALIGNAS(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE)
    void *memory_region;
    size_t buffer_size;
    _Atomic(size_t) write_offset;
    _Atomic(size_t) read_offset;
    _Atomic(size_t) available_bytes;
    size_t alignment;
    bool allow_overwrite;
    AsthraPoolStatistics stats;
    char name[64];
} AsthraRingBuffer;

// Slab allocator for kernel-style allocation
typedef struct AsthraSlab {
    struct AsthraSlab *next;
    void *memory_region;
    size_t object_size;
    size_t objects_per_slab;
    _Atomic(size_t) free_objects;
    uint8_t *free_bitmap;
    AsthraMemoryBlock *free_list;
} AsthraSlab;

typedef struct {
    ASTHRA_MEMORY_POOL_ALIGNAS(ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE)
    AsthraSlab *slabs;
    size_t object_size;
    size_t objects_per_slab;
    size_t alignment;
    _Atomic(size_t) total_slabs;
    AsthraPoolStatistics stats;
    char name[64];
} AsthraSlabAllocator;

// Unified memory pool manager
typedef struct {
    AsthraFixedSizePool *fixed_pools;
    AsthraStackAllocator *stack_allocators;
    AsthraRingBuffer *ring_buffers;
    AsthraSlabAllocator *slab_allocators;
    size_t pool_count;
    size_t max_pools;
    AsthraPoolStrategy default_strategy;
    AsthraPoolLayout default_layout;
    AsthraPoolThreadSafety default_thread_safety;
    AsthraPoolStatistics global_stats;
} AsthraMemoryPoolManager;

// =============================================================================
// FIXED-SIZE POOL OPERATIONS
// =============================================================================

// Create and destroy fixed-size pools
AsthraFixedSizePool* asthra_fixed_pool_create(
    const char *name,
    size_t block_size,
    size_t block_count,
    size_t alignment,
    AsthraPoolThreadSafety thread_safety
);

void asthra_fixed_pool_destroy(AsthraFixedSizePool *pool);

// Allocation and deallocation
void* asthra_fixed_pool_alloc(AsthraFixedSizePool *pool);
bool asthra_fixed_pool_free(AsthraFixedSizePool *pool, void *ptr);

// Pool management
bool asthra_fixed_pool_expand(AsthraFixedSizePool *pool, size_t additional_blocks);
void asthra_fixed_pool_reset(AsthraFixedSizePool *pool);
bool asthra_fixed_pool_validate(const AsthraFixedSizePool *pool);

// =============================================================================
// STACK ALLOCATOR OPERATIONS
// =============================================================================

// Create and destroy stack allocators
AsthraStackAllocator* asthra_stack_allocator_create(
    const char *name,
    size_t total_size,
    size_t alignment
);

void asthra_stack_allocator_destroy(AsthraStackAllocator *allocator);

// Allocation operations
void* asthra_stack_alloc(AsthraStackAllocator *allocator, size_t size);
void* asthra_stack_alloc_aligned(AsthraStackAllocator *allocator, size_t size, size_t alignment);

// Checkpoint operations for bulk deallocation
typedef struct {
    size_t offset;
    uint64_t timestamp;
} AsthraStackCheckpoint;

AsthraStackCheckpoint asthra_stack_checkpoint(AsthraStackAllocator *allocator);
void asthra_stack_restore(AsthraStackAllocator *allocator, AsthraStackCheckpoint checkpoint);
void asthra_stack_reset(AsthraStackAllocator *allocator);

// =============================================================================
// RING BUFFER OPERATIONS
// =============================================================================

// Create and destroy ring buffers
AsthraRingBuffer* asthra_ring_buffer_create(
    const char *name,
    size_t buffer_size,
    size_t alignment,
    bool allow_overwrite
);

void asthra_ring_buffer_destroy(AsthraRingBuffer *buffer);

// Write operations
size_t asthra_ring_buffer_write(AsthraRingBuffer *buffer, const void *data, size_t size);
bool asthra_ring_buffer_write_atomic(AsthraRingBuffer *buffer, const void *data, size_t size);

// Read operations
size_t asthra_ring_buffer_read(AsthraRingBuffer *buffer, void *data, size_t size);
size_t asthra_ring_buffer_peek(const AsthraRingBuffer *buffer, void *data, size_t size);

// Buffer state
size_t asthra_ring_buffer_available_write(const AsthraRingBuffer *buffer);
size_t asthra_ring_buffer_available_read(const AsthraRingBuffer *buffer);
bool asthra_ring_buffer_is_empty(const AsthraRingBuffer *buffer);
bool asthra_ring_buffer_is_full(const AsthraRingBuffer *buffer);

// =============================================================================
// SLAB ALLOCATOR OPERATIONS
// =============================================================================

// Create and destroy slab allocators
AsthraSlabAllocator* asthra_slab_allocator_create(
    const char *name,
    size_t object_size,
    size_t objects_per_slab,
    size_t alignment
);

void asthra_slab_allocator_destroy(AsthraSlabAllocator *allocator);

// Allocation operations
void* asthra_slab_alloc(AsthraSlabAllocator *allocator);
bool asthra_slab_free(AsthraSlabAllocator *allocator, void *ptr);

// Slab management
bool asthra_slab_allocator_expand(AsthraSlabAllocator *allocator);
void asthra_slab_allocator_shrink(AsthraSlabAllocator *allocator);

// =============================================================================
// MEMORY POOL MANAGER
// =============================================================================

// Global pool manager
AsthraMemoryPoolManager* asthra_pool_manager_create(size_t max_pools);
void asthra_pool_manager_destroy(AsthraMemoryPoolManager *manager);

// Pool registration
bool asthra_pool_manager_register_fixed_pool(
    AsthraMemoryPoolManager *manager,
    AsthraFixedSizePool *pool
);

bool asthra_pool_manager_register_stack_allocator(
    AsthraMemoryPoolManager *manager,
    AsthraStackAllocator *allocator
);

bool asthra_pool_manager_register_ring_buffer(
    AsthraMemoryPoolManager *manager,
    AsthraRingBuffer *buffer
);

bool asthra_pool_manager_register_slab_allocator(
    AsthraMemoryPoolManager *manager,
    AsthraSlabAllocator *allocator
);

// Global allocation interface
void* asthra_pool_manager_alloc(
    AsthraMemoryPoolManager *manager,
    size_t size,
    size_t alignment,
    AsthraPoolStrategy strategy
);

bool asthra_pool_manager_free(AsthraMemoryPoolManager *manager, void *ptr);

// =============================================================================
// CACHE OPTIMIZATION UTILITIES
// =============================================================================

// Memory prefetching hints
static inline void asthra_prefetch_read(const void *addr, int locality) {
    #if defined(__GNUC__) || defined(__clang__)
        // Locality must be a constant integer (0-3)
        switch (locality) {
            case 0: __builtin_prefetch(addr, 0, 0); break;
            case 1: __builtin_prefetch(addr, 0, 1); break;
            case 2: __builtin_prefetch(addr, 0, 2); break;
            case 3: __builtin_prefetch(addr, 0, 3); break;
            default: __builtin_prefetch(addr, 0, 3); break;
        }
    #else
        (void)addr; (void)locality;
    #endif
}

static inline void asthra_prefetch_write(const void *addr, int locality) {
    #if defined(__GNUC__) || defined(__clang__)
        // Locality must be a constant integer (0-3)
        switch (locality) {
            case 0: __builtin_prefetch(addr, 1, 0); break;
            case 1: __builtin_prefetch(addr, 1, 1); break;
            case 2: __builtin_prefetch(addr, 1, 2); break;
            case 3: __builtin_prefetch(addr, 1, 3); break;
            default: __builtin_prefetch(addr, 1, 3); break;
        }
    #else
        (void)addr; (void)locality;
    #endif
}

// Cache-friendly memory operations
void asthra_memory_copy_prefetch(void *dest, const void *src, size_t size);
void asthra_memory_set_prefetch(void *dest, int value, size_t size);

// Alignment utilities
size_t asthra_align_up(size_t value, size_t alignment);
size_t asthra_align_down(size_t value, size_t alignment);
bool asthra_is_aligned(const void *ptr, size_t alignment);

// =============================================================================
// STATISTICS AND MONITORING
// =============================================================================

// Statistics collection
void asthra_pool_statistics_reset(AsthraPoolStatistics *stats);
void asthra_pool_statistics_merge(
    AsthraPoolStatistics *dest,
    const AsthraPoolStatistics *src
);

// Performance monitoring
typedef struct {
    uint64_t allocation_time_ns;
    uint64_t deallocation_time_ns;
    size_t memory_usage_bytes;
    double fragmentation_ratio;
    double cache_hit_ratio;
} AsthraPoolPerformanceMetrics;

AsthraPoolPerformanceMetrics asthra_pool_get_performance_metrics(
    const AsthraPoolStatistics *stats
);

// Reporting
void asthra_pool_print_statistics(const AsthraPoolStatistics *stats, const char *pool_name);
void asthra_pool_manager_print_summary(const AsthraMemoryPoolManager *manager);

// =============================================================================
// COMPILE-TIME VALIDATION
// =============================================================================

// Ensure proper alignment for performance-critical structures
ASTHRA_MEMORY_POOL_STATIC_ASSERT(
    sizeof(AsthraPoolStatistics) >= ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE,
    "AsthraPoolStatistics should be cache-line aligned"
);

ASTHRA_MEMORY_POOL_STATIC_ASSERT(
    sizeof(AsthraFixedSizePool) >= ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE,
    "AsthraFixedSizePool should be cache-line aligned"
);

ASTHRA_MEMORY_POOL_STATIC_ASSERT(
    sizeof(AsthraStackAllocator) >= ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE,
    "AsthraStackAllocator should be cache-line aligned"
);

ASTHRA_MEMORY_POOL_STATIC_ASSERT(
    sizeof(AsthraRingBuffer) >= ASTHRA_MEMORY_POOL_CACHE_LINE_SIZE,
    "AsthraRingBuffer should be cache-line aligned"
);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_MEMORY_POOL_H 
