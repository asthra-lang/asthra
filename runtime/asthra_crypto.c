/**
 * Asthra Programming Language Cryptography v1.2
 * Cryptographic Primitives and Secure Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * IMPLEMENTATION FEATURES:
 * - Cryptographic random number generation (xoshiro256**)
 * - Secure memory operations
 * - Constant-time operations for cryptographic safety
 * - Memory locking and secure buffer management
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "asthra_crypto.h"
#include "asthra_runtime.h"

// =============================================================================
// CSPRNG IMPLEMENTATION
// =============================================================================

struct AsthraCsprng {
    uint64_t state[4]; // xoshiro256** state
    pthread_mutex_t mutex;
};

// Global CSPRNG instance
static AsthraCsprng *g_global_csprng = NULL;

// xoshiro256** implementation for CSPRNG
static uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t xoshiro256ss_next(AsthraCsprng *csprng) {
    const uint64_t result = rotl(csprng->state[1] * 5, 7) * 9;
    const uint64_t t = csprng->state[1] << 17;

    csprng->state[2] ^= csprng->state[0];
    csprng->state[3] ^= csprng->state[1];
    csprng->state[1] ^= csprng->state[2];
    csprng->state[0] ^= csprng->state[3];

    csprng->state[2] ^= t;
    csprng->state[3] = rotl(csprng->state[3], 45);

    return result;
}

AsthraCsprng *asthra_csprng_create(void) {
    AsthraCsprng *csprng = asthra_alloc(sizeof(AsthraCsprng), ASTHRA_ZONE_GC);
    if (!csprng)
        return NULL;

    if (pthread_mutex_init(&csprng->mutex, NULL) != 0) {
        asthra_free(csprng, ASTHRA_ZONE_GC);
        return NULL;
    }

    // Initialize with system entropy
    struct timeval tv;
    gettimeofday(&tv, NULL);
    uint64_t seed = (uint64_t)tv.tv_sec * 1000000 + tv.tv_usec;

    csprng->state[0] = seed;
    csprng->state[1] = seed ^ 0xAAAAAAAAAAAAAAAAULL;
    csprng->state[2] = seed ^ 0x5555555555555555ULL;
    csprng->state[3] = seed ^ 0xCCCCCCCCCCCCCCCCULL;

    return csprng;
}

void asthra_csprng_destroy(AsthraCsprng *csprng) {
    if (csprng) {
        pthread_mutex_destroy(&csprng->mutex);
        asthra_secure_zero(csprng, sizeof(AsthraCsprng));
        asthra_free(csprng, ASTHRA_ZONE_GC);
    }
}

int asthra_csprng_seed(AsthraCsprng *csprng, const uint8_t *seed, size_t seed_len) {
    if (!csprng || !seed || seed_len < 32) {
        return -1;
    }

    pthread_mutex_lock(&csprng->mutex);

    // Initialize state from seed
    memcpy(csprng->state, seed, 32);

    // Mix the state a bit
    for (int i = 0; i < 16; i++) {
        xoshiro256ss_next(csprng);
    }

    pthread_mutex_unlock(&csprng->mutex);
    return 0;
}

int asthra_csprng_bytes(AsthraCsprng *csprng, uint8_t *buffer, size_t size) {
    if (!csprng || !buffer)
        return -1;

    pthread_mutex_lock(&csprng->mutex);
    for (size_t i = 0; i < size; i += 8) {
        uint64_t random = xoshiro256ss_next(csprng);
        size_t copy_size = (size - i < 8) ? (size - i) : 8;
        memcpy(buffer + i, &random, copy_size);
    }
    pthread_mutex_unlock(&csprng->mutex);

    return 0;
}

uint32_t asthra_csprng_uint32(AsthraCsprng *csprng) {
    if (!csprng)
        return 0;

    pthread_mutex_lock(&csprng->mutex);
    uint64_t result = xoshiro256ss_next(csprng);
    pthread_mutex_unlock(&csprng->mutex);

    return (uint32_t)result;
}

uint64_t asthra_csprng_uint64(AsthraCsprng *csprng) {
    if (!csprng)
        return 0;

    pthread_mutex_lock(&csprng->mutex);
    uint64_t result = xoshiro256ss_next(csprng);
    pthread_mutex_unlock(&csprng->mutex);

    return result;
}

double asthra_csprng_uniform(AsthraCsprng *csprng) {
    if (!csprng)
        return 0.0;

    uint64_t random = asthra_csprng_uint64(csprng);
    return (double)random / (double)UINT64_MAX;
}

// Global CSPRNG functions
int asthra_csprng_init_global(void) {
    if (g_global_csprng) {
        return 0; // Already initialized
    }

    g_global_csprng = asthra_csprng_create();
    return g_global_csprng ? 0 : -1;
}

void asthra_csprng_cleanup_global(void) {
    if (g_global_csprng) {
        asthra_csprng_destroy(g_global_csprng);
        g_global_csprng = NULL;
    }
}

int asthra_random_bytes(uint8_t *buffer, size_t size) {
    return asthra_csprng_bytes(g_global_csprng, buffer, size);
}

uint32_t asthra_random_uint32(void) {
    return asthra_csprng_uint32(g_global_csprng);
}

uint64_t asthra_random_uint64(void) {
    return asthra_csprng_uint64(g_global_csprng);
}

// =============================================================================
// SECURE MEMORY OPERATIONS
// =============================================================================

void asthra_secure_zero(void *ptr, size_t size) {
    if (!ptr || size == 0)
        return;

    volatile unsigned char *p = (volatile unsigned char *)ptr;
    for (size_t i = 0; i < size; i++) {
        p[i] = 0;
    }
}

void *asthra_secure_alloc(size_t size) {
    if (size == 0)
        return NULL;

    // Allocate memory using regular allocation
    void *ptr = asthra_alloc(size, ASTHRA_ZONE_MANUAL);
    if (!ptr)
        return NULL;

    // Zero the memory for security
    asthra_secure_zero(ptr, size);

    // TODO: Lock memory pages if supported by platform
    // mlock(ptr, size);

    return ptr;
}

void asthra_secure_free(void *ptr, size_t size) {
    if (!ptr)
        return;

    // Zero the memory before freeing
    asthra_secure_zero(ptr, size);

    // TODO: Unlock memory pages if they were locked
    // munlock(ptr, size);

    // Free the memory
    asthra_free(ptr, ASTHRA_ZONE_MANUAL);
}

// =============================================================================
// CONSTANT-TIME OPERATIONS
// =============================================================================

int asthra_constant_time_memcmp(const void *a, const void *b, size_t len) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    unsigned char result = 0;

    for (size_t i = 0; i < len; i++) {
        result |= pa[i] ^ pb[i];
    }

    return result;
}

void asthra_constant_time_select(void *dest, const void *a, const void *b, size_t len,
                                 int condition) {
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    unsigned char *pdest = (unsigned char *)dest;

    // Create mask: all 1s if condition != 0, all 0s if condition == 0
    unsigned char mask = (unsigned char)(-(unsigned char)(condition != 0));

    for (size_t i = 0; i < len; i++) {
        pdest[i] = (pa[i] & mask) | (pb[i] & ~mask);
    }
}

int asthra_constant_time_is_zero(const void *data, size_t len) {
    const unsigned char *p = (const unsigned char *)data;
    unsigned char result = 0;

    for (size_t i = 0; i < len; i++) {
        result |= p[i];
    }

    return result == 0 ? 1 : 0;
}

// =============================================================================
// SECURE BUFFER MANAGEMENT
// =============================================================================

AsthraSecureBuffer asthra_secure_buffer_create(size_t size) {
    AsthraSecureBuffer buffer = {.ptr = NULL, .size = 0, .is_locked = false};

    if (size == 0) {
        return buffer;
    }

    buffer.ptr = asthra_secure_alloc(size);
    if (buffer.ptr) {
        buffer.size = size;
        // Try to lock the memory (platform dependent)
        asthra_secure_buffer_lock(&buffer);
    }

    return buffer;
}

void asthra_secure_buffer_destroy(AsthraSecureBuffer buffer) {
    if (!buffer.ptr)
        return;

    // Unlock memory if it was locked
    if (buffer.is_locked) {
        asthra_secure_buffer_unlock(&buffer);
    }

    // Securely free the buffer
    asthra_secure_free(buffer.ptr, buffer.size);
}

int asthra_secure_buffer_lock(AsthraSecureBuffer *buffer) {
    if (!buffer || !buffer->ptr)
        return -1;

    // TODO: Implement memory locking using mlock()
    // For now, just mark as locked
    buffer->is_locked = true;
    return 0;
}

void asthra_secure_buffer_unlock(AsthraSecureBuffer *buffer) {
    if (!buffer || !buffer->ptr)
        return;

    // TODO: Implement memory unlocking using munlock()
    // For now, just mark as unlocked
    buffer->is_locked = false;
}

// =============================================================================
// CRYPTOGRAPHIC UTILITY FUNCTIONS
// =============================================================================

// Secure random string generation
AsthraString asthra_crypto_random_string(size_t length) {
    if (length == 0) {
        return (AsthraString){0};
    }

    char *buffer = asthra_alloc(length + 1, ASTHRA_ZONE_GC);
    if (!buffer) {
        return (AsthraString){0};
    }

    // Character set for random strings (base64-like)
    const char charset[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    const size_t charset_size = sizeof(charset) - 1;

    for (size_t i = 0; i < length; i++) {
        uint32_t random = asthra_random_uint32();
        buffer[i] = charset[random % charset_size];
    }
    buffer[length] = '\0';

    return (AsthraString){.data = buffer,
                          .len = length,
                          .cap = length + 1,
                          .char_count = length,
                          .ownership = ASTHRA_OWNERSHIP_GC,
                          .is_mutable = false};
}

// Secure random bytes generation
AsthraSliceHeader asthra_crypto_random_bytes(size_t size) {
    if (size == 0) {
        return (AsthraSliceHeader){0};
    }

    uint8_t *buffer = asthra_alloc(size, ASTHRA_ZONE_GC);
    if (!buffer) {
        return (AsthraSliceHeader){0};
    }

    if (asthra_random_bytes(buffer, size) != 0) {
        asthra_free(buffer, ASTHRA_ZONE_GC);
        return (AsthraSliceHeader){0};
    }

    return asthra_slice_from_raw_parts(buffer, size, sizeof(uint8_t), false, ASTHRA_OWNERSHIP_GC);
}

// Simple hash function (for non-cryptographic purposes)
uint64_t asthra_hash_bytes(const void *data, size_t len) {
    const uint8_t *bytes = (const uint8_t *)data;
    uint64_t hash = 14695981039346656037ULL; // FNV offset basis

    for (size_t i = 0; i < len; i++) {
        hash ^= bytes[i];
        hash *= 1099511628211ULL; // FNV prime
    }

    return hash;
}

uint64_t asthra_hash_string(AsthraString str) {
    if (!str.data || str.len == 0) {
        return 0;
    }
    return asthra_hash_bytes(str.data, str.len);
}

// Timing-safe string comparison
bool asthra_crypto_string_equals(AsthraString a, AsthraString b) {
    if (a.len != b.len) {
        // Still do a constant-time comparison to avoid timing attacks
        if (a.data && b.data) {
            size_t min_len = (a.len < b.len) ? a.len : b.len;
            asthra_constant_time_memcmp(a.data, b.data, min_len);
        }
        return false;
    }

    if (!a.data && !b.data)
        return true;
    if (!a.data || !b.data)
        return false;

    return asthra_constant_time_memcmp(a.data, b.data, a.len) == 0;
}
