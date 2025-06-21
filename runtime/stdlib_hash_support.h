/**
 * Asthra Programming Language Runtime - Hash Support Module
 * FFI Support for stdlib::hash package
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This module provides C runtime support for hash functions, specifically
 * SipHash implementation to support the stdlib::hash Asthra package.
 */

#ifndef ASTHRA_STDLIB_HASH_SUPPORT_H
#define ASTHRA_STDLIB_HASH_SUPPORT_H

#include "types/asthra_runtime_result.h"
#include "core/asthra_runtime_core.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SIPHASH TYPES AND STRUCTURES
// =============================================================================

/**
 * SipHash key structure (128-bit key)
 */
typedef struct {
    uint64_t k0;
    uint64_t k1;
} AsthraSipHashKey;

/**
 * SipHash variants enumeration
 */
typedef enum {
    ASTHRA_SIPHASH_1_3 = 0,  // Fast variant (1 compression round, 3 finalization)
    ASTHRA_SIPHASH_2_4 = 1,  // Standard variant (2 compression rounds, 4 finalization)
    ASTHRA_SIPHASH_4_8 = 2   // High-security variant (4 compression rounds, 8 finalization)
} AsthraSipHashVariant;

/**
 * SipHash context for streaming operations
 */
typedef struct {
    AsthraSipHashKey key;
    AsthraSipHashVariant variant;
    uint8_t buffer[8];
    size_t buffer_len;
    size_t total_len;
    uint64_t v0, v1, v2, v3;
} AsthraSipHashContext;

/**
 * Hash function types enumeration
 */
typedef enum {
    ASTHRA_HASH_SIPHASH_2_4 = 0,
    ASTHRA_HASH_SIPHASH_1_3 = 1,
    ASTHRA_HASH_FNV_1A = 2,
    ASTHRA_HASH_CITYHASH = 3
} AshtraHashFunction;

/**
 * Hash configuration structure
 */
typedef struct {
    AshtraHashFunction function;
    uint64_t seed;
    bool has_seed;
    bool use_random_seed;
} AsthrHashConfig;

// =============================================================================
// SIPHASH KEY GENERATION AND MANAGEMENT
// =============================================================================

/**
 * Generate random SipHash key
 * @return Random 128-bit SipHash key
 */
AsthraSipHashKey asthra_siphash_generate_key(void);

/**
 * Create SipHash key from two 64-bit values
 * @param k0 First 64-bit key component
 * @param k1 Second 64-bit key component
 * @return SipHash key structure
 */
AsthraSipHashKey asthra_siphash_key_from_u64(uint64_t k0, uint64_t k1);

/**
 * Create SipHash key from byte array (16 bytes)
 * @param bytes 16-byte array containing key data
 * @return Result containing SipHash key or error
 */
AsthraResult asthra_siphash_key_from_bytes(const uint8_t* bytes);

/**
 * Convert SipHash key to byte array
 * @param key SipHash key to convert
 * @param bytes Output buffer (must be at least 16 bytes)
 */
void asthra_siphash_key_to_bytes(AsthraSipHashKey key, uint8_t* bytes);

// =============================================================================
// ONE-SHOT SIPHASH FUNCTIONS
// =============================================================================

/**
 * Hash data using SipHash-2-4 (standard variant)
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param key SipHash key
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_24(const uint8_t* data, size_t len, AsthraSipHashKey key);

/**
 * Hash data using SipHash-1-3 (fast variant)
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param key SipHash key
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_13(const uint8_t* data, size_t len, AsthraSipHashKey key);

/**
 * Hash data using SipHash-4-8 (high-security variant)
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param key SipHash key
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_48(const uint8_t* data, size_t len, AsthraSipHashKey key);

/**
 * Hash data with specified SipHash variant
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param key SipHash key
 * @param variant SipHash variant to use
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_hash_variant(const uint8_t* data, size_t len, AsthraSipHashKey key, AsthraSipHashVariant variant);

/**
 * Hash string using SipHash-2-4
 * @param str Null-terminated string to hash
 * @param key SipHash key
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_hash_string(const char* str, AsthraSipHashKey key);

/**
 * Hash string with random key (convenience function)
 * @param str Null-terminated string to hash
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_hash_string_random(const char* str);

// =============================================================================
// STREAMING SIPHASH INTERFACE
// =============================================================================

/**
 * Initialize SipHash context for streaming
 * @param ctx Context to initialize
 * @param key SipHash key
 * @param variant SipHash variant to use
 */
