/**
 * Asthra Programming Language Runtime
 * 128-bit Integer Support Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file implements runtime support for 128-bit integer operations.
 * It provides both native implementations (when available) and emulated
 * implementations for platforms without native 128-bit support.
 */

#include "asthra_int128.h"
#include <string.h>
#include <stdlib.h>
#include <limits.h>

// =============================================================================
// CONSTANTS
// =============================================================================

#if ASTHRA_HAS_NATIVE_INT128
    const asthra_i128 ASTHRA_I128_MAX = ((asthra_i128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const asthra_i128 ASTHRA_I128_MIN = -(((asthra_i128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL) - 1;
    const asthra_u128 ASTHRA_U128_MAX = ~(asthra_u128)0;
    const asthra_u128 ASTHRA_U128_MIN = 0;
#else
    const asthra_i128 ASTHRA_I128_MAX = {.low = 0xFFFFFFFFFFFFFFFFULL, .high = 0x7FFFFFFFFFFFFFFFLL};
    const asthra_i128 ASTHRA_I128_MIN = {.low = 0, .high = (int64_t)0x8000000000000000LL};
    const asthra_u128 ASTHRA_U128_MAX = {.low = 0xFFFFFFFFFFFFFFFFULL, .high = 0xFFFFFFFFFFFFFFFFULL};
    const asthra_u128 ASTHRA_U128_MIN = {.low = 0, .high = 0};
#endif

// =============================================================================
// ARITHMETIC OPERATIONS - NATIVE IMPLEMENTATION
// =============================================================================

#if ASTHRA_HAS_NATIVE_INT128

asthra_i128 asthra_i128_add(asthra_i128 a, asthra_i128 b) {
    return a + b;
}

asthra_i128 asthra_i128_sub(asthra_i128 a, asthra_i128 b) {
    return a - b;
}

asthra_i128 asthra_i128_mul(asthra_i128 a, asthra_i128 b) {
    return a * b;
}

asthra_i128 asthra_i128_div(asthra_i128 a, asthra_i128 b) {
    if (b == 0) return 0;
    return a / b;
}

asthra_i128 asthra_i128_mod(asthra_i128 a, asthra_i128 b) {
    if (b == 0) return 0;
    return a % b;
}

asthra_i128 asthra_i128_neg(asthra_i128 a) {
    return -a;
}

asthra_u128 asthra_u128_add(asthra_u128 a, asthra_u128 b) {
    return a + b;
}

asthra_u128 asthra_u128_sub(asthra_u128 a, asthra_u128 b) {
    return a - b;
}

asthra_u128 asthra_u128_mul(asthra_u128 a, asthra_u128 b) {
    return a * b;
}

asthra_u128 asthra_u128_div(asthra_u128 a, asthra_u128 b) {
    if (b == 0) return 0;
    return a / b;
}

asthra_u128 asthra_u128_mod(asthra_u128 a, asthra_u128 b) {
    if (b == 0) return 0;
    return a % b;
}

#else

// =============================================================================
// ARITHMETIC OPERATIONS - EMULATED IMPLEMENTATION
// =============================================================================

asthra_i128 asthra_i128_add(asthra_i128 a, asthra_i128 b) {
    asthra_i128 result;
    result.low = a.low + b.low;
    result.high = a.high + b.high + (result.low < a.low ? 1 : 0);
    return result;
}

asthra_i128 asthra_i128_sub(asthra_i128 a, asthra_i128 b) {
    asthra_i128 result;
    result.low = a.low - b.low;
    result.high = a.high - b.high - (a.low < b.low ? 1 : 0);
    return result;
}

asthra_i128 asthra_i128_mul(asthra_i128 a, asthra_i128 b) {
    // Simplified multiplication - doesn't handle overflow
    asthra_i128 result;
    uint64_t a_low = a.low;
    uint64_t b_low = b.low;
    int64_t a_high = a.high;
    int64_t b_high = b.high;
    
    // Calculate low * low
    result.low = a_low * b_low;
    
    // Calculate cross products and high * low products
    uint64_t cross1 = (uint64_t)a_high * b_low;
    uint64_t cross2 = a_low * (uint64_t)b_high;
    
    // Add to high part (ignoring overflow)
    result.high = cross1 + cross2;
    
    return result;
}

asthra_i128 asthra_i128_div(asthra_i128 a, asthra_i128 b) {
    // Simplified division - only handles small divisors
    if (b.high == 0 && b.low == 0) {
        asthra_i128 zero = {0, 0};
        return zero;
    }
    
    // For now, only handle division by values that fit in 64 bits
    if (b.high != 0 && b.high != -1) {
        asthra_i128 zero = {0, 0};
        return zero;
    }
    
    // Simple case: divide by small positive number
    if (b.high == 0) {
        asthra_i128 result;
        result.high = a.high / (int64_t)b.low;
        result.low = a.low / b.low;
        return result;
    }
    
    // Return zero for complex cases
    asthra_i128 zero = {0, 0};
    return zero;
}

asthra_i128 asthra_i128_mod(asthra_i128 a, asthra_i128 b) {
    // Simplified modulo - only handles small divisors
    if (b.high == 0 && b.low == 0) {
        asthra_i128 zero = {0, 0};
        return zero;
    }
    
    // For now, only handle modulo by values that fit in 64 bits
    if (b.high == 0) {
        asthra_i128 result;
        result.high = 0;
        result.low = a.low % b.low;
        return result;
    }
    
    // Return zero for complex cases
    asthra_i128 zero = {0, 0};
    return zero;
}

asthra_i128 asthra_i128_neg(asthra_i128 a) {
    asthra_i128 result;
    result.low = ~a.low + 1;
    result.high = ~a.high + (result.low == 0 ? 1 : 0);
    return result;
}

asthra_u128 asthra_u128_add(asthra_u128 a, asthra_u128 b) {
    asthra_u128 result;
    result.low = a.low + b.low;
    result.high = a.high + b.high + (result.low < a.low ? 1 : 0);
    return result;
}

asthra_u128 asthra_u128_sub(asthra_u128 a, asthra_u128 b) {
    asthra_u128 result;
    result.low = a.low - b.low;
    result.high = a.high - b.high - (a.low < b.low ? 1 : 0);
    return result;
}

asthra_u128 asthra_u128_mul(asthra_u128 a, asthra_u128 b) {
    // Simplified multiplication - doesn't handle overflow
    asthra_u128 result;
    
    // Calculate low * low
    result.low = a.low * b.low;
    
    // Calculate cross products
    uint64_t cross1 = a.high * b.low;
    uint64_t cross2 = a.low * b.high;
    
    // Add to high part (ignoring overflow)
    result.high = cross1 + cross2;
    
    return result;
}

asthra_u128 asthra_u128_div(asthra_u128 a, asthra_u128 b) {
    // Simplified division - only handles small divisors
    if (b.high == 0 && b.low == 0) {
        asthra_u128 zero = {0, 0};
        return zero;
    }
    
    // For now, only handle division by values that fit in 64 bits
    if (b.high == 0) {
        asthra_u128 result;
        result.high = a.high / b.low;
        result.low = a.low / b.low;
        return result;
    }
    
    // Return zero for complex cases
    asthra_u128 zero = {0, 0};
    return zero;
}

asthra_u128 asthra_u128_mod(asthra_u128 a, asthra_u128 b) {
    // Simplified modulo - only handles small divisors
    if (b.high == 0 && b.low == 0) {
        asthra_u128 zero = {0, 0};
        return zero;
    }
    
    // For now, only handle modulo by values that fit in 64 bits
    if (b.high == 0) {
        asthra_u128 result;
        result.high = 0;
        result.low = a.low % b.low;
        return result;
    }
    
    // Return zero for complex cases
    asthra_u128 zero = {0, 0};
    return zero;
}

#endif // ASTHRA_HAS_NATIVE_INT128

// =============================================================================
// BITWISE OPERATIONS
// =============================================================================

#if ASTHRA_HAS_NATIVE_INT128

asthra_i128 asthra_i128_and(asthra_i128 a, asthra_i128 b) {
    return a & b;
}

asthra_i128 asthra_i128_or(asthra_i128 a, asthra_i128 b) {
    return a | b;
}

asthra_i128 asthra_i128_xor(asthra_i128 a, asthra_i128 b) {
    return a ^ b;
}

asthra_i128 asthra_i128_not(asthra_i128 a) {
    return ~a;
}

asthra_i128 asthra_i128_shl(asthra_i128 a, int shift) {
    if (shift < 0 || shift >= 128) return a;
    return a << shift;
}

asthra_i128 asthra_i128_shr(asthra_i128 a, int shift) {
    if (shift < 0 || shift >= 128) return a;
    return a >> shift;
}

asthra_u128 asthra_u128_and(asthra_u128 a, asthra_u128 b) {
    return a & b;
}

asthra_u128 asthra_u128_or(asthra_u128 a, asthra_u128 b) {
    return a | b;
}

asthra_u128 asthra_u128_xor(asthra_u128 a, asthra_u128 b) {
    return a ^ b;
}

asthra_u128 asthra_u128_not(asthra_u128 a) {
    return ~a;
}

asthra_u128 asthra_u128_shl(asthra_u128 a, int shift) {
    if (shift < 0 || shift >= 128) return a;
    return a << shift;
}

asthra_u128 asthra_u128_shr(asthra_u128 a, int shift) {
    if (shift < 0 || shift >= 128) return a;
    return a >> shift;
}

#else

asthra_i128 asthra_i128_and(asthra_i128 a, asthra_i128 b) {
    asthra_i128 result;
    result.low = a.low & b.low;
    result.high = a.high & b.high;
    return result;
}

asthra_i128 asthra_i128_or(asthra_i128 a, asthra_i128 b) {
    asthra_i128 result;
    result.low = a.low | b.low;
    result.high = a.high | b.high;
    return result;
}

asthra_i128 asthra_i128_xor(asthra_i128 a, asthra_i128 b) {
    asthra_i128 result;
    result.low = a.low ^ b.low;
    result.high = a.high ^ b.high;
    return result;
}

asthra_i128 asthra_i128_not(asthra_i128 a) {
    asthra_i128 result;
    result.low = ~a.low;
    result.high = ~a.high;
    return result;
}

asthra_i128 asthra_i128_shl(asthra_i128 a, int shift) {
    asthra_i128 result;
    if (shift < 0 || shift >= 128) return a;
    
    if (shift >= 64) {
        result.low = 0;
        result.high = (int64_t)(a.low << (shift - 64));
    } else if (shift == 0) {
        return a;
    } else {
        result.low = a.low << shift;
        result.high = (a.high << shift) | (int64_t)(a.low >> (64 - shift));
    }
    return result;
}

asthra_i128 asthra_i128_shr(asthra_i128 a, int shift) {
    asthra_i128 result;
    if (shift < 0 || shift >= 128) return a;
    
    if (shift >= 64) {
        result.high = a.high >> 63; // Sign extend
        result.low = (uint64_t)(a.high >> (shift - 64));
    } else if (shift == 0) {
        return a;
    } else {
        result.high = a.high >> shift;
        result.low = (a.low >> shift) | ((uint64_t)a.high << (64 - shift));
    }
    return result;
}

asthra_u128 asthra_u128_and(asthra_u128 a, asthra_u128 b) {
    asthra_u128 result;
    result.low = a.low & b.low;
    result.high = a.high & b.high;
    return result;
}

asthra_u128 asthra_u128_or(asthra_u128 a, asthra_u128 b) {
    asthra_u128 result;
    result.low = a.low | b.low;
    result.high = a.high | b.high;
    return result;
}

asthra_u128 asthra_u128_xor(asthra_u128 a, asthra_u128 b) {
    asthra_u128 result;
    result.low = a.low ^ b.low;
    result.high = a.high ^ b.high;
    return result;
}

asthra_u128 asthra_u128_not(asthra_u128 a) {
    asthra_u128 result;
    result.low = ~a.low;
    result.high = ~a.high;
    return result;
}

asthra_u128 asthra_u128_shl(asthra_u128 a, int shift) {
    asthra_u128 result;
    if (shift < 0 || shift >= 128) return a;
    
    if (shift >= 64) {
        result.low = 0;
        result.high = a.low << (shift - 64);
    } else if (shift == 0) {
        return a;
    } else {
        result.low = a.low << shift;
        result.high = (a.high << shift) | (a.low >> (64 - shift));
    }
    return result;
}

asthra_u128 asthra_u128_shr(asthra_u128 a, int shift) {
    asthra_u128 result;
    if (shift < 0 || shift >= 128) return a;
    
    if (shift >= 64) {
        result.high = 0;
        result.low = a.high >> (shift - 64);
    } else if (shift == 0) {
        return a;
    } else {
        result.high = a.high >> shift;
        result.low = (a.low >> shift) | (a.high << (64 - shift));
    }
    return result;
}

#endif // ASTHRA_HAS_NATIVE_INT128

// =============================================================================
// COMPARISON OPERATIONS
// =============================================================================

#if ASTHRA_HAS_NATIVE_INT128

bool asthra_i128_eq(asthra_i128 a, asthra_i128 b) {
    return a == b;
}

bool asthra_i128_ne(asthra_i128 a, asthra_i128 b) {
    return a != b;
}

bool asthra_i128_lt(asthra_i128 a, asthra_i128 b) {
    return a < b;
}

bool asthra_i128_le(asthra_i128 a, asthra_i128 b) {
    return a <= b;
}

bool asthra_i128_gt(asthra_i128 a, asthra_i128 b) {
    return a > b;
}

bool asthra_i128_ge(asthra_i128 a, asthra_i128 b) {
    return a >= b;
}

bool asthra_u128_eq(asthra_u128 a, asthra_u128 b) {
    return a == b;
}

bool asthra_u128_ne(asthra_u128 a, asthra_u128 b) {
    return a != b;
}

bool asthra_u128_lt(asthra_u128 a, asthra_u128 b) {
    return a < b;
}

bool asthra_u128_le(asthra_u128 a, asthra_u128 b) {
    return a <= b;
}

bool asthra_u128_gt(asthra_u128 a, asthra_u128 b) {
    return a > b;
}

bool asthra_u128_ge(asthra_u128 a, asthra_u128 b) {
    return a >= b;
}

#else

bool asthra_i128_eq(asthra_i128 a, asthra_i128 b) {
    return a.high == b.high && a.low == b.low;
}

bool asthra_i128_ne(asthra_i128 a, asthra_i128 b) {
    return a.high != b.high || a.low != b.low;
}

bool asthra_i128_lt(asthra_i128 a, asthra_i128 b) {
    if (a.high != b.high) {
        return a.high < b.high;
    }
    return a.low < b.low;
}

bool asthra_i128_le(asthra_i128 a, asthra_i128 b) {
    return asthra_i128_eq(a, b) || asthra_i128_lt(a, b);
}

bool asthra_i128_gt(asthra_i128 a, asthra_i128 b) {
    return asthra_i128_lt(b, a);
}

bool asthra_i128_ge(asthra_i128 a, asthra_i128 b) {
    return asthra_i128_eq(a, b) || asthra_i128_gt(a, b);
}

bool asthra_u128_eq(asthra_u128 a, asthra_u128 b) {
    return a.high == b.high && a.low == b.low;
}

bool asthra_u128_ne(asthra_u128 a, asthra_u128 b) {
    return a.high != b.high || a.low != b.low;
}

bool asthra_u128_lt(asthra_u128 a, asthra_u128 b) {
    if (a.high != b.high) {
        return a.high < b.high;
    }
    return a.low < b.low;
}

bool asthra_u128_le(asthra_u128 a, asthra_u128 b) {
    return asthra_u128_eq(a, b) || asthra_u128_lt(a, b);
}

bool asthra_u128_gt(asthra_u128 a, asthra_u128 b) {
    return asthra_u128_lt(b, a);
}

bool asthra_u128_ge(asthra_u128 a, asthra_u128 b) {
    return asthra_u128_eq(a, b) || asthra_u128_gt(a, b);
}

#endif // ASTHRA_HAS_NATIVE_INT128

// =============================================================================
// CONVERSION FUNCTIONS
// =============================================================================

#if ASTHRA_HAS_NATIVE_INT128

asthra_i128 asthra_i128_from_i64(int64_t value) {
    return (asthra_i128)value;
}

asthra_i128 asthra_i128_from_u64(uint64_t value) {
    return (asthra_i128)value;
}

asthra_u128 asthra_u128_from_u64(uint64_t value) {
    return (asthra_u128)value;
}

int64_t asthra_i128_to_i64(asthra_i128 value) {
    return (int64_t)value;
}

uint64_t asthra_i128_to_u64(asthra_i128 value) {
    return (uint64_t)value;
}

uint64_t asthra_u128_to_u64(asthra_u128 value) {
    return (uint64_t)value;
}

asthra_u128 asthra_i128_to_u128(asthra_i128 value) {
    return (asthra_u128)value;
}

asthra_i128 asthra_u128_to_i128(asthra_u128 value) {
    return (asthra_i128)value;
}

#else

asthra_i128 asthra_i128_from_i64(int64_t value) {
    asthra_i128 result;
    result.low = (uint64_t)value;
    result.high = value < 0 ? -1 : 0;
    return result;
}

asthra_i128 asthra_i128_from_u64(uint64_t value) {
    asthra_i128 result;
    result.low = value;
    result.high = 0;
    return result;
}

asthra_u128 asthra_u128_from_u64(uint64_t value) {
    asthra_u128 result;
    result.low = value;
    result.high = 0;
    return result;
}

int64_t asthra_i128_to_i64(asthra_i128 value) {
    return (int64_t)value.low;
}

uint64_t asthra_i128_to_u64(asthra_i128 value) {
    return value.low;
}

uint64_t asthra_u128_to_u64(asthra_u128 value) {
    return value.low;
}

asthra_u128 asthra_i128_to_u128(asthra_i128 value) {
    asthra_u128 result;
    result.low = value.low;
    result.high = (uint64_t)value.high;
    return result;
}

asthra_i128 asthra_u128_to_i128(asthra_u128 value) {
    asthra_i128 result;
    result.low = value.low;
    result.high = (int64_t)value.high;
    return result;
}

#endif // ASTHRA_HAS_NATIVE_INT128

// =============================================================================
// STRING CONVERSION
// =============================================================================

int asthra_i128_to_string(asthra_i128 value, char *buffer, int base) {
    if (!buffer || (base != 2 && base != 8 && base != 10 && base != 16)) {
        return 0;
    }
    
    // Handle zero case
    if (asthra_i128_is_zero(value)) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }
    
    // Handle negative numbers for base 10
    bool is_negative = false;
    if (base == 10 && asthra_i128_lt(value, asthra_i128_from_i64(0))) {
        is_negative = true;
        value = asthra_i128_neg(value);
    }
    
    // Convert to string (reversed)
    char temp[130]; // Max binary representation + null
    int pos = 0;
    
#if ASTHRA_HAS_NATIVE_INT128
    asthra_u128 uvalue = (asthra_u128)value;
    const char *digits = "0123456789ABCDEF";
    
    while (uvalue > 0) {
        temp[pos++] = digits[uvalue % base];
        uvalue /= base;
    }
#else
    // Simplified conversion for emulated mode
    asthra_u128 uvalue = asthra_i128_to_u128(value);
    const char *digits = "0123456789ABCDEF";
    
    // Only handle base 10 and 16 for now
    if (base == 10 || base == 16) {
        while (!asthra_u128_is_zero(uvalue)) {
            uint64_t remainder = uvalue.low % base;
            temp[pos++] = digits[remainder];
            
            // Divide by base (simplified)
            if (base == 10) {
                uvalue.low /= 10;
                if (uvalue.high > 0) {
                    uint64_t carry = uvalue.high % 10;
                    uvalue.high /= 10;
                    uvalue.low += carry * (UINT64_MAX / 10 + 1);
                }
            } else if (base == 16) {
                uvalue.low >>= 4;
                uvalue.low |= (uvalue.high & 0xF) << 60;
                uvalue.high >>= 4;
            }
        }
    }
#endif
    
    // Add sign if needed
    int start = 0;
    if (is_negative) {
        buffer[0] = '-';
        start = 1;
    }
    
    // Reverse the string
    for (int i = 0; i < pos; i++) {
        buffer[start + i] = temp[pos - 1 - i];
    }
    buffer[start + pos] = '\0';
    
    return start + pos;
}

int asthra_u128_to_string(asthra_u128 value, char *buffer, int base) {
    if (!buffer || (base != 2 && base != 8 && base != 10 && base != 16)) {
        return 0;
    }
    
    // Handle zero case
    if (asthra_u128_is_zero(value)) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }
    
    // Convert to string (reversed)
    char temp[130]; // Max binary representation + null
    int pos = 0;
    
#if ASTHRA_HAS_NATIVE_INT128
    const char *digits = "0123456789ABCDEF";
    
    while (value > 0) {
        temp[pos++] = digits[value % base];
        value /= base;
    }
#else
    const char *digits = "0123456789ABCDEF";
    
    // Only handle base 10 and 16 for now
    if (base == 10 || base == 16) {
        while (!asthra_u128_is_zero(value)) {
            uint64_t remainder = value.low % base;
            temp[pos++] = digits[remainder];
            
            // Divide by base (simplified)
            if (base == 10) {
                value.low /= 10;
                if (value.high > 0) {
                    uint64_t carry = value.high % 10;
                    value.high /= 10;
                    value.low += carry * (UINT64_MAX / 10 + 1);
                }
            } else if (base == 16) {
                value.low >>= 4;
                value.low |= (value.high & 0xF) << 60;
                value.high >>= 4;
            }
        }
    }
#endif
    
    // Reverse the string
    for (int i = 0; i < pos; i++) {
        buffer[i] = temp[pos - 1 - i];
    }
    buffer[pos] = '\0';
    
    return pos;
}

asthra_i128 asthra_i128_from_string(const char *str, char **endptr, int base) {
    // Simplified implementation - only handles base 10
    if (!str || base != 10) {
        if (endptr) *endptr = (char *)str;
        return asthra_i128_from_i64(0);
    }
    
    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    // Handle sign
    bool is_negative = false;
    if (*str == '-') {
        is_negative = true;
        str++;
    } else if (*str == '+') {
        str++;
    }
    
    // Parse digits
    asthra_i128 result = asthra_i128_from_i64(0);
    asthra_i128 ten = asthra_i128_from_i64(10);
    
    while (*str >= '0' && *str <= '9') {
        asthra_i128 digit = asthra_i128_from_i64(*str - '0');
        result = asthra_i128_add(asthra_i128_mul(result, ten), digit);
        str++;
    }
    
    if (endptr) *endptr = (char *)str;
    
    return is_negative ? asthra_i128_neg(result) : result;
}

asthra_u128 asthra_u128_from_string(const char *str, char **endptr, int base) {
    // Simplified implementation - only handles base 10
    if (!str || base != 10) {
        if (endptr) *endptr = (char *)str;
        return asthra_u128_from_u64(0);
    }
    
    // Skip whitespace
    while (*str == ' ' || *str == '\t' || *str == '\n') {
        str++;
    }
    
    // Skip optional +
    if (*str == '+') {
        str++;
    }
    
    // Parse digits
    asthra_u128 result = asthra_u128_from_u64(0);
    asthra_u128 ten = asthra_u128_from_u64(10);
    
    while (*str >= '0' && *str <= '9') {
        asthra_u128 digit = asthra_u128_from_u64(*str - '0');
        result = asthra_u128_add(asthra_u128_mul(result, ten), digit);
        str++;
    }
    
    if (endptr) *endptr = (char *)str;
    
    return result;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

#if ASTHRA_HAS_NATIVE_INT128

bool asthra_i128_is_zero(asthra_i128 value) {
    return value == 0;
}

bool asthra_u128_is_zero(asthra_u128 value) {
    return value == 0;
}

int asthra_i128_clz(asthra_i128 value) {
    if (value == 0) return 128;
    
    // Use compiler builtin if available
    #ifdef __GNUC__
        unsigned long long high = (unsigned long long)(value >> 64);
        if (high != 0) {
            return __builtin_clzll(high);
        } else {
            return 64 + __builtin_clzll((unsigned long long)value);
        }
    #else
        // Fallback implementation
        int count = 0;
        asthra_i128 mask = (asthra_i128)1 << 127;
        while ((value & mask) == 0 && count < 128) {
            count++;
            mask >>= 1;
        }
        return count;
    #endif
}

int asthra_u128_clz(asthra_u128 value) {
    if (value == 0) return 128;
    
    // Use compiler builtin if available
    #ifdef __GNUC__
        unsigned long long high = (unsigned long long)(value >> 64);
        if (high != 0) {
            return __builtin_clzll(high);
        } else {
            return 64 + __builtin_clzll((unsigned long long)value);
        }
    #else
        // Fallback implementation
        int count = 0;
        asthra_u128 mask = (asthra_u128)1 << 127;
        while ((value & mask) == 0 && count < 128) {
            count++;
            mask >>= 1;
        }
        return count;
    #endif
}

int asthra_i128_ctz(asthra_i128 value) {
    if (value == 0) return 128;
    
    #ifdef __GNUC__
        unsigned long long low = (unsigned long long)value;
        if (low != 0) {
            return __builtin_ctzll(low);
        } else {
            return 64 + __builtin_ctzll((unsigned long long)(value >> 64));
        }
    #else
        int count = 0;
        while ((value & 1) == 0 && count < 128) {
            count++;
            value >>= 1;
        }
        return count;
    #endif
}

int asthra_u128_ctz(asthra_u128 value) {
    if (value == 0) return 128;
    
    #ifdef __GNUC__
        unsigned long long low = (unsigned long long)value;
        if (low != 0) {
            return __builtin_ctzll(low);
        } else {
            return 64 + __builtin_ctzll((unsigned long long)(value >> 64));
        }
    #else
        int count = 0;
        while ((value & 1) == 0 && count < 128) {
            count++;
            value >>= 1;
        }
        return count;
    #endif
}

int asthra_i128_popcount(asthra_i128 value) {
    #ifdef __GNUC__
        return __builtin_popcountll((unsigned long long)value) + 
               __builtin_popcountll((unsigned long long)(value >> 64));
    #else
        int count = 0;
        while (value) {
            count += value & 1;
            value >>= 1;
        }
        return count;
    #endif
}

int asthra_u128_popcount(asthra_u128 value) {
    #ifdef __GNUC__
        return __builtin_popcountll((unsigned long long)value) + 
               __builtin_popcountll((unsigned long long)(value >> 64));
    #else
        int count = 0;
        while (value) {
            count += value & 1;
            value >>= 1;
        }
        return count;
    #endif
}

#else

bool asthra_i128_is_zero(asthra_i128 value) {
    return value.low == 0 && value.high == 0;
}

bool asthra_u128_is_zero(asthra_u128 value) {
    return value.low == 0 && value.high == 0;
}

int asthra_i128_clz(asthra_i128 value) {
    if (asthra_i128_is_zero(value)) return 128;
    
    if (value.high != 0 && value.high != -1) {
        // Count leading zeros in high part
        int count = 0;
        uint64_t mask = 1ULL << 63;
        uint64_t uhigh = (uint64_t)value.high;
        while ((uhigh & mask) == 0 && count < 64) {
            count++;
            mask >>= 1;
        }
        return count;
    } else {
        // Count leading zeros in low part + 64
        int count = 64;
        uint64_t mask = 1ULL << 63;
        while ((value.low & mask) == 0 && count < 128) {
            count++;
            mask >>= 1;
        }
        return count;
    }
}

int asthra_u128_clz(asthra_u128 value) {
    if (asthra_u128_is_zero(value)) return 128;
    
    if (value.high != 0) {
        // Count leading zeros in high part
        int count = 0;
        uint64_t mask = 1ULL << 63;
        while ((value.high & mask) == 0 && count < 64) {
            count++;
            mask >>= 1;
        }
        return count;
    } else {
        // Count leading zeros in low part + 64
        int count = 64;
        uint64_t mask = 1ULL << 63;
        while ((value.low & mask) == 0 && count < 128) {
            count++;
            mask >>= 1;
        }
        return count;
    }
}

int asthra_i128_ctz(asthra_i128 value) {
    if (asthra_i128_is_zero(value)) return 128;
    
    if (value.low != 0) {
        // Count trailing zeros in low part
        int count = 0;
        while ((value.low & 1) == 0 && count < 64) {
            count++;
            value.low >>= 1;
        }
        return count;
    } else {
        // Count trailing zeros in high part + 64
        int count = 64;
        uint64_t uhigh = (uint64_t)value.high;
        while ((uhigh & 1) == 0 && count < 128) {
            count++;
            uhigh >>= 1;
        }
        return count;
    }
}

int asthra_u128_ctz(asthra_u128 value) {
    if (asthra_u128_is_zero(value)) return 128;
    
    if (value.low != 0) {
        // Count trailing zeros in low part
        int count = 0;
        while ((value.low & 1) == 0 && count < 64) {
            count++;
            value.low >>= 1;
        }
        return count;
    } else {
        // Count trailing zeros in high part + 64
        int count = 64;
        while ((value.high & 1) == 0 && count < 128) {
            count++;
            value.high >>= 1;
        }
        return count;
    }
}

int asthra_i128_popcount(asthra_i128 value) {
    int count = 0;
    
    // Count bits in low part
    uint64_t low = value.low;
    while (low) {
        count += low & 1;
        low >>= 1;
    }
    
    // Count bits in high part
    uint64_t high = (uint64_t)value.high;
    while (high) {
        count += high & 1;
        high >>= 1;
    }
    
    return count;
}

int asthra_u128_popcount(asthra_u128 value) {
    int count = 0;
    
    // Count bits in low part
    uint64_t low = value.low;
    while (low) {
        count += low & 1;
        low >>= 1;
    }
    
    // Count bits in high part
    uint64_t high = value.high;
    while (high) {
        count += high & 1;
        high >>= 1;
    }
    
    return count;
}

#endif // ASTHRA_HAS_NATIVE_INT128

// =============================================================================
// OVERFLOW CHECKING
// =============================================================================

bool asthra_i128_add_overflow(asthra_i128 a, asthra_i128 b, asthra_i128 *result) {
    if (!result) return false;
    
    *result = asthra_i128_add(a, b);
    
    // Check for overflow
    // Positive + Positive = Negative is overflow
    // Negative + Negative = Positive is overflow
    bool a_negative = asthra_i128_lt(a, asthra_i128_from_i64(0));
    bool b_negative = asthra_i128_lt(b, asthra_i128_from_i64(0));
    bool result_negative = asthra_i128_lt(*result, asthra_i128_from_i64(0));
    
    if (a_negative == b_negative && a_negative != result_negative) {
        return false; // Overflow occurred
    }
    
    return true;
}

bool asthra_i128_sub_overflow(asthra_i128 a, asthra_i128 b, asthra_i128 *result) {
    if (!result) return false;
    
    *result = asthra_i128_sub(a, b);
    
    // Check for overflow
    // Positive - Negative = Negative is overflow
    // Negative - Positive = Positive is overflow
    bool a_negative = asthra_i128_lt(a, asthra_i128_from_i64(0));
    bool b_negative = asthra_i128_lt(b, asthra_i128_from_i64(0));
    bool result_negative = asthra_i128_lt(*result, asthra_i128_from_i64(0));
    
    if (a_negative != b_negative && a_negative != result_negative) {
        return false; // Overflow occurred
    }
    
    return true;
}

bool asthra_i128_mul_overflow(asthra_i128 a, asthra_i128 b, asthra_i128 *result) {
    if (!result) return false;
    
    *result = asthra_i128_mul(a, b);
    
    // Simple overflow check: if a != 0 and result / a != b, then overflow
    if (!asthra_i128_is_zero(a)) {
        asthra_i128 check = asthra_i128_div(*result, a);
        if (!asthra_i128_eq(check, b)) {
            return false; // Overflow occurred
        }
    }
    
    return true;
}

bool asthra_u128_add_overflow(asthra_u128 a, asthra_u128 b, asthra_u128 *result) {
    if (!result) return false;
    
    *result = asthra_u128_add(a, b);
    
    // For unsigned, overflow if result < a or result < b
    if (asthra_u128_lt(*result, a) || asthra_u128_lt(*result, b)) {
        return false; // Overflow occurred
    }
    
    return true;
}

bool asthra_u128_sub_overflow(asthra_u128 a, asthra_u128 b, asthra_u128 *result) {
    if (!result) return false;
    
    *result = asthra_u128_sub(a, b);
    
    // For unsigned, overflow if a < b
    if (asthra_u128_lt(a, b)) {
        return false; // Underflow occurred
    }
    
    return true;
}

bool asthra_u128_mul_overflow(asthra_u128 a, asthra_u128 b, asthra_u128 *result) {
    if (!result) return false;
    
    *result = asthra_u128_mul(a, b);
    
    // Simple overflow check: if a != 0 and result / a != b, then overflow
    if (!asthra_u128_is_zero(a)) {
        asthra_u128 check = asthra_u128_div(*result, a);
        if (!asthra_u128_eq(check, b)) {
            return false; // Overflow occurred
        }
    }
    
    return true;
}