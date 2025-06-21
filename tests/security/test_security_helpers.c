/**
 * Asthra Programming Language v1.2 Security Test Helpers Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of helper functions for security tests.
 */

#include "test_security_helpers.h"
#include <unistd.h>  // For getpid()

// =============================================================================
// SECURITY TEST HELPERS AND MOCK IMPLEMENTATIONS
// =============================================================================

// Mock constant-time comparison function
int mock_constant_time_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    volatile uint8_t result = 0;
    volatile uint8_t dummy = 0;
    
    // Ensure constant-time by always comparing all bytes
    // Use volatile operations to prevent optimization
    for (size_t i = 0; i < len; i++) {
        volatile uint8_t diff = a[i] ^ b[i];
        result |= diff;
        
        // Consistent operations to ensure stable timing
        dummy ^= diff;
        dummy = (dummy << 1) | (dummy >> 7);  // Rotate
    }
    
    // Force some operations to ensure consistent timing
    for (size_t i = 0; i < 5; i++) {
        dummy = dummy ^ (uint8_t)i;
    }
    
    // Return 0 if equal, non-zero if different
    return (result != 0) ? 1 : 0;
}

// Mock variable-time comparison (for testing contrast)
int mock_variable_time_compare(const uint8_t *a, const uint8_t *b, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (a[i] != b[i]) {
            // Early exit on first difference - this creates timing variations
            // Add variable delay based on position to emphasize timing differences
            volatile int dummy = 0;
            for (int j = 0; j < (int)(i * 5); j++) {
                dummy += j;
            }
            return a[i] - b[i];
        }
    }
    
    // If all bytes match, add full delay
    volatile int dummy = 0;
    for (int j = 0; j < (int)(len * 5); j++) {
        dummy += j;
    }
    return 0;
}

// Mock secure memory zeroing
void mock_secure_zero(volatile void *ptr, size_t size) {
    volatile uint8_t *bytes = (volatile uint8_t*)ptr;
    for (size_t i = 0; i < size; i++) {
        bytes[i] = 0;
    }
}

// Mock CSPRNG implementation using a simple LFSR-based PRNG
static uint64_t csprng_state = 0x1234567890ABCDEF;

void mock_csprng_fill(uint8_t *buffer, size_t size) {
    // Initialize state with better entropy if not already done
    if (csprng_state == 0x1234567890ABCDEF) {
        // Mix in some runtime entropy
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        csprng_state ^= (uint64_t)ts.tv_nsec;
        csprng_state ^= ((uint64_t)&buffer << 32);  // Stack address entropy
        csprng_state ^= (uint64_t)getpid() << 16;   // Process ID
        
        // Ensure non-zero state
        if (csprng_state == 0) csprng_state = 0xBADC0FFEE0DDF00D;
    }
    
    // Use a 64-bit LFSR with good statistical properties
    for (size_t i = 0; i < size; i++) {
        // Xorshift64* algorithm
        csprng_state ^= csprng_state >> 12;
        csprng_state ^= csprng_state << 25;
        csprng_state ^= csprng_state >> 27;
        uint64_t result = csprng_state * 0x2545F4914F6CDD1DULL;
        
        // Extract byte
        buffer[i] = (uint8_t)(result >> ((i % 8) * 8));
        
        // Additional mixing for better distribution
        if (i % 8 == 7) {
            csprng_state = result;
        }
    }
}

// RDTSC timing function (x86-64 specific)
uint64_t rdtsc(void) {
#if defined(__x86_64__) || defined(__i386__)
    uint32_t lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a" (lo), "=d" (hi));
    return ((uint64_t)hi << 32) | lo;
#else
    // Fallback for other architectures
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
#endif
} 
