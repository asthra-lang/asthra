/**
 * Asthra Programming Language Runtime - Hash Support Implementation
 * FFI Support for stdlib::hash package
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module provides C runtime support for hash functions, specifically
 * a complete SipHash implementation to support the stdlib::hash Asthra package.
 *
 * SipHash algorithm implementation based on the original reference by
 * Jean-Philippe Aumasson and Daniel J. Bernstein.
 */

#include "stdlib_hash_support.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =============================================================================
// INTERNAL SIPHASH IMPLEMENTATION
// =============================================================================

/**
 * SipHash round function (core compression/finalization operation)
 */
static inline void sip_round(uint64_t *v0, uint64_t *v1, uint64_t *v2, uint64_t *v3) {
    *v0 += *v1;
    *v1 = (*v1 << 13) | (*v1 >> (64 - 13));
    *v1 ^= *v0;
    *v0 = (*v0 << 32) | (*v0 >> (64 - 32));

    *v2 += *v3;
    *v3 = (*v3 << 16) | (*v3 >> (64 - 16));
    *v3 ^= *v2;

    *v0 += *v3;
    *v3 = (*v3 << 21) | (*v3 >> (64 - 21));
    *v3 ^= *v0;

    *v2 += *v1;
    *v1 = (*v1 << 17) | (*v1 >> (64 - 17));
    *v1 ^= *v2;
    *v2 = (*v2 << 32) | (*v2 >> (64 - 32));
}

/**
 * Core SipHash implementation with configurable rounds
 * @param data Input data
 * @param len Length of input data
 * @param key 128-bit key
 * @param c_rounds Number of compression rounds
 * @param d_rounds Number of finalization rounds
 * @return 64-bit hash value
 */
static uint64_t siphash_internal(const uint8_t *data, size_t len, AsthraSipHashKey key,
                                 int c_rounds, int d_rounds) {
    uint64_t v0 = key.k0 ^ 0x736f6d6570736575ULL;
    uint64_t v1 = key.k1 ^ 0x646f72616e646f6dULL;
    uint64_t v2 = key.k0 ^ 0x6c7967656e657261ULL;
    uint64_t v3 = key.k1 ^ 0x7465646279746573ULL;

    const uint8_t *end = data + len - (len % sizeof(uint64_t));
    const int left = len & 7;
    uint64_t b = ((uint64_t)len) << 56;

    // Process 64-bit blocks
    for (; data != end; data += 8) {
        uint64_t m = asthra_bytes_to_u64_le(data);
        v3 ^= m;

        // Compression rounds
        for (int i = 0; i < c_rounds; ++i) {
            sip_round(&v0, &v1, &v2, &v3);
        }

        v0 ^= m;
    }

    // Handle remaining bytes
    switch (left) {
    case 7:
        b |= ((uint64_t)data[6]) << 48; // fallthrough
    case 6:
        b |= ((uint64_t)data[5]) << 40; // fallthrough
    case 5:
        b |= ((uint64_t)data[4]) << 32; // fallthrough
    case 4:
        b |= ((uint64_t)data[3]) << 24; // fallthrough
    case 3:
        b |= ((uint64_t)data[2]) << 16; // fallthrough
    case 2:
        b |= ((uint64_t)data[1]) << 8; // fallthrough
    case 1:
        b |= ((uint64_t)data[0]); // fallthrough
    case 0:
        break;
    }

    v3 ^= b;

    // Final compression rounds
    for (int i = 0; i < c_rounds; ++i) {
        sip_round(&v0, &v1, &v2, &v3);
    }

    v0 ^= b;
    v2 ^= 0xff;

    // Finalization rounds
    for (int i = 0; i < d_rounds; ++i) {
        sip_round(&v0, &v1, &v2, &v3);
    }

    return v0 ^ v1 ^ v2 ^ v3;
}

// =============================================================================
// SIPHASH KEY GENERATION AND MANAGEMENT
// =============================================================================

AsthraSipHashKey asthra_siphash_generate_key(void) {
    AsthraSipHashKey key;
    key.k0 = asthra_random_u64();
    key.k1 = asthra_random_u64();
    return key;
}

