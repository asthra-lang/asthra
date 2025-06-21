/**
 * Asthra Safe C Memory Interface v1.0 - Ownership and Variant System
 * Ownership tracking, transfer, cleanup function registration, and reference counting
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_OWNERSHIP_H
#define ASTHRA_FFI_OWNERSHIP_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include "asthra_ffi_memory_core.h"
#include "asthra_ffi_result.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OWNERSHIP TRACKING AND VALIDATION
// =============================================================================

/**
 * Register external pointer for ownership tracking
 * @param ptr Pointer to register
 * @param size Size of the allocation
 * @param ownership Ownership transfer semantics
 * @param cleanup Optional cleanup function
 * @return Result indicating success or failure
 */
AsthraFFIResult Asthra_ownership_register(void* ptr, size_t size, 
                                         AsthraOwnershipTransfer ownership,
                                         void (*cleanup)(void*));

/**
 * Unregister pointer from ownership tracking
 * @param ptr Pointer to unregister
 * @return Result indicating success or failure
 */
AsthraFFIResult Asthra_ownership_unregister(void* ptr);

/**
 * Transfer ownership of a tracked pointer
 * @param ptr Pointer to transfer
 * @param new_ownership New ownership semantics
 * @return Result containing old ownership type or error
 */
AsthraFFIResult Asthra_ownership_transfer(void* ptr, AsthraOwnershipTransfer new_ownership);

/**
 * Query ownership information for a pointer
 * @param ptr Pointer to query
 * @return Result containing ownership information or error
 */
AsthraFFIResult Asthra_ownership_query(void* ptr);

/**
 * Ownership information structure
 */
typedef struct {
    AsthraOwnershipTransfer ownership;
    size_t size;
    AsthraMemoryZoneHint zone_hint;
    bool is_secure;
} AsthraOwnershipInfo;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_OWNERSHIP_H 
