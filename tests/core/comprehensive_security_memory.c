/**
 * Comprehensive Security Tests - Memory Security
 * 
 * Secure memory zeroing and memory security validation functions.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "comprehensive_security_common.h"

// =============================================================================
// MEMORY SECURITY FUNCTIONS
// =============================================================================

bool asthra_v12_verify_secure_memory_zeroing(void *ptr, size_t size) {
    if (!ptr || size == 0) return false;

    uint8_t *bytes = (uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        if (bytes[i] != 0) {
            return false;
        }
    }
    return true;
}

void asthra_v12_secure_memory_zero(void *ptr, size_t size) {
    if (!ptr || size == 0) return;

    // Use volatile to prevent compiler optimization
    volatile uint8_t *volatile_ptr = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        volatile_ptr[i] = 0;
    }
}