AsthraSipHashKey asthra_siphash_key_from_u64(uint64_t k0, uint64_t k1) {
    AsthraSipHashKey key;
    key.k0 = k0;
    key.k1 = k1;
    return key;
}

AsthraResult asthra_siphash_key_from_bytes(const uint8_t *bytes) {
    if (!bytes) {
        return asthra_result_err_cstr("Null bytes pointer for SipHash key");
    }

    AsthraSipHashKey key;
    key.k0 = asthra_bytes_to_u64_le(bytes);
    key.k1 = asthra_bytes_to_u64_le(bytes + 8);

    AsthraSipHashKey *key_ptr = malloc(sizeof(AsthraSipHashKey));
    if (!key_ptr) {
        return asthra_result_err_cstr("Failed to allocate memory for SipHash key");
    }
    memcpy(key_ptr, &key, sizeof(AsthraSipHashKey));

    return asthra_result_ok(key_ptr, sizeof(AsthraSipHashKey), 0, ASTHRA_OWNERSHIP_C);
}

void asthra_siphash_key_to_bytes(AsthraSipHashKey key, uint8_t *bytes) {
    if (!bytes)
        return;

    asthra_u64_to_bytes_le(key.k0, bytes);
    asthra_u64_to_bytes_le(key.k1, bytes + 8);
}

// =============================================================================
// ONE-SHOT SIPHASH FUNCTIONS
// =============================================================================

uint64_t asthra_siphash_24(const uint8_t *data, size_t len, AsthraSipHashKey key) {
    return siphash_internal(data, len, key, 2, 4);
}

uint64_t asthra_siphash_13(const uint8_t *data, size_t len, AsthraSipHashKey key) {
    return siphash_internal(data, len, key, 1, 3);
}

uint64_t asthra_siphash_48(const uint8_t *data, size_t len, AsthraSipHashKey key) {
    return siphash_internal(data, len, key, 4, 8);
}

uint64_t asthra_siphash_hash_variant(const uint8_t *data, size_t len, AsthraSipHashKey key,
                                     AsthraSipHashVariant variant) {
    switch (variant) {
    case ASTHRA_SIPHASH_1_3:
        return asthra_siphash_13(data, len, key);
    case ASTHRA_SIPHASH_2_4:
        return asthra_siphash_24(data, len, key);
    case ASTHRA_SIPHASH_4_8:
        return asthra_siphash_48(data, len, key);
    default:
        return asthra_siphash_24(data, len, key); // Default to standard variant
    }
}

uint64_t asthra_siphash_hash_string(const char *str, AsthraSipHashKey key) {
    if (!str)
        return 0;
    return asthra_siphash_24((const uint8_t *)str, strlen(str), key);
}

uint64_t asthra_siphash_hash_string_random(const char *str) {
    AsthraSipHashKey key = asthra_siphash_generate_key();
    return asthra_siphash_hash_string(str, key);
}

// =============================================================================
// STREAMING SIPHASH INTERFACE
// =============================================================================

void asthra_siphash_init_context(AsthraSipHashContext *ctx, AsthraSipHashKey key,
                                 AsthraSipHashVariant variant) {
    if (!ctx)
        return;

    ctx->key = key;
    ctx->variant = variant;
    ctx->buffer_len = 0;
    ctx->total_len = 0;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));

    // Initialize SipHash state
    ctx->v0 = key.k0 ^ 0x736f6d6570736575ULL;
    ctx->v1 = key.k1 ^ 0x646f72616e646f6dULL;
    ctx->v2 = key.k0 ^ 0x6c7967656e657261ULL;
    ctx->v3 = key.k1 ^ 0x7465646279746573ULL;
}

