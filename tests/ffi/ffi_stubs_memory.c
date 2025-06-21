/**
 * FFI Test Stubs - Memory Management Implementation
 * 
 * Provides FFI memory management with tracking
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_stubs_memory.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global FFI memory manager instance
static FFIMemoryManager global_ffi_mem_manager = {0};

// =============================================================================
// FFI MEMORY MANAGEMENT IMPLEMENTATION
// =============================================================================

void ffi_memory_manager_init(void) {
    atomic_init(&global_ffi_mem_manager.allocated_bytes, 0);
    atomic_init(&global_ffi_mem_manager.freed_bytes, 0);
    atomic_init(&global_ffi_mem_manager.current_usage, 0);
    atomic_init(&global_ffi_mem_manager.peak_usage, 0);
    atomic_init(&global_ffi_mem_manager.allocations, 0);
    atomic_init(&global_ffi_mem_manager.frees, 0);
    global_ffi_mem_manager.leak_detected = false;
    printf("[FFI-MEM] FFI memory manager initialized\n");
}

void* ffi_allocate_memory(size_t size) {
    void *ptr = malloc(size);
    if (ptr) {
        atomic_fetch_add(&global_ffi_mem_manager.allocated_bytes, size);
        size_t current = atomic_fetch_add(&global_ffi_mem_manager.current_usage, size) + size;
        atomic_fetch_add(&global_ffi_mem_manager.allocations, 1);
        
        // Update peak usage if necessary
        size_t peak = atomic_load(&global_ffi_mem_manager.peak_usage);
        while (current > peak) {
            if (atomic_compare_exchange_weak(&global_ffi_mem_manager.peak_usage, &peak, current)) {
                break;
            }
        }
        
        printf("[FFI-MEM] Allocated %zu bytes at %p. Current: %zu\n", 
               size, ptr, atomic_load(&global_ffi_mem_manager.current_usage));
    } else {
        fprintf(stderr, "[FFI-MEM] Memory allocation failed for %zu bytes\n", size);
    }
    return ptr;
}

void ffi_free_memory(void* ptr, size_t size) {
    if (ptr) {
        // Update statistics first
        atomic_fetch_add(&global_ffi_mem_manager.freed_bytes, size);
        atomic_fetch_sub(&global_ffi_mem_manager.current_usage, size);
        atomic_fetch_add(&global_ffi_mem_manager.frees, 1);
        size_t current_usage = atomic_load(&global_ffi_mem_manager.current_usage);
        
        // Log before freeing
        printf("[FFI-MEM] Freed %zu bytes at %p. Current: %zu\n", 
               size, ptr, current_usage);
        
        // Now free the memory
        free(ptr);
    } else {
        fprintf(stderr, "[FFI-MEM] Attempted to free NULL pointer\n");
    }
}

FFIMemoryStats ffi_get_memory_statistics(void) {
    FFIMemoryStats stats = {0};
    stats.total_allocated_bytes = atomic_load(&global_ffi_mem_manager.allocated_bytes);
    stats.total_freed_bytes = atomic_load(&global_ffi_mem_manager.freed_bytes);
    stats.current_memory_usage = atomic_load(&global_ffi_mem_manager.current_usage);
    stats.peak_memory_usage = atomic_load(&global_ffi_mem_manager.peak_usage);
    stats.total_allocations = atomic_load(&global_ffi_mem_manager.allocations);
    stats.total_frees = atomic_load(&global_ffi_mem_manager.frees);
    stats.leak_detected = (stats.current_memory_usage > 0); // Simple leak detection
    return stats;
}

void ffi_memory_manager_cleanup(void) {
    // Report any detected leaks
    size_t current_usage = atomic_load(&global_ffi_mem_manager.current_usage);
    if (current_usage > 0) {
        fprintf(stderr, "[FFI-MEM] WARNING: FFI memory leak detected! Current usage: %zu bytes\n",
                current_usage);
        global_ffi_mem_manager.leak_detected = true;
    } else {
        printf("[FFI-MEM] All FFI memory successfully freed. No leaks detected.\n");
    }
    // Reset for next run, but don't re-init atomics here
    memset(&global_ffi_mem_manager, 0, sizeof(FFIMemoryManager));
}