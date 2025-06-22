/**
 * Asthra Programming Language Runtime v1.2 - Memory Management Module
 * Memory Allocation, Garbage Collection, and Memory Zone Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides memory management functionality including
 * garbage collection, memory zones, and allocation functions.
 */

#ifndef ASTHRA_RUNTIME_MEMORY_H
#define ASTHRA_RUNTIME_MEMORY_H

#include "../core/asthra_runtime_core.h"
#include "../collections/asthra_runtime_slices.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// C17 ENHANCED MEMORY MANAGEMENT
// =============================================================================

// C17 aligned allocation support
#if ASTHRA_HAS_ALIGNED_ALLOC
    void *asthra_aligned_alloc(size_t alignment, size_t size, AsthraMemoryZone zone);
    void asthra_aligned_free(void *ptr, AsthraMemoryZone zone);
#endif

// Thread-local GC state for performance optimization
typedef struct {
    void **gc_roots;
    size_t root_count;
    size_t root_capacity;
    asthra_atomic_size_t allocations_since_gc;
} AsthraThreadGCState;

#if ASTHRA_HAS_THREAD_LOCAL
    extern ASTHRA_THREAD_LOCAL AsthraThreadGCState *asthra_thread_gc_state;
#endif

// C17 atomic memory statistics
typedef struct {
    asthra_atomic_counter_t total_allocations;
    asthra_atomic_counter_t total_deallocations;
    asthra_atomic_size_t current_memory_usage;
    asthra_atomic_size_t peak_memory_usage;
    asthra_atomic_counter_t gc_collections;
    asthra_atomic_counter_t gc_time_ms;
    asthra_atomic_counter_t tasks_spawned;
    asthra_atomic_counter_t tasks_completed;
    asthra_atomic_counter_t ffi_calls;
    asthra_atomic_bool gc_running;
} AsthraAtomicMemoryStats;

// =============================================================================
// GARBAGE COLLECTION CONFIGURATION
// =============================================================================

// GC configuration with C17 designated initializers support
typedef struct {
    size_t initial_heap_size;
    size_t max_heap_size;
    double gc_threshold;      // Trigger GC when heap usage exceeds this ratio
    bool conservative_mode;   // Use conservative scanning
    bool concurrent_gc;       // Enable concurrent garbage collection
    bool use_thread_local_roots; // Use thread-local GC roots for performance
} AsthraGCConfig;

// Default GC configuration using C17 compound literal
#define ASTHRA_DEFAULT_GC_CONFIG ((AsthraGCConfig){ \
    .initial_heap_size = 1024 * 1024, \
    .max_heap_size = 64 * 1024 * 1024, \
    .gc_threshold = 0.8, \
    .conservative_mode = true, \
    .concurrent_gc = false, \
    .use_thread_local_roots = ASTHRA_HAS_THREAD_LOCAL \
})

// =============================================================================
// RUNTIME INITIALIZATION AND CLEANUP
// =============================================================================

// Runtime initialization and cleanup
int asthra_runtime_init(AsthraGCConfig *gc_config);
int asthra_runtime_init_with_args(AsthraGCConfig *gc_config, int argc, char **argv);
void asthra_runtime_cleanup(void);

// Command-line arguments access  
AsthraSliceHeader asthra_runtime_get_args(void);

// =============================================================================
// MEMORY ALLOCATION FUNCTIONS
// =============================================================================

// Memory allocation with C17 enhancements
void *asthra_alloc(size_t size, AsthraMemoryZone zone);
void *asthra_realloc(void *ptr, size_t new_size, AsthraMemoryZone zone);
void asthra_free(void *ptr, AsthraMemoryZone zone);
void *asthra_alloc_zeroed(size_t size, AsthraMemoryZone zone);

// =============================================================================
// GARBAGE COLLECTION OPERATIONS
// =============================================================================

// GC operations with atomic counters
void asthra_gc_collect(void);
void asthra_gc_register_root(void *ptr);
void asthra_gc_unregister_root(void *ptr);
void asthra_gc_pin_memory(void *ptr, size_t size);
void asthra_gc_unpin_memory(void *ptr);
size_t asthra_gc_get_heap_size(void);
size_t asthra_gc_get_used_memory(void);

// Thread-local GC root management
#if ASTHRA_HAS_THREAD_LOCAL
void asthra_gc_register_thread_local_root(void *ptr);
void asthra_gc_unregister_thread_local_root(void *ptr);
void asthra_gc_flush_thread_local_roots(void);
#endif

// =============================================================================
// RUNTIME STATISTICS WITH C17 ATOMICS
// =============================================================================

typedef struct {
    asthra_atomic_counter_t total_allocations;
    asthra_atomic_counter_t total_deallocations;
    asthra_atomic_size_t current_memory_usage;
    asthra_atomic_size_t peak_memory_usage;
    asthra_atomic_counter_t gc_collections;
    asthra_atomic_counter_t gc_time_ms;
    asthra_atomic_counter_t tasks_spawned;
    asthra_atomic_counter_t tasks_completed;
    asthra_atomic_counter_t ffi_calls;
} AsthraRuntimeStats;

AsthraRuntimeStats asthra_get_runtime_stats(void);
void asthra_reset_runtime_stats(void);

// Internal functions for atomic statistics updates
void asthra_atomic_stats_update_allocation(size_t size);
void asthra_atomic_stats_update_deallocation(size_t size);
void asthra_atomic_stats_update_gc_collection(uint64_t time_ms);
void asthra_atomic_stats_update_task_spawned(void);
void asthra_atomic_stats_update_task_completed(void);
void asthra_atomic_stats_update_ffi_call(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_MEMORY_H 