void asthra_siphash_update(AsthraSipHashContext *ctx, const uint8_t *data, size_t len) {
    if (!ctx || !data)
        return;

    ctx->total_len += len;

    // Handle buffered data first
    if (ctx->buffer_len > 0) {
        size_t needed = 8 - ctx->buffer_len;
        size_t take = (len < needed) ? len : needed;

        memcpy(ctx->buffer + ctx->buffer_len, data, take);
        ctx->buffer_len += take;
        data += take;
        len -= take;

        if (ctx->buffer_len == 8) {
            uint64_t m = asthra_bytes_to_u64_le(ctx->buffer);
            ctx->v3 ^= m;

            // Apply compression rounds based on variant
            int c_rounds = (ctx->variant == ASTHRA_SIPHASH_1_3)   ? 1
                           : (ctx->variant == ASTHRA_SIPHASH_4_8) ? 4
                                                                  : 2;
            for (int i = 0; i < c_rounds; ++i) {
                sip_round(&ctx->v0, &ctx->v1, &ctx->v2, &ctx->v3);
            }

            ctx->v0 ^= m;
            ctx->buffer_len = 0;
        }
    }

    // Process full 8-byte blocks
    int c_rounds = (ctx->variant == ASTHRA_SIPHASH_1_3)   ? 1
                   : (ctx->variant == ASTHRA_SIPHASH_4_8) ? 4
                                                          : 2;

    while (len >= 8) {
        uint64_t m = asthra_bytes_to_u64_le(data);
        ctx->v3 ^= m;

        for (int i = 0; i < c_rounds; ++i) {
            sip_round(&ctx->v0, &ctx->v1, &ctx->v2, &ctx->v3);
        }

        ctx->v0 ^= m;
        data += 8;
        len -= 8;
    }

    // Buffer remaining bytes
    if (len > 0) {
        memcpy(ctx->buffer + ctx->buffer_len, data, len);
        ctx->buffer_len += len;
    }
}

uint64_t asthra_siphash_finalize(AsthraSipHashContext *ctx) {
    if (!ctx)
        return 0;

    uint64_t b = ((uint64_t)ctx->total_len) << 56;

    // Handle remaining buffered bytes
    for (size_t i = 0; i < ctx->buffer_len; ++i) {
        b |= ((uint64_t)ctx->buffer[i]) << (i * 8);
    }

    ctx->v3 ^= b;

    // Final compression rounds
    int c_rounds = (ctx->variant == ASTHRA_SIPHASH_1_3)   ? 1
                   : (ctx->variant == ASTHRA_SIPHASH_4_8) ? 4
                                                          : 2;
    for (int i = 0; i < c_rounds; ++i) {
        sip_round(&ctx->v0, &ctx->v1, &ctx->v2, &ctx->v3);
    }

    ctx->v0 ^= b;
    ctx->v2 ^= 0xff;

    // Finalization rounds
    int d_rounds = (ctx->variant == ASTHRA_SIPHASH_1_3)   ? 3
                   : (ctx->variant == ASTHRA_SIPHASH_4_8) ? 8
                                                          : 4;
    for (int i = 0; i < d_rounds; ++i) {
        sip_round(&ctx->v0, &ctx->v1, &ctx->v2, &ctx->v3);
    }

    return ctx->v0 ^ ctx->v1 ^ ctx->v2 ^ ctx->v3;
}

// =============================================================================
// GENERAL HASH FUNCTIONS
// =============================================================================

AsthraResult asthra_hash_bytes_with_config(const uint8_t *data, size_t len,
                                           const AsthrHashConfig *config) {
    if (!data || !config) {
        return asthra_result_err_cstr("Invalid parameters for hash_bytes_with_config");
    }

    uint64_t hash_value = 0;
    AsthraSipHashKey key;

    // Determine key based on configuration
    if (config->has_seed) {
        key = asthra_siphash_key_from_u64(config->seed, config->seed ^ 0xAAAAAAAAAAAAAAAAULL);
    } else if (config->use_random_seed) {
        key = asthra_siphash_generate_key();
    } else {
        // Default deterministic key for testing
        key = asthra_siphash_key_from_u64(0x0706050403020100ULL, 0x0f0e0d0c0b0a0908ULL);
    }

    // Apply hash function based on configuration
    switch (config->function) {
    case ASTHRA_HASH_SIPHASH_2_4:
        hash_value = asthra_siphash_24(data, len, key);
        break;
    case ASTHRA_HASH_SIPHASH_1_3:
        hash_value = asthra_siphash_13(data, len, key);
        break;
    case ASTHRA_HASH_FNV_1A:
        hash_value = asthra_fnv1a_hash(data, len);
        break;
    default:
        return asthra_result_err_cstr("Unsupported hash function");
    }

    return asthra_result_ok_uint64(hash_value);
}

