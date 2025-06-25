/**
 * Asthra Programming Language Runtime v1.2 - Cryptographic Module
 * Cryptographic Support and Secure Operations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides cryptographic functionality including
 * CSPRNG operations, secure memory, and constant-time operations.
 */

#ifndef ASTHRA_RUNTIME_CRYPTO_H
#define ASTHRA_RUNTIME_CRYPTO_H

#include "../core/asthra_runtime_core.h"

#ifdef __cplusplus
extern "C" {

// asthra_crypto_random_string
;

// asthra_crypto_random_bytes
;

// asthra_hash_bytes
;

// asthra_hash_string
;

// asthra_crypto_string_equals
;

#endif

// =============================================================================
// SECURITY OPERATIONS
// =============================================================================

void asthra_secure_zero(void *ptr, size_t size);
void *asthra_secure_alloc(size_t size);
void asthra_secure_free(void *ptr, size_t size);

// =============================================================================
// CRYPTOGRAPHIC SUPPORT
// =============================================================================

typedef struct AsthraCsprng AsthraCsprng;

// CSPRNG operations
AsthraCsprng *asthra_csprng_create(void);
void asthra_csprng_destroy(AsthraCsprng *csprng);
int asthra_csprng_seed(AsthraCsprng *csprng, const uint8_t *seed, size_t seed_len);
int asthra_csprng_bytes(AsthraCsprng *csprng, uint8_t *buffer, size_t size);
uint32_t asthra_csprng_uint32(AsthraCsprng *csprng);
uint64_t asthra_csprng_uint64(AsthraCsprng *csprng);
double asthra_csprng_uniform(AsthraCsprng *csprng);

// Global CSPRNG
int asthra_csprng_init_global(void);
void asthra_csprng_cleanup_global(void);
int asthra_random_bytes(uint8_t *buffer, size_t size);
uint32_t asthra_random_uint32(void);
uint64_t asthra_random_uint64(void);

// =============================================================================
// CONSTANT-TIME OPERATIONS
// =============================================================================

// Constant-time operations
int asthra_constant_time_memcmp(const void *a, const void *b, size_t len);
void asthra_constant_time_select(void *dest, const void *a, const void *b, size_t len,
                                 int condition);
int asthra_constant_time_is_zero(const void *data, size_t len);

// =============================================================================
// SECURE BUFFER MANAGEMENT
// =============================================================================

// Secure buffer management
typedef struct {
    void *ptr;
    size_t size;
    bool is_locked;
} AsthraSecureBuffer;

AsthraSecureBuffer asthra_secure_buffer_create(size_t size);
void asthra_secure_buffer_destroy(AsthraSecureBuffer buffer);
int asthra_secure_buffer_lock(AsthraSecureBuffer *buffer);
void asthra_secure_buffer_unlock(AsthraSecureBuffer *buffer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_RUNTIME_CRYPTO_H
