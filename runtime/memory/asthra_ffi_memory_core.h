/**
 * Asthra Safe C Memory Interface v1.0 - Core Memory Management
 * Basic allocation/deallocation functions and memory zone management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_MEMORY_CORE_H
#define ASTHRA_FFI_MEMORY_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CORE FFI MEMORY TYPES
// =============================================================================

/**
 * Memory zone hints for allocation strategy
 */
typedef enum {
    ASTHRA_ZONE_HINT_GC,           // Prefer GC-managed heap
    ASTHRA_ZONE_HINT_MANUAL,       // Prefer manual memory zone
    ASTHRA_ZONE_HINT_PINNED,       // Prefer pinned memory zone
    ASTHRA_ZONE_HINT_STACK,        // Stack-allocated (temporary)
    ASTHRA_ZONE_HINT_SECURE        // Secure memory (locked, zeroed on free)
} AsthraMemoryZoneHint;

/**
 * Ownership transfer semantics for FFI operations
 */
typedef enum {
    ASTHRA_OWNERSHIP_TRANSFER_FULL,    // Full ownership transfer
    ASTHRA_OWNERSHIP_TRANSFER_NONE,    // No ownership transfer (borrowed)
    ASTHRA_OWNERSHIP_TRANSFER_SHARED   // Shared ownership (reference counted)
} AsthraOwnershipTransfer;

/**
 * Internal memory block tracking structure
 */
typedef struct AsthraFFIMemoryBlock {
    void *ptr;
    size_t size;
    AsthraMemoryZoneHint zone_hint;
    AsthraOwnershipTransfer ownership;
    void (*cleanup)(void*);
    bool is_secure;
    struct AsthraFFIMemoryBlock *next;
} AsthraFFIMemoryBlock;

/**
 * FFI Memory statistics
 */
typedef struct {
    size_t total_allocations;
    size_t total_frees;
    size_t current_allocations;
    size_t peak_allocations;
    size_t bytes_allocated;
    size_t bytes_freed;
    size_t current_bytes;
    size_t peak_bytes;
    size_t slice_count;
    size_t string_count;
    size_t result_count;
} AsthraFFIMemoryStats;

/**
 * FFI Memory manager structure
 */
typedef struct {
    AsthraFFIMemoryBlock *blocks;
    AsthraFFIMemoryStats stats;
    pthread_mutex_t mutex;
    bool initialized;
} AsthraFFIMemoryManager;

// =============================================================================
// CORE FFI MEMORY ALLOCATION FUNCTIONS
// =============================================================================

/**
 * Initialize the FFI memory system
 * @return 0 on success, -1 on failure
 */
int asthra_ffi_memory_init(void);

/**
 * Cleanup the FFI memory system
 */
void asthra_ffi_memory_cleanup(void);

/**
 * Allocate memory with zone hint and ownership tracking
 * @param size Size in bytes to allocate
 * @param zone_hint Preferred memory zone
 * @return Pointer to allocated memory or NULL on failure
 */
void* Asthra_ffi_alloc(size_t size, AsthraMemoryZoneHint zone_hint);

/**
 * Free memory with zone hint and ownership validation
 * @param ptr Pointer to memory to free
 * @param current_zone_hint Current zone hint for validation
 */
void Asthra_ffi_free(void* ptr, AsthraMemoryZoneHint current_zone_hint);

/**
 * Reallocate memory with zone migration support
 * @param ptr Existing pointer (can be NULL)
 * @param new_size New size in bytes
 * @param zone_hint Target zone hint
 * @return Pointer to reallocated memory or NULL on failure
 */
void* Asthra_ffi_realloc(void* ptr, size_t new_size, AsthraMemoryZoneHint zone_hint);

/**
 * Allocate zeroed memory
 * @param size Size in bytes to allocate
 * @param zone_hint Preferred memory zone
 * @return Pointer to zeroed memory or NULL on failure
 */
void* Asthra_ffi_alloc_zeroed(size_t size, AsthraMemoryZoneHint zone_hint);

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

