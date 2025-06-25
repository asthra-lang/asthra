/**
 * Asthra Programming Language Runtime
 * 128-bit Integer Support
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This header provides runtime support for 128-bit integer operations (i128/u128).
 * It includes type definitions, arithmetic operations, conversions, and utilities
 * for working with 128-bit integers in Asthra programs.
 */

#ifndef ASTHRA_INT128_H
#define ASTHRA_INT128_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TYPE DEFINITIONS
// =============================================================================

/**
 * Platform-specific 128-bit integer types
 * Use compiler builtins when available, otherwise emulate with two 64-bit integers
 */
#if defined(__SIZEOF_INT128__) && __SIZEOF_INT128__ == 16
// Clang supports native 128-bit integers
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#endif
typedef __int128 asthra_i128;
typedef unsigned __int128 asthra_u128;
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#define ASTHRA_HAS_NATIVE_INT128 1
#else
// Emulate 128-bit integers with two 64-bit values
typedef struct {
    uint64_t low; // Lower 64 bits
    int64_t high; // Upper 64 bits (signed for i128)
} asthra_i128;

typedef struct {
    uint64_t low;  // Lower 64 bits
    uint64_t high; // Upper 64 bits
} asthra_u128;

#define ASTHRA_HAS_NATIVE_INT128 0
#endif

// =============================================================================
// CONSTANTS
// =============================================================================

// Maximum and minimum values for 128-bit integers
extern const asthra_i128 ASTHRA_I128_MAX;
extern const asthra_i128 ASTHRA_I128_MIN;
extern const asthra_u128 ASTHRA_U128_MAX;
extern const asthra_u128 ASTHRA_U128_MIN;

// =============================================================================
// ARITHMETIC OPERATIONS - SIGNED (i128)
// =============================================================================

/**
 * Add two signed 128-bit integers
 * @param a First operand
 * @param b Second operand
 * @return a + b
 */
asthra_i128 asthra_i128_add(asthra_i128 a, asthra_i128 b);

/**
 * Subtract two signed 128-bit integers
 * @param a First operand
 * @param b Second operand
 * @return a - b
 */
asthra_i128 asthra_i128_sub(asthra_i128 a, asthra_i128 b);

/**
 * Multiply two signed 128-bit integers
 * @param a First operand
 * @param b Second operand
 * @return a * b
 */
asthra_i128 asthra_i128_mul(asthra_i128 a, asthra_i128 b);

/**
 * Divide two signed 128-bit integers
 * @param a Dividend
 * @param b Divisor
 * @return a / b (returns 0 if b is 0)
 */
asthra_i128 asthra_i128_div(asthra_i128 a, asthra_i128 b);

/**
 * Modulo operation for signed 128-bit integers
 * @param a Dividend
 * @param b Divisor
 * @return a % b (returns 0 if b is 0)
 */
asthra_i128 asthra_i128_mod(asthra_i128 a, asthra_i128 b);

/**
 * Negate a signed 128-bit integer
 * @param a Value to negate
 * @return -a
 */
asthra_i128 asthra_i128_neg(asthra_i128 a);

// =============================================================================
// ARITHMETIC OPERATIONS - UNSIGNED (u128)
// =============================================================================

/**
 * Add two unsigned 128-bit integers
 * @param a First operand
 * @param b Second operand
 * @return a + b
 */
asthra_u128 asthra_u128_add(asthra_u128 a, asthra_u128 b);

/**
 * Subtract two unsigned 128-bit integers
 * @param a First operand
 * @param b Second operand
 * @return a - b
 */
asthra_u128 asthra_u128_sub(asthra_u128 a, asthra_u128 b);

/**
 * Multiply two unsigned 128-bit integers
 * @param a First operand
 * @param b Second operand
 * @return a * b
 */
asthra_u128 asthra_u128_mul(asthra_u128 a, asthra_u128 b);

/**
 * Divide two unsigned 128-bit integers
 * @param a Dividend
 * @param b Divisor
 * @return a / b (returns 0 if b is 0)
 */
asthra_u128 asthra_u128_div(asthra_u128 a, asthra_u128 b);

/**
 * Modulo operation for unsigned 128-bit integers
 * @param a Dividend
 * @param b Divisor
 * @return a % b (returns 0 if b is 0)
 */
asthra_u128 asthra_u128_mod(asthra_u128 a, asthra_u128 b);

// =============================================================================
// BITWISE OPERATIONS
// =============================================================================

// Signed bitwise operations
asthra_i128 asthra_i128_and(asthra_i128 a, asthra_i128 b);
asthra_i128 asthra_i128_or(asthra_i128 a, asthra_i128 b);
asthra_i128 asthra_i128_xor(asthra_i128 a, asthra_i128 b);
asthra_i128 asthra_i128_not(asthra_i128 a);
asthra_i128 asthra_i128_shl(asthra_i128 a, int shift);
asthra_i128 asthra_i128_shr(asthra_i128 a, int shift);

