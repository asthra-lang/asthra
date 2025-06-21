/**
 * Asthra Safe C Memory Interface v1.0 - Core Memory Management Implementation
 * Basic allocation/deallocation functions and memory zone management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <assert.h>

#include "asthra_ffi_memory_core.h"
#include "../asthra_runtime.h"

// =============================================================================
// GLOBAL STATE AND INTERNAL DATA STRUCTURES
// =============================================================================

AsthraFFIMemoryManager g_ffi_memory = {0};

// Error codes for FFI operations
#define ASTHRA_FFI_ERROR_NONE           0
#define ASTHRA_FFI_ERROR_NULL_POINTER   1
#define ASTHRA_FFI_ERROR_OUT_OF_MEMORY  2
#define ASTHRA_FFI_ERROR_BOUNDS_CHECK   3
#define ASTHRA_FFI_ERROR_INVALID_SLICE  4
#define ASTHRA_FFI_ERROR_OWNERSHIP      5
#define ASTHRA_FFI_ERROR_TYPE_MISMATCH  6
#define ASTHRA_FFI_ERROR_INVALID_ARGS   7

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

int asthra_ffi_memory_init(void) {
    if (g_ffi_memory.initialized) {
        return 0;
    }
    
    if (pthread_mutex_init(&g_ffi_memory.mutex, NULL) != 0) {
        return -1;
    }
    
    memset(&g_ffi_memory.stats, 0, sizeof(AsthraFFIMemoryStats));
    g_ffi_memory.blocks = NULL;
    g_ffi_memory.initialized = true;
    
    return 0;
}

void asthra_ffi_memory_cleanup(void) {
    if (!g_ffi_memory.initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    // Free all remaining blocks
    AsthraFFIMemoryBlock *block = g_ffi_memory.blocks;
    while (block) {
        AsthraFFIMemoryBlock *next = block->next;
        
        // Call cleanup function if provided
        if (block->cleanup) {
            block->cleanup(block->ptr);
        }
        
        // Free based on zone hint
        switch (block->zone_hint) {
            case ASTHRA_ZONE_HINT_GC:
                asthra_free(block->ptr, ASTHRA_ZONE_GC);
                break;
            case ASTHRA_ZONE_HINT_MANUAL:
            case ASTHRA_ZONE_HINT_STACK:
                free(block->ptr);
                break;
            case ASTHRA_ZONE_HINT_PINNED:
                asthra_free(block->ptr, ASTHRA_ZONE_PINNED);
                break;
            case ASTHRA_ZONE_HINT_SECURE:
                Asthra_secure_free(block->ptr, block->size);
                break;
            default:
                free(block->ptr);
                break;
        }
        
        free(block);
        block = next;
    }
    
    g_ffi_memory.blocks = NULL;
    g_ffi_memory.initialized = false;
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    pthread_mutex_destroy(&g_ffi_memory.mutex);
}

AsthraFFIMemoryBlock* asthra_ffi_find_memory_block(void *ptr) {
    AsthraFFIMemoryBlock *block = g_ffi_memory.blocks;
    while (block) {
        if (block->ptr == ptr) {
            return block;
        }
        block = block->next;
    }
    return NULL;
}

void asthra_ffi_add_memory_block(void *ptr, size_t size, AsthraMemoryZoneHint zone_hint,
                               AsthraOwnershipTransfer ownership, void (*cleanup)(void*), bool is_secure) {
    AsthraFFIMemoryBlock *block = malloc(sizeof(AsthraFFIMemoryBlock));
    if (!block) return;
    
    block->ptr = ptr;
    block->size = size;
    block->zone_hint = zone_hint;
    block->ownership = ownership;
    block->cleanup = cleanup;
    block->is_secure = is_secure;
    block->next = g_ffi_memory.blocks;
    g_ffi_memory.blocks = block;
    
    // Update statistics
    g_ffi_memory.stats.total_allocations++;
    g_ffi_memory.stats.current_allocations++;
    g_ffi_memory.stats.bytes_allocated += size;
    g_ffi_memory.stats.current_bytes += size;
    
    if (g_ffi_memory.stats.current_allocations > g_ffi_memory.stats.peak_allocations) {
        g_ffi_memory.stats.peak_allocations = g_ffi_memory.stats.current_allocations;
    }
    if (g_ffi_memory.stats.current_bytes > g_ffi_memory.stats.peak_bytes) {
        g_ffi_memory.stats.peak_bytes = g_ffi_memory.stats.current_bytes;
    }
}

void asthra_ffi_remove_memory_block(void *ptr) {
    AsthraFFIMemoryBlock **current = &g_ffi_memory.blocks;
    while (*current) {
        if ((*current)->ptr == ptr) {
            AsthraFFIMemoryBlock *to_remove = *current;
            *current = (*current)->next;
            
            // Update statistics
            g_ffi_memory.stats.total_frees++;
            g_ffi_memory.stats.current_allocations--;
            g_ffi_memory.stats.bytes_freed += to_remove->size;
            g_ffi_memory.stats.current_bytes -= to_remove->size;
            
            free(to_remove);
            return;
        }
        current = &(*current)->next;
    }
}

// Internal allocation function (assumes mutex is already locked)
static void* ffi_alloc_internal(size_t size, AsthraMemoryZoneHint zone_hint) {
    if (size == 0) return NULL;
    
    void *ptr = NULL;
    
    // Map zone hints to actual allocation strategies
    switch (zone_hint) {
        case ASTHRA_ZONE_HINT_GC:
            ptr = asthra_alloc(size, ASTHRA_ZONE_GC);
            break;
        case ASTHRA_ZONE_HINT_MANUAL:
            ptr = malloc(size);
            break;
        case ASTHRA_ZONE_HINT_PINNED:
            ptr = asthra_alloc(size, ASTHRA_ZONE_PINNED);
            break;
        case ASTHRA_ZONE_HINT_STACK:
            // Stack allocation not supported for FFI
            ptr = malloc(size);
            break;
        case ASTHRA_ZONE_HINT_SECURE:
            ptr = Asthra_secure_alloc(size);
            break;
        default:
            ptr = malloc(size);
            break;
    }
    
    if (ptr) {
        asthra_ffi_add_memory_block(ptr, size, zone_hint, ASTHRA_OWNERSHIP_TRANSFER_FULL, NULL, 
                        zone_hint == ASTHRA_ZONE_HINT_SECURE);
    }
    
    return ptr;
}

// Internal free function (assumes mutex is already locked)
static void ffi_free_internal(void* ptr, AsthraMemoryZoneHint zone_hint) {
    if (!ptr) return;
    
    AsthraFFIMemoryBlock *block = asthra_ffi_find_memory_block(ptr);
    if (!block) {
        return;
    }
    
    // Call cleanup function if provided
    if (block->cleanup) {
        block->cleanup(ptr);
    }
    
    // Free based on zone hint
    switch (zone_hint) {
        case ASTHRA_ZONE_HINT_GC:
            asthra_free(ptr, ASTHRA_ZONE_GC);
            break;
        case ASTHRA_ZONE_HINT_MANUAL:
        case ASTHRA_ZONE_HINT_STACK:
            free(ptr);
            break;
        case ASTHRA_ZONE_HINT_PINNED:
            asthra_free(ptr, ASTHRA_ZONE_PINNED);
            break;
        case ASTHRA_ZONE_HINT_SECURE:
            Asthra_secure_free(ptr, block->size);
            break;
        default:
            free(ptr);
            break;
    }
    
    asthra_ffi_remove_memory_block(ptr);
}

// =============================================================================
// PUBLIC FFI MEMORY ALLOCATION FUNCTIONS
// =============================================================================

void* Asthra_ffi_alloc(size_t size, AsthraMemoryZoneHint zone_hint) {
    if (asthra_ffi_memory_init() != 0) {
        return NULL;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    void *result = ffi_alloc_internal(size, zone_hint);
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return result;
}

void Asthra_ffi_free(void* ptr, AsthraMemoryZoneHint current_zone_hint) {
    if (!ptr || !g_ffi_memory.initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    ffi_free_internal(ptr, current_zone_hint);
    pthread_mutex_unlock(&g_ffi_memory.mutex);
}

void* Asthra_ffi_realloc(void* ptr, size_t new_size, AsthraMemoryZoneHint zone_hint) {
    if (asthra_ffi_memory_init() != 0) {
        return NULL;
    }
    
    if (!ptr) {
        return Asthra_ffi_alloc(new_size, zone_hint);
    }
    
    if (new_size == 0) {
        Asthra_ffi_free(ptr, zone_hint);
        return NULL;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    AsthraFFIMemoryBlock *block = asthra_ffi_find_memory_block(ptr);
    if (!block) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return NULL;
    }
    
    // Allocate new memory
    void *new_ptr = ffi_alloc_internal(new_size, zone_hint);
    if (!new_ptr) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return NULL;
    }
    
    // Copy old data
    size_t copy_size = (block->size < new_size) ? block->size : new_size;
    memcpy(new_ptr, ptr, copy_size);
    
    // Free old memory
    ffi_free_internal(ptr, block->zone_hint);
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    return new_ptr;
}

void* Asthra_ffi_alloc_zeroed(size_t size, AsthraMemoryZoneHint zone_hint) {
    void *ptr = Asthra_ffi_alloc(size, zone_hint);
    if (ptr) {
        memset(ptr, 0, size);
    }
    return ptr;
}

// =============================================================================
// MEMORY STATISTICS AND VALIDATION
// =============================================================================

AsthraFFIMemoryStats Asthra_ffi_get_memory_stats(void) {
    AsthraFFIMemoryStats stats = {0};
    
    if (!g_ffi_memory.initialized) {
        return stats;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    stats = g_ffi_memory.stats;
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return stats;
}

void Asthra_ffi_reset_memory_stats(void) {
    if (!g_ffi_memory.initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    memset(&g_ffi_memory.stats, 0, sizeof(AsthraFFIMemoryStats));
    pthread_mutex_unlock(&g_ffi_memory.mutex);
}

int Asthra_ffi_validate_all_pointers(void) {
    if (!g_ffi_memory.initialized) {
        return ASTHRA_FFI_ERROR_NONE;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    AsthraFFIMemoryBlock *block = g_ffi_memory.blocks;
    while (block) {
        if (!block->ptr) {
            pthread_mutex_unlock(&g_ffi_memory.mutex);
            return ASTHRA_FFI_ERROR_NULL_POINTER;
        }
        
        if (block->size == 0) {
            pthread_mutex_unlock(&g_ffi_memory.mutex);
            return ASTHRA_FFI_ERROR_INVALID_ARGS;
        }
        
        block = block->next;
    }
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    return ASTHRA_FFI_ERROR_NONE;
}

void Asthra_ffi_dump_memory_state(FILE* output_file) {
    if (!output_file || !g_ffi_memory.initialized) {
        return;
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    fprintf(output_file, "=== FFI Memory State Dump ===\n");
    fprintf(output_file, "Initialized: %s\n", g_ffi_memory.initialized ? "true" : "false");
    fprintf(output_file, "Total allocations: %zu\n", g_ffi_memory.stats.total_allocations);
    fprintf(output_file, "Total frees: %zu\n", g_ffi_memory.stats.total_frees);
    fprintf(output_file, "Current allocations: %zu\n", g_ffi_memory.stats.current_allocations);
    fprintf(output_file, "Peak allocations: %zu\n", g_ffi_memory.stats.peak_allocations);
    fprintf(output_file, "Current bytes: %zu\n", g_ffi_memory.stats.current_bytes);
    fprintf(output_file, "Peak bytes: %zu\n", g_ffi_memory.stats.peak_bytes);
    
    fprintf(output_file, "\nActive Memory Blocks:\n");
    AsthraFFIMemoryBlock *block = g_ffi_memory.blocks;
    int block_count = 0;
    while (block) {
        fprintf(output_file, "  Block %d: ptr=%p, size=%zu, zone=%d, ownership=%d, secure=%s\n",
                block_count, block->ptr, block->size, block->zone_hint, block->ownership,
                block->is_secure ? "true" : "false");
        block = block->next;
        block_count++;
    }
    
    fprintf(output_file, "=== End Memory State Dump ===\n");
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
}

// =============================================================================
// SHARED RESULT TYPE FUNCTIONS (CONSOLIDATED FROM DUPLICATED CODE)
// =============================================================================

AsthraFFIResult Asthra_result_ok(void* value, size_t value_size, uint32_t type_id, 
                                AsthraOwnershipTransfer ownership) {
    AsthraFFIResult result;
    result.tag = ASTHRA_FFI_RESULT_OK;
    result.data.ok.value = value;
    result.data.ok.value_size = value_size;
    result.data.ok.value_type_id = type_id;
    result.data.ok.ownership = ownership;
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    g_ffi_memory.stats.result_count++;
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return result;
}

AsthraFFIResult Asthra_result_err(int error_code, const char* error_message, 
                                 const char* error_source, void* error_context) {
    AsthraFFIResult result;
    result.tag = ASTHRA_FFI_RESULT_ERR;
    result.data.err.error_code = error_code;
    result.data.err.error_source = error_source;
    result.data.err.error_context = error_context;
    
    if (error_message) {
        strncpy(result.data.err.error_message, error_message, 
                sizeof(result.data.err.error_message) - 1);
        result.data.err.error_message[sizeof(result.data.err.error_message) - 1] = '\0';
    } else {
        result.data.err.error_message[0] = '\0';
    }
    
    pthread_mutex_lock(&g_ffi_memory.mutex);
    g_ffi_memory.stats.result_count++;
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return result;
}

bool Asthra_result_is_ok(AsthraFFIResult result) {
    return result.tag == ASTHRA_FFI_RESULT_OK;
}

bool Asthra_result_is_err(AsthraFFIResult result) {
    return result.tag == ASTHRA_FFI_RESULT_ERR;
}

// =============================================================================
// SECURE MEMORY OPERATIONS
// =============================================================================

void Asthra_secure_zero(void* ptr, size_t size) {
    if (!ptr || size == 0) return;
    
    // Use volatile to prevent compiler optimization
    volatile char *volatile_ptr = (volatile char *)ptr;
    for (size_t i = 0; i < size; i++) {
        volatile_ptr[i] = 0;
    }
    
    // Memory barrier to ensure writes are completed
    __asm__ __volatile__("" ::: "memory");
}

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif
void* Asthra_secure_alloc(size_t size) {
    if (size == 0) return NULL;
    
    // Allocate memory and lock it in RAM
    void *ptr = NULL;
    ptr = malloc(size);
    if (!ptr) return NULL;
    
    // Try to lock the memory (may fail on some systems)
    // Cast to void to silence unused result warning
    (void)mlock(ptr, size);
    
    // Zero the memory
    Asthra_secure_zero(ptr, size);
    
    return ptr;
}
#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif

void Asthra_secure_free(void* ptr, size_t size) {
    if (!ptr) return;
    
    // Securely zero the memory
    Asthra_secure_zero(ptr, size);
    
    // Unlock the memory
    munlock(ptr, size);
    
    // Free the memory
    free(ptr);
} 