/**
 * Find memory block by pointer
 * @param ptr Pointer to search for
 * @return Block if found, NULL otherwise
 */
AsthraFFIMemoryBlock* asthra_ffi_find_memory_block(void *ptr);

/**
 * Add memory block to tracking
 * @param ptr Pointer to track
 * @param size Size of allocation
 * @param zone_hint Zone hint used
 * @param ownership Ownership transfer type
 * @param cleanup Cleanup function (optional)
 * @param is_secure Whether this is secure memory
 */
void asthra_ffi_add_memory_block(void *ptr, size_t size, AsthraMemoryZoneHint zone_hint,
                                AsthraOwnershipTransfer ownership, void (*cleanup)(void*), bool is_secure);

/**
 * Remove memory block from tracking
 * @param ptr Pointer to remove
 */
void asthra_ffi_remove_memory_block(void *ptr);

/**
 * Get memory statistics
 * @return Current memory statistics
 */
AsthraFFIMemoryStats Asthra_ffi_get_memory_stats(void);

/**
 * Reset memory statistics
 */
void Asthra_ffi_reset_memory_stats(void);

/**
 * Validate all tracked pointers
 * @return Success/failure result
 */
int Asthra_ffi_validate_all_pointers(void);

/**
 * Dump memory state to file
 * @param output_file File to write to
 */
void Asthra_ffi_dump_memory_state(FILE* output_file);

// =============================================================================
// SECURE MEMORY OPERATIONS
// =============================================================================

/**
 * Securely zero memory
 * @param ptr Pointer to memory
 * @param size Size to zero
 */
void Asthra_secure_zero(void* ptr, size_t size);

/**
 * Allocate secure memory
 * @param size Size to allocate
 * @return Pointer to secure memory or NULL on failure
 */
void* Asthra_secure_alloc(size_t size);

/**
 * Free secure memory
 * @param ptr Pointer to secure memory
 * @param size Size of allocation
 */
void Asthra_secure_free(void* ptr, size_t size);

// =============================================================================
// RESULT TYPE FUNCTIONS (SHARED ACROSS FFI MODULES)
// =============================================================================

// Forward declarations for result types
typedef enum {
    ASTHRA_FFI_RESULT_OK,
    ASTHRA_FFI_RESULT_ERR
} AsthraFFIResultTag;

typedef struct {
    AsthraFFIResultTag tag;
    union {
        struct {
            void *value;
            size_t value_size;
            uint32_t value_type_id;
            AsthraOwnershipTransfer ownership;
        } ok;
        struct {
            int error_code;
            char error_message[256];
            const char *error_source;
            void *error_context;
        } err;
    } data;
} AsthraFFIResult;

/**
 * Create a successful result (shared implementation)
 * @param value Pointer to the success value
 * @param value_size Size of the value in bytes
 * @param type_id Type identifier for the value
 * @param ownership Ownership transfer semantics
 * @return Result containing the value
 */
AsthraFFIResult Asthra_result_ok(void* value, size_t value_size, uint32_t type_id, 
                                AsthraOwnershipTransfer ownership);

/**
 * Create an error result (shared implementation)
 * @param error_code Error code
 * @param error_message Human-readable error message
 * @param error_source Source location of the error
 * @param error_context Additional error context
 * @return Result containing the error
 */
AsthraFFIResult Asthra_result_err(int error_code, const char* error_message, 
                                 const char* error_source, void* error_context);

/**
 * Check if result is successful (shared implementation)
 * @param result Result to check
 * @return true if result is Ok, false otherwise
 */
bool Asthra_result_is_ok(AsthraFFIResult result);

/**
 * Check if result is an error (shared implementation)
 * @param result Result to check
 * @return true if result is Err, false otherwise
 */
bool Asthra_result_is_err(AsthraFFIResult result);

// Global memory manager instance (defined in implementation)
extern AsthraFFIMemoryManager g_ffi_memory;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_MEMORY_CORE_H 
