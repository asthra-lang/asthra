/**
 * Asthra Programming Language Memory Management v1.2
 * Memory Zone Management and Garbage Collection
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * IMPLEMENTATION FEATURES:
 * - Memory zone management with C17 enhancements
 * - Conservative garbage collector with zone isolation
 * - C17 aligned allocation support
 * - Thread-safe allocation tracking
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <pthread.h>
#include <assert.h>
#include "asthra_statistics.h"
#include <inttypes.h>

// C17 modernization includes
#include <stdatomic.h>
#include <stdalign.h>

#include "asthra_runtime.h"
#include "asthra_memory.h"

// Import atomic compatibility layer from core
#if ASTHRA_HAS_ATOMICS
    #define ASTHRA_ATOMIC_LOAD(ptr) atomic_load(ptr)
    #define ASTHRA_ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
    #define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
    #define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) atomic_fetch_sub(ptr, val)
    #define ASTHRA_ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired) \
        atomic_compare_exchange_weak(ptr, expected, desired)
    #define ASTHRA_ATOMIC_ADD(ptr, val) atomic_fetch_add(ptr, val)
    #define ASTHRA_ATOMIC_SUB(ptr, val) atomic_fetch_sub(ptr, val)
#else
    #define ASTHRA_ATOMIC_LOAD(ptr) (*(ptr))
    #define ASTHRA_ATOMIC_STORE(ptr, val) (*(ptr) = (val))
    #define ASTHRA_ATOMIC_FETCH_ADD(ptr, val) (__sync_fetch_and_add(ptr, val))
    #define ASTHRA_ATOMIC_FETCH_SUB(ptr, val) (__sync_fetch_and_sub(ptr, val))
    #define ASTHRA_ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired) \
        (__sync_bool_compare_and_swap(ptr, *(expected), desired))
    #define ASTHRA_ATOMIC_ADD(ptr, val) __sync_fetch_and_add(ptr, val)
    #define ASTHRA_ATOMIC_SUB(ptr, val) __sync_fetch_and_sub(ptr, val)
#endif

// Aligned allocation wrapper
#if ASTHRA_HAS_ALIGNED_ALLOC
    #define ASTHRA_ALIGNED_ALLOC(alignment, size) aligned_alloc(alignment, size)
    #define ASTHRA_ALIGNED_FREE(ptr) free(ptr)
#else
    // Fallback using posix_memalign
    static inline void* asthra_fallback_aligned_alloc(size_t alignment, size_t size) {
        void *ptr = NULL;
        if (posix_memalign(&ptr, alignment, size) != 0) {
            return NULL;
        }
        return ptr;
    }
    #define ASTHRA_ALIGNED_ALLOC(alignment, size) asthra_fallback_aligned_alloc(alignment, size)
    #define ASTHRA_ALIGNED_FREE(ptr) free(ptr)
#endif

// =============================================================================
// MEMORY ZONE MANAGEMENT WITH C17 ENHANCEMENTS
// =============================================================================

typedef struct AsthraMemoryBlock {
    void *ptr;
    size_t size;
    AsthraMemoryZone zone;
    bool is_pinned;
    struct AsthraMemoryBlock *next;
} AsthraMemoryBlock;

typedef struct {
    AsthraMemoryBlock *blocks;
    asthra_atomic_size_t total_size;      // C17 atomic counter
    asthra_atomic_size_t block_count;     // C17 atomic counter
    pthread_mutex_t mutex;
} AsthraMemoryZoneData;

static AsthraMemoryZoneData g_memory_zones[4] = {0}; // GC, MANUAL, PINNED, STACK

// Compile-time validation of memory zone structure
ASTHRA_STATIC_ASSERT(sizeof(AsthraMemoryZoneData) <= 128, "Memory zone data should be compact");

// =============================================================================
// GARBAGE COLLECTOR IMPLEMENTATION WITH C17 ATOMICS
// =============================================================================

// GC Magic number for heap corruption detection
#define GC_MAGIC 0xDEADBEEF

// GC object header - prefixes all GC-allocated objects
typedef struct GCHeader {
    uint32_t magic;                       // Magic number for validation
    size_t size;                          // Size of the allocation
    asthra_atomic_bool marked;            // Mark bit for GC
    void (*finalizer)(void*);             // Optional finalizer
    struct GCHeader *next;                // Next object in heap
    struct GCHeader *prev;                // Previous object in heap
} GCHeader;

typedef struct AsthraGCRoot {
    void *ptr;
    struct AsthraGCRoot *next;
} AsthraGCRoot;

typedef struct {
    // Root management (hybrid approach - linked list + array)
    AsthraGCRoot *roots_list;             // Linked list of roots
    void **roots;                         // Array of root pointers for fast iteration
    size_t roots_count;                   // Number of roots in array
    size_t roots_capacity;                // Capacity of roots array
    
    // Heap management
    GCHeader *heap_start;                 // First object in heap
    GCHeader *heap_end;                   // Last object in heap
    AsthraMemoryBlock *heap_blocks;       // Memory blocks
    
    // Statistics
    asthra_atomic_size_t heap_size;       // C17 atomic - total heap size
    asthra_atomic_size_t used_memory;     // C17 atomic - currently used memory
    asthra_atomic_size_t total_allocated; // Total bytes allocated
    asthra_atomic_size_t total_collected; // Total bytes collected
    asthra_atomic_size_t collection_count; // Number of collections
    asthra_atomic_bool gc_running;        // C17 atomic bool - GC running flag
    pthread_mutex_t gc_mutex;             // Mutex for thread safety
} AsthraGC;

static AsthraGC g_gc = {0};

// =============================================================================
// MEMORY ZONE INITIALIZATION
// =============================================================================

int asthra_memory_zones_init(void) {
    // Initialize memory zones
    for (int i = 0; i < 4; i++) {
        if (pthread_mutex_init(&g_memory_zones[i].mutex, NULL) != 0) {
            return -1;
        }
        #if ASTHRA_HAS_ATOMICS
        atomic_init(&g_memory_zones[i].total_size, 0);
        atomic_init(&g_memory_zones[i].block_count, 0);
        #else
        g_memory_zones[i].total_size = 0;
        g_memory_zones[i].block_count = 0;
        #endif
    }

    // Initialize GC with C17 atomics
    if (pthread_mutex_init(&g_gc.gc_mutex, NULL) != 0) {
        return -1;
    }
    #if ASTHRA_HAS_ATOMICS
    atomic_init(&g_gc.heap_size, 0);
    atomic_init(&g_gc.used_memory, 0);
    atomic_init(&g_gc.gc_running, false);
    #else
    g_gc.heap_size = 0;
    g_gc.used_memory = 0;
    g_gc.gc_running = false;
    #endif

    return 0;
}

void asthra_memory_zones_cleanup(void) {
    // Cleanup memory zones
    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&g_memory_zones[i].mutex);
        AsthraMemoryBlock *block = g_memory_zones[i].blocks;
        while (block) {
            AsthraMemoryBlock *next = block->next;
            free(block);
            block = next;
        }
        g_memory_zones[i].blocks = NULL;
        pthread_mutex_unlock(&g_memory_zones[i].mutex);
        pthread_mutex_destroy(&g_memory_zones[i].mutex);
    }

    // Cleanup GC
    pthread_mutex_destroy(&g_gc.gc_mutex);
}

// =============================================================================
// C17 ENHANCED MEMORY ALLOCATION
// =============================================================================

static AsthraMemoryBlock *create_memory_block(void *ptr, size_t size, AsthraMemoryZone zone) {
    AsthraMemoryBlock *block = malloc(sizeof(AsthraMemoryBlock));
    if (!block) return NULL;
    
    // Use C17 compound literal for initialization
    *block = (AsthraMemoryBlock){
        .ptr = ptr,
        .size = size,
        .zone = zone,
        .is_pinned = (zone == ASTHRA_ZONE_PINNED),
        .next = NULL
    };
    
    return block;
}

void *asthra_alloc(size_t size, AsthraMemoryZone zone) {
    if (size == 0) return NULL;

    void *ptr = NULL;
    AsthraMemoryZoneData *zone_data = &g_memory_zones[zone];

    // Use appropriate allocation strategy based on zone
    switch (zone) {
        case ASTHRA_ZONE_GC:
            ptr = malloc(size);
            break;
        case ASTHRA_ZONE_MANUAL:
            ptr = malloc(size);
            break;
        case ASTHRA_ZONE_PINNED:
            ptr = malloc(size);
            if (ptr) {
                asthra_gc_pin_memory(ptr, size);
            }
            break;
        case ASTHRA_ZONE_STACK:
            // Stack allocation not supported, fallback to malloc
            ptr = malloc(size);
            break;
    }

    if (ptr) {
        pthread_mutex_lock(&zone_data->mutex);
        
        AsthraMemoryBlock *block = create_memory_block(ptr, size, zone);
        if (block) {
            block->next = zone_data->blocks;
            zone_data->blocks = block;
            
            // Update atomic counters
            ASTHRA_ATOMIC_FETCH_ADD(&zone_data->total_size, size);
            ASTHRA_ATOMIC_FETCH_ADD(&zone_data->block_count, 1);
            
            // Update global statistics
            asthra_atomic_stats_update_allocation(size);
        }
        
        pthread_mutex_unlock(&zone_data->mutex);

        // Update thread-local allocation counter
        #if ASTHRA_HAS_THREAD_LOCAL
        extern ASTHRA_THREAD_LOCAL AsthraThreadGCState *asthra_thread_gc_state;
        if (asthra_thread_gc_state) {
            ASTHRA_ATOMIC_FETCH_ADD(&asthra_thread_gc_state->allocations_since_gc, 1);
        }
        #endif
    }

    return ptr;
}

#if ASTHRA_HAS_ALIGNED_ALLOC
void *asthra_aligned_alloc(size_t alignment, size_t size, AsthraMemoryZone zone) {
    if (size == 0 || alignment == 0) return NULL;
    
    // Ensure alignment is power of 2 and size is multiple of alignment
    if ((alignment & (alignment - 1)) != 0) return NULL;
    if (size % alignment != 0) {
        size = ((size + alignment - 1) / alignment) * alignment;
    }

    void *ptr = ASTHRA_ALIGNED_ALLOC(alignment, size);
    if (!ptr) return NULL;

    AsthraMemoryZoneData *zone_data = &g_memory_zones[zone];
    pthread_mutex_lock(&zone_data->mutex);
    
    AsthraMemoryBlock *block = create_memory_block(ptr, size, zone);
    if (block) {
        block->next = zone_data->blocks;
        zone_data->blocks = block;
        
        // Update atomic counters
        ASTHRA_ATOMIC_FETCH_ADD(&zone_data->total_size, size);
        ASTHRA_ATOMIC_FETCH_ADD(&zone_data->block_count, 1);
        
        // Update global statistics
        asthra_atomic_stats_record_aligned_allocation(size, alignment);
    }
    
    pthread_mutex_unlock(&zone_data->mutex);
    return ptr;
}

void asthra_aligned_free(void *ptr, AsthraMemoryZone zone) {
    if (!ptr) return;
    
    AsthraMemoryZoneData *zone_data = &g_memory_zones[zone];
    pthread_mutex_lock(&zone_data->mutex);
    
    AsthraMemoryBlock **current = &zone_data->blocks;
    while (*current) {
        if ((*current)->ptr == ptr) {
            AsthraMemoryBlock *to_remove = *current;
            *current = (*current)->next;
            
            // Update atomic counters
            ASTHRA_ATOMIC_FETCH_SUB(&zone_data->total_size, to_remove->size);
            ASTHRA_ATOMIC_FETCH_SUB(&zone_data->block_count, 1);
            
            // Update global statistics
            asthra_atomic_stats_update_deallocation(to_remove->size);
            
            if (zone == ASTHRA_ZONE_PINNED) {
                asthra_gc_unpin_memory(ptr);
            }
            
            free(to_remove);
            break;
        }
        current = &(*current)->next;
    }
    
    pthread_mutex_unlock(&zone_data->mutex);
    ASTHRA_ALIGNED_FREE(ptr);
}
#endif

void *asthra_alloc_zeroed(size_t size, AsthraMemoryZone zone) {
    void *ptr = asthra_alloc(size, zone);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

void asthra_free(void *ptr, AsthraMemoryZone zone) {
    if (!ptr) return;

    AsthraMemoryZoneData *zone_data = &g_memory_zones[zone];
    pthread_mutex_lock(&zone_data->mutex);
    
    AsthraMemoryBlock **current = &zone_data->blocks;
    while (*current) {
        if ((*current)->ptr == ptr) {
            AsthraMemoryBlock *to_remove = *current;
            *current = (*current)->next;
            
            // Update atomic counters
            ASTHRA_ATOMIC_FETCH_SUB(&zone_data->total_size, to_remove->size);
            ASTHRA_ATOMIC_FETCH_SUB(&zone_data->block_count, 1);
            
            // Update global statistics
            asthra_atomic_stats_update_deallocation(to_remove->size);
            
            if (zone == ASTHRA_ZONE_PINNED) {
                asthra_gc_unpin_memory(ptr);
            }
            
            free(to_remove);
            break;
        }
        current = &(*current)->next;
    }
    
    pthread_mutex_unlock(&zone_data->mutex);
    free(ptr);
}

// =============================================================================
// GARBAGE COLLECTION IMPLEMENTATION
// =============================================================================

// Mark an object and all objects it references
static void gc_mark_object(GCHeader *header) {
    if (!header || header->magic != GC_MAGIC) {
        return;
    }
    
    // If already marked, we're done (prevents cycles)
    if (ASTHRA_ATOMIC_LOAD(&header->marked)) {
        return;
    }
    
    // Mark this object
    ASTHRA_ATOMIC_STORE(&header->marked, true);
    
    // For a conservative collector, we scan the object's memory
    // looking for potential pointers to other GC objects
    char *obj_start = (char*)header + sizeof(GCHeader);
    char *obj_end = obj_start + header->size;
    
    // Scan for pointers (aligned to sizeof(void*))
    for (char *p = obj_start; p + sizeof(void*) <= obj_end; p += sizeof(void*)) {
        void *potential_ptr = *(void**)p;
        
        // Check if this could be a pointer to a GC object
        if (potential_ptr) {
            // Walk the heap to see if this points to a valid GC object
            GCHeader *current = g_gc.heap_start;
            while (current && current <= g_gc.heap_end) {
                if (current->magic != GC_MAGIC) {
                    break; // Heap corruption
                }
                
                char *current_obj = (char*)current + sizeof(GCHeader);
                if (potential_ptr >= (void*)current_obj && 
                    potential_ptr < (void*)(current_obj + current->size)) {
                    // Found a pointer to a GC object
                    gc_mark_object(current);
                    break;
                }
                
                current = current->next;
            }
        }
    }
}

void asthra_gc_collect(void) {
    pthread_mutex_lock(&g_gc.gc_mutex);
    if (!ASTHRA_ATOMIC_LOAD(&g_gc.gc_running)) {
        ASTHRA_ATOMIC_STORE(&g_gc.gc_running, true);
        
        // Implement actual GC collection
        uint64_t start_time = asthra_get_timestamp_ms();
        
        // Phase 1: Mark all reachable objects from roots
        // Mark all registered roots
        for (size_t i = 0; i < g_gc.roots_count; i++) {
            GCHeader *header = (GCHeader*)((char*)g_gc.roots[i] - sizeof(GCHeader));
            if (header->magic == GC_MAGIC) {
                gc_mark_object(header);
            }
        }
        
        // Phase 2: Sweep unreachable objects
        size_t swept_count = 0;
        size_t swept_bytes = 0;
        GCHeader *current = g_gc.heap_start;
        GCHeader *prev = NULL;
        
        while (current < g_gc.heap_end) {
            if (current->magic != GC_MAGIC) {
                // Corrupted heap, stop collection
                break;
            }
            
            GCHeader *next = current->next;
            
            if (!ASTHRA_ATOMIC_LOAD(&current->marked)) {
                // Object is unreachable, free it
                swept_count++;
                swept_bytes += current->size;
                
                // Remove from linked list
                if (prev) {
                    prev->next = next;
                } else {
                    g_gc.heap_start = next;
                }
                
                // Call finalizer if present
                if (current->finalizer) {
                    current->finalizer((char*)current + sizeof(GCHeader));
                }
                
                // Free the memory
                free(current);
            } else {
                // Object is reachable, clear mark for next GC
                ASTHRA_ATOMIC_STORE(&current->marked, false);
                prev = current;
            }
            
            current = next;
        }
        
        // Update statistics
        ASTHRA_ATOMIC_SUB(&g_gc.total_allocated, swept_bytes);
        ASTHRA_ATOMIC_ADD(&g_gc.total_collected, swept_bytes);
        ASTHRA_ATOMIC_ADD(&g_gc.collection_count, 1);
        
        uint64_t collection_time = asthra_get_timestamp_ms() - start_time;
        // Update global statistics (convert ms to ns)
        asthra_atomic_stats_update_gc_collection(collection_time * 1000000);
        (void)collection_time; // Suppress unused variable warning
        
        ASTHRA_ATOMIC_STORE(&g_gc.gc_running, false);
    }
    pthread_mutex_unlock(&g_gc.gc_mutex);
}

void asthra_gc_register_root(void *ptr) {
    if (!ptr) return;
    
    pthread_mutex_lock(&g_gc.gc_mutex);
    
    AsthraGCRoot *root = malloc(sizeof(AsthraGCRoot));
    if (root) {
        root->ptr = ptr;
        root->next = g_gc.roots;
        g_gc.roots = root;
    }
    
    pthread_mutex_unlock(&g_gc.gc_mutex);
}

void asthra_gc_unregister_root(void *ptr) {
    if (!ptr) return;
    
    pthread_mutex_lock(&g_gc.gc_mutex);
    
    AsthraGCRoot **current = &g_gc.roots;
    while (*current) {
        if ((*current)->ptr == ptr) {
            AsthraGCRoot *to_remove = *current;
            *current = (*current)->next;
            free(to_remove);
            break;
        }
        current = &(*current)->next;
    }
    
    pthread_mutex_unlock(&g_gc.gc_mutex);
}

size_t asthra_gc_get_heap_size(void) {
    return ASTHRA_ATOMIC_LOAD(&g_gc.heap_size);
}

size_t asthra_gc_get_used_memory(void) {
    return ASTHRA_ATOMIC_LOAD(&g_memory_zones[ASTHRA_ZONE_GC].total_size);
}

void asthra_gc_pin_memory(void *ptr, size_t size) {
    if (!ptr) return;
    
    // For now, this is a simple implementation that marks memory as pinned
    // In a full GC implementation, this would prevent the GC from moving or collecting this memory
    pthread_mutex_lock(&g_gc.gc_mutex);
    
    // Find the memory block and mark it as pinned
    AsthraMemoryZoneData *zone_data = &g_memory_zones[ASTHRA_ZONE_PINNED];
    pthread_mutex_lock(&zone_data->mutex);
    
    AsthraMemoryBlock *current = zone_data->blocks;
    while (current) {
        if (current->ptr == ptr) {
            current->is_pinned = true;
            break;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&zone_data->mutex);
    pthread_mutex_unlock(&g_gc.gc_mutex);
    
    // Suppress unused parameter warning
    (void)size;
}

void asthra_gc_unpin_memory(void *ptr) {
    if (!ptr) return;
    
    // Unmark memory as pinned, allowing GC to manage it normally
    pthread_mutex_lock(&g_gc.gc_mutex);
    
    // Find the memory block and unmark it as pinned
    AsthraMemoryZoneData *zone_data = &g_memory_zones[ASTHRA_ZONE_PINNED];
    pthread_mutex_lock(&zone_data->mutex);
    
    AsthraMemoryBlock *current = zone_data->blocks;
    while (current) {
        if (current->ptr == ptr) {
            current->is_pinned = false;
            break;
        }
        current = current->next;
    }
    
    pthread_mutex_unlock(&zone_data->mutex);
    pthread_mutex_unlock(&g_gc.gc_mutex);
}

// =============================================================================
// MEMORY ZONE DIAGNOSTICS
// =============================================================================

void asthra_log_memory_zones(void) {
    printf("[MEMORY] Memory zone statistics:\n");
    const char *zone_names[] = {"GC", "MANUAL", "PINNED", "STACK"};
    
    for (int i = 0; i < 4; i++) {
        pthread_mutex_lock(&g_memory_zones[i].mutex);
        printf("[MEMORY]   %s: %zu bytes in %zu blocks\n", 
               zone_names[i], 
               ASTHRA_ATOMIC_LOAD(&g_memory_zones[i].total_size),
               ASTHRA_ATOMIC_LOAD(&g_memory_zones[i].block_count));
        pthread_mutex_unlock(&g_memory_zones[i].mutex);
    }
}

void asthra_log_gc_stats(void) {
    AsthraRuntimeStats stats = asthra_get_runtime_stats();
    printf("[GC] Garbage collection statistics:\n");
    printf("[GC]   Collections: %" PRIu64 "\n", stats.gc_collections);
    printf("[GC]   GC time: %" PRIu64 " ms\n", stats.gc_time_ms);
    printf("[GC]   Heap size: %zu bytes\n", asthra_gc_get_heap_size());
    printf("[GC]   Used memory: %zu bytes\n", asthra_gc_get_used_memory());
} 