// Unsigned bitwise operations
asthra_u128 asthra_u128_and(asthra_u128 a, asthra_u128 b);
asthra_u128 asthra_u128_or(asthra_u128 a, asthra_u128 b);
asthra_u128 asthra_u128_xor(asthra_u128 a, asthra_u128 b);
asthra_u128 asthra_u128_not(asthra_u128 a);
asthra_u128 asthra_u128_shl(asthra_u128 a, int shift);
asthra_u128 asthra_u128_shr(asthra_u128 a, int shift);

// =============================================================================
// COMPARISON OPERATIONS
// =============================================================================

// Signed comparisons
bool asthra_i128_eq(asthra_i128 a, asthra_i128 b);
bool asthra_i128_ne(asthra_i128 a, asthra_i128 b);
bool asthra_i128_lt(asthra_i128 a, asthra_i128 b);
bool asthra_i128_le(asthra_i128 a, asthra_i128 b);
bool asthra_i128_gt(asthra_i128 a, asthra_i128 b);
bool asthra_i128_ge(asthra_i128 a, asthra_i128 b);

// Unsigned comparisons
bool asthra_u128_eq(asthra_u128 a, asthra_u128 b);
bool asthra_u128_ne(asthra_u128 a, asthra_u128 b);
bool asthra_u128_lt(asthra_u128 a, asthra_u128 b);
bool asthra_u128_le(asthra_u128 a, asthra_u128 b);
bool asthra_u128_gt(asthra_u128 a, asthra_u128 b);
bool asthra_u128_ge(asthra_u128 a, asthra_u128 b);

// =============================================================================
// CONVERSION FUNCTIONS
// =============================================================================

// From smaller integer types to 128-bit
asthra_i128 asthra_i128_from_i64(int64_t value);
asthra_i128 asthra_i128_from_u64(uint64_t value);
asthra_u128 asthra_u128_from_u64(uint64_t value);

// From 128-bit to smaller types (with truncation)
int64_t asthra_i128_to_i64(asthra_i128 value);
uint64_t asthra_i128_to_u64(asthra_i128 value);
uint64_t asthra_u128_to_u64(asthra_u128 value);

// Between signed and unsigned 128-bit
asthra_u128 asthra_i128_to_u128(asthra_i128 value);
asthra_i128 asthra_u128_to_i128(asthra_u128 value);

// =============================================================================
// STRING CONVERSION
// =============================================================================

/**
 * Convert i128 to string
 * @param value Value to convert
 * @param buffer Output buffer (must be at least 41 bytes for max value + sign + null)
 * @param base Number base (2, 8, 10, or 16)
 * @return Number of characters written (excluding null terminator)
 */
int asthra_i128_to_string(asthra_i128 value, char *buffer, int base);

/**
 * Convert u128 to string
 * @param value Value to convert
 * @param buffer Output buffer (must be at least 40 bytes for max value + null)
 * @param base Number base (2, 8, 10, or 16)
 * @return Number of characters written (excluding null terminator)
 */
int asthra_u128_to_string(asthra_u128 value, char *buffer, int base);

/**
 * Parse i128 from string
 * @param str Input string
 * @param endptr If not NULL, set to point to first non-parsed character
 * @param base Number base (0 for auto-detect, or 2-36)
 * @return Parsed value (0 on error)
 */
asthra_i128 asthra_i128_from_string(const char *str, char **endptr, int base);

/**
 * Parse u128 from string
 * @param str Input string
 * @param endptr If not NULL, set to point to first non-parsed character
 * @param base Number base (0 for auto-detect, or 2-36)
 * @return Parsed value (0 on error)
 */
asthra_u128 asthra_u128_from_string(const char *str, char **endptr, int base);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Check if a 128-bit value is zero
 */
bool asthra_i128_is_zero(asthra_i128 value);
bool asthra_u128_is_zero(asthra_u128 value);

/**
 * Get the number of leading zeros in a 128-bit value
 */
int asthra_i128_clz(asthra_i128 value);
int asthra_u128_clz(asthra_u128 value);

/**
 * Get the number of trailing zeros in a 128-bit value
 */
int asthra_i128_ctz(asthra_i128 value);
int asthra_u128_ctz(asthra_u128 value);

/**
 * Count the number of set bits (population count)
 */
int asthra_i128_popcount(asthra_i128 value);
int asthra_u128_popcount(asthra_u128 value);

// =============================================================================
// OVERFLOW CHECKING
// =============================================================================

/**
 * Checked arithmetic operations that detect overflow
 * Return true if operation succeeded without overflow
 */
bool asthra_i128_add_overflow(asthra_i128 a, asthra_i128 b, asthra_i128 *result);
bool asthra_i128_sub_overflow(asthra_i128 a, asthra_i128 b, asthra_i128 *result);
bool asthra_i128_mul_overflow(asthra_i128 a, asthra_i128 b, asthra_i128 *result);

bool asthra_u128_add_overflow(asthra_u128 a, asthra_u128 b, asthra_u128 *result);
bool asthra_u128_sub_overflow(asthra_u128 a, asthra_u128 b, asthra_u128 *result);
bool asthra_u128_mul_overflow(asthra_u128 a, asthra_u128 b, asthra_u128 *result);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_INT128_H