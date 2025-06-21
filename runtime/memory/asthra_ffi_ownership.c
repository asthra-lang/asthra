/**
 * Asthra Safe C Memory Interface v1.0 - Ownership and Variant System Implementation
 * Ownership tracking, transfer, cleanup function registration, and reference counting
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "asthra_ffi_ownership.h"
#include "asthra_ffi_memory_core.h"

// =============================================================================
// OWNERSHIP TRACKING AND VALIDATION
// =============================================================================

AsthraFFIResult Asthra_ownership_register(void* ptr, size_t size, 
                                         AsthraOwnershipTransfer ownership,
                                         void (*cleanup)(void*)) {
    if (!ptr) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                               "Pointer is NULL", 
                               "Asthra_ownership_register", NULL);
    }
    
    if (asthra_ffi_memory_init() != 0) {
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                               "Failed to initialize FFI memory system", 
                               "Asthra_ownership_register", NULL);
    }
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    // Check if already registered
    AsthraFFIMemoryBlock *existing = asthra_ffi_find_memory_block(ptr);
    if (existing) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return Asthra_result_err(5, // ASTHRA_FFI_ERROR_OWNERSHIP
                               "Pointer already registered", 
                               "Asthra_ownership_register", ptr);
    }
    
    asthra_ffi_add_memory_block(ptr, size, ASTHRA_ZONE_HINT_MANUAL, ownership, cleanup, false);
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return Asthra_result_ok(NULL, 0, 0, ASTHRA_OWNERSHIP_TRANSFER_NONE);
}

AsthraFFIResult Asthra_ownership_unregister(void* ptr) {
    if (!ptr) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                               "Pointer is NULL", 
                               "Asthra_ownership_unregister", NULL);
    }
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    AsthraFFIMemoryBlock *block = asthra_ffi_find_memory_block(ptr);
    if (!block) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return Asthra_result_err(5, // ASTHRA_FFI_ERROR_OWNERSHIP
                               "Pointer not registered", 
                               "Asthra_ownership_unregister", ptr);
    }
    
    asthra_ffi_remove_memory_block(ptr);
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return Asthra_result_ok(NULL, 0, 0, ASTHRA_OWNERSHIP_TRANSFER_NONE);
}

AsthraFFIResult Asthra_ownership_transfer(void* ptr, AsthraOwnershipTransfer new_ownership) {
    if (!ptr) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                               "Pointer is NULL", 
                               "Asthra_ownership_transfer", NULL);
    }
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    AsthraFFIMemoryBlock *block = asthra_ffi_find_memory_block(ptr);
    if (!block) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return Asthra_result_err(5, // ASTHRA_FFI_ERROR_OWNERSHIP
                               "Pointer not registered", 
                               "Asthra_ownership_transfer", ptr);
    }
    
    AsthraOwnershipTransfer old_ownership = block->ownership;
    block->ownership = new_ownership;
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    int *old_ownership_ptr = malloc(sizeof(int));
    if (old_ownership_ptr) {
        *old_ownership_ptr = (int)old_ownership;
    }
    
    return Asthra_result_ok(old_ownership_ptr, sizeof(int), 0, 
                          ASTHRA_OWNERSHIP_TRANSFER_FULL);
}

AsthraFFIResult Asthra_ownership_query(void* ptr) {
    if (!ptr) {
        return Asthra_result_err(1, // ASTHRA_FFI_ERROR_NULL_POINTER
                               "Pointer is NULL", 
                               "Asthra_ownership_query", NULL);
    }
    
    extern AsthraFFIMemoryManager g_ffi_memory;
    pthread_mutex_lock(&g_ffi_memory.mutex);
    
    AsthraFFIMemoryBlock *block = asthra_ffi_find_memory_block(ptr);
    if (!block) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return Asthra_result_err(5, // ASTHRA_FFI_ERROR_OWNERSHIP
                               "Pointer not registered", 
                               "Asthra_ownership_query", ptr);
    }
    
    AsthraOwnershipInfo *info = malloc(sizeof(AsthraOwnershipInfo));
    if (!info) {
        pthread_mutex_unlock(&g_ffi_memory.mutex);
        return Asthra_result_err(2, // ASTHRA_FFI_ERROR_OUT_OF_MEMORY
                               "Failed to allocate ownership info", 
                               "Asthra_ownership_query", NULL);
    }
    
    info->ownership = block->ownership;
    info->size = block->size;
    info->zone_hint = block->zone_hint;
    info->is_secure = block->is_secure;
    
    pthread_mutex_unlock(&g_ffi_memory.mutex);
    
    return Asthra_result_ok(info, sizeof(AsthraOwnershipInfo), 0, 
                          ASTHRA_OWNERSHIP_TRANSFER_FULL);
} 
