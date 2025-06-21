/**
 * Asthra Programming Language Compiler
 * Bit Vector Operations for Data Flow Analysis
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Bit vector data structure and operations for efficient set manipulation
 * in data flow analysis algorithms.
 */

#ifndef ASTHRA_OPTIMIZER_BITVECTOR_H
#define ASTHRA_OPTIMIZER_BITVECTOR_H

#include "optimizer_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BIT VECTOR STRUCTURE
// =============================================================================

// Bit vector for data flow analysis
struct BitVector {
    uint64_t *bits;
    size_t size;  // Number of bits
    size_t word_count;  // Number of 64-bit words
};

// =============================================================================
// BIT VECTOR OPERATIONS
// =============================================================================

BitVector *bitvector_create(size_t size);
void bitvector_destroy(BitVector *vec);
BitVector *bitvector_clone(const BitVector *vec);

// Bit operations
bool bitvector_set(BitVector *vec, size_t bit);
bool bitvector_clear(BitVector *vec, size_t bit);
bool bitvector_test(const BitVector *vec, size_t bit);
void bitvector_clear_all(BitVector *vec);
void bitvector_set_all(BitVector *vec);

// Set operations
bool bitvector_union(BitVector *dest, const BitVector *src);
bool bitvector_intersection(BitVector *dest, const BitVector *src);
bool bitvector_difference(BitVector *dest, const BitVector *src);
bool bitvector_equals(const BitVector *vec1, const BitVector *vec2);

// Type-safe bit vector operations
#define bitvector_operation(vec1, vec2, op) _Generic((op), \
    int: bitvector_union, \
    default: bitvector_union \
)(vec1, vec2)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_OPTIMIZER_BITVECTOR_H 