AsthraResult asthra_hash_string_with_config(const char *str, const AsthrHashConfig *config) {
    if (!str) {
        return asthra_result_err_cstr("Null string for hash_string_with_config");
    }
    return asthra_hash_bytes_with_config((const uint8_t *)str, strlen(str), config);
}

uint64_t asthra_hash_bytes(const uint8_t *data, size_t len) {
    if (!data)
        return 0;
    AsthraSipHashKey key = asthra_siphash_generate_key();
    return asthra_siphash_24(data, len, key);
}

uint64_t asthra_hash_string(const char *str) {
    if (!str)
        return 0;
    return asthra_hash_bytes((const uint8_t *)str, strlen(str));
}

// =============================================================================
// HASH CONFIGURATION
// =============================================================================

AsthrHashConfig asthra_hash_default_config(void) {
    AsthrHashConfig config;
    config.function = ASTHRA_HASH_SIPHASH_2_4;
    config.seed = 0;
    config.has_seed = false;
    config.use_random_seed = true;
    return config;
}

AsthrHashConfig asthra_hash_config_with_function(AshtraHashFunction function) {
    AsthrHashConfig config;
    config.function = function;
    config.seed = 0;
    config.has_seed = false;
    config.use_random_seed = true;
    return config;
}

AsthrHashConfig asthra_hash_config_with_seed(AshtraHashFunction function, uint64_t seed) {
    AsthrHashConfig config;
    config.function = function;
    config.seed = seed;
    config.has_seed = true;
    config.use_random_seed = false;
    return config;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

uint64_t asthra_random_u64(void) {
    static bool seeded = false;
    if (!seeded) {
        srand((unsigned int)time(NULL));
        seeded = true;
    }

    uint64_t result = 0;
    for (int i = 0; i < 64; i += 15) {
        result = (result << 15) | (rand() & 0x7FFF);
    }
    return result;
}

uint64_t asthra_bytes_to_u64_le(const uint8_t *bytes) {
    if (!bytes)
        return 0;

    uint64_t result = 0;
    for (int i = 0; i < 8; ++i) {
        result |= ((uint64_t)bytes[i]) << (i * 8);
    }
    return result;
}

void asthra_u64_to_bytes_le(uint64_t value, uint8_t *bytes) {
    if (!bytes)
        return;

    for (int i = 0; i < 8; ++i) {
        bytes[i] = (uint8_t)(value >> (i * 8));
    }
}

const uint8_t *asthra_string_to_bytes(const char *str, size_t *len) {
    if (!str || !len) {
        if (len)
            *len = 0;
        return NULL;
    }

    *len = strlen(str);
    return (const uint8_t *)str;
}

const char *asthra_hash_function_name(AshtraHashFunction function) {
    switch (function) {
    case ASTHRA_HASH_SIPHASH_2_4:
        return "SipHash-2-4";
    case ASTHRA_HASH_SIPHASH_1_3:
        return "SipHash-1-3";
    case ASTHRA_HASH_FNV_1A:
        return "FNV-1a";
    case ASTHRA_HASH_CITYHASH:
        return "CityHash";
    default:
        return "Unknown";
    }
}

const char *asthra_siphash_variant_name(AsthraSipHashVariant variant) {
    switch (variant) {
    case ASTHRA_SIPHASH_1_3:
        return "SipHash-1-3";
    case ASTHRA_SIPHASH_2_4:
        return "SipHash-2-4";
    case ASTHRA_SIPHASH_4_8:
        return "SipHash-4-8";
    default:
        return "Unknown";
    }
}

// =============================================================================
// LEGACY/COMPATIBILITY FUNCTIONS
// =============================================================================

uint64_t asthra_fnv1a_hash(const uint8_t *data, size_t len) {
    if (!data)
        return 0;

    const uint64_t FNV_OFFSET_BASIS = 0xcbf29ce484222325ULL;
    const uint64_t FNV_PRIME = 0x100000001b3ULL;

    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < len; ++i) {
        hash ^= data[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

uint64_t asthra_fnv1a_hash_string(const char *str) {
    if (!str)
        return 0;
    return asthra_fnv1a_hash((const uint8_t *)str, strlen(str));
}