void asthra_siphash_init_context(AsthraSipHashContext* ctx, AsthraSipHashKey key, AsthraSipHashVariant variant);

/**
 * Update SipHash context with more data
 * @param ctx SipHash context
 * @param data Data to add to hash
 * @param len Length of data in bytes
 */
void asthra_siphash_update(AsthraSipHashContext* ctx, const uint8_t* data, size_t len);

/**
 * Finalize SipHash context and get result
 * @param ctx SipHash context
 * @return 64-bit hash value
 */
uint64_t asthra_siphash_finalize(AsthraSipHashContext* ctx);

// =============================================================================
// GENERAL HASH FUNCTIONS
// =============================================================================

/**
 * Hash bytes using specified configuration
 * @param data Data to hash
 * @param len Length of data in bytes
 * @param config Hash configuration
 * @return Result containing hash value or error
 */
AsthraResult asthra_hash_bytes_with_config(const uint8_t* data, size_t len, const AsthrHashConfig* config);

/**
 * Hash string using specified configuration
 * @param str Null-terminated string to hash
 * @param config Hash configuration
 * @return Result containing hash value or error
 */
AsthraResult asthra_hash_string_with_config(const char* str, const AsthrHashConfig* config);

/**
 * Hash bytes using default SipHash-2-4 with random key
 * @param data Data to hash
 * @param len Length of data in bytes
 * @return 64-bit hash value
 */
uint64_t asthra_hash_bytes(const uint8_t* data, size_t len);

/**
 * Hash string using default SipHash-2-4 with random key
 * @param str Null-terminated string to hash
 * @return 64-bit hash value
 */
uint64_t asthra_hash_string(const char* str);

// =============================================================================
// HASH CONFIGURATION
// =============================================================================

/**
 * Create default hash configuration (SipHash-2-4 with random seed)
 * @return Default hash configuration
 */
AsthrHashConfig asthra_hash_default_config(void);

/**
 * Create hash configuration with specific function
 * @param function Hash function to use
 * @return Hash configuration
 */
AsthrHashConfig asthra_hash_config_with_function(AshtraHashFunction function);

/**
 * Create hash configuration with custom seed
 * @param function Hash function to use
 * @param seed Custom seed value
 * @return Hash configuration
 */
AsthrHashConfig asthra_hash_config_with_seed(AshtraHashFunction function, uint64_t seed);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Generate random 64-bit value
 * @return Random 64-bit unsigned integer
 */
uint64_t asthra_random_u64(void);

/**
 * Convert 8 bytes to 64-bit little-endian value
 * @param bytes Input byte array (at least 8 bytes)
 * @return 64-bit value in host byte order
 */
uint64_t asthra_bytes_to_u64_le(const uint8_t* bytes);

/**
 * Convert 64-bit value to 8 bytes in little-endian format
 * @param value 64-bit value to convert
 * @param bytes Output buffer (at least 8 bytes)
 */
void asthra_u64_to_bytes_le(uint64_t value, uint8_t* bytes);

/**
 * Convert string to byte array (for hashing)
 * @param str Input string
 * @param len Output parameter for length
 * @return Pointer to byte representation of string
 */
const uint8_t* asthra_string_to_bytes(const char* str, size_t* len);

/**
 * Get hash function name as string
 * @param function Hash function type
 * @return String representation of hash function
 */
const char* asthra_hash_function_name(AshtraHashFunction function);

/**
 * Get SipHash variant name as string
 * @param variant SipHash variant
 * @return String representation of SipHash variant
 */
const char* asthra_siphash_variant_name(AsthraSipHashVariant variant);

// =============================================================================
// LEGACY/COMPATIBILITY FUNCTIONS
// =============================================================================

/**
 * FNV-1a hash for compatibility (simple implementation)
 * @param data Data to hash
 * @param len Length of data in bytes
 * @return 64-bit hash value
 */
uint64_t asthra_fnv1a_hash(const uint8_t* data, size_t len);

/**
 * FNV-1a hash for strings
 * @param str Null-terminated string to hash
 * @return 64-bit hash value
 */
uint64_t asthra_fnv1a_hash_string(const char* str);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_STDLIB_HASH_SUPPORT_H 
