/**
 * Asthra Programming Language Compiler
 * Optimizer Bit Vector Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Bit vector operations for data flow analysis.
 */

#include "optimizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// C17 static assertion for bit vector operations
_Static_assert(CHAR_BIT == 8, "Bit vector operations assume 8-bit bytes");

// =============================================================================
// BIT VECTOR OPERATIONS
// =============================================================================

BitVector *bitvector_create(size_t size) {
    BitVector *vec = malloc(sizeof(BitVector));
    if (!vec) return NULL;
    
    size_t word_count = (size + 63) / 64;  // Round up to nearest 64-bit word
    vec->bits = calloc(word_count, sizeof(uint64_t));
    if (!vec->bits) {
        free(vec);
        return NULL;
    }
    
    vec->size = size;
    vec->word_count = word_count;
    return vec;
}

void bitvector_destroy(BitVector *vec) {
    if (!vec) return;
    free(vec->bits);
    free(vec);
}

BitVector *bitvector_clone(const BitVector *vec) {
    if (!vec) return NULL;
    
    BitVector *clone = bitvector_create(vec->size);
    if (!clone) return NULL;
    
    memcpy(clone->bits, vec->bits, vec->word_count * sizeof(uint64_t));
    return clone;
}

bool bitvector_set(BitVector *vec, size_t bit) {
    if (!vec || bit >= vec->size) return false;
    
    size_t word_index = bit / 64;
    size_t bit_index = bit % 64;
    vec->bits[word_index] |= (1ULL << bit_index);
    return true;
}

bool bitvector_clear(BitVector *vec, size_t bit) {
    if (!vec || bit >= vec->size) return false;
    
    size_t word_index = bit / 64;
    size_t bit_index = bit % 64;
    vec->bits[word_index] &= ~(1ULL << bit_index);
    return true;
}

bool bitvector_test(const BitVector *vec, size_t bit) {
    if (!vec || bit >= vec->size) return false;
    
    size_t word_index = bit / 64;
    size_t bit_index = bit % 64;
    return (vec->bits[word_index] & (1ULL << bit_index)) != 0;
}

void bitvector_clear_all(BitVector *vec) {
    if (!vec) return;
    
    memset(vec->bits, 0, vec->word_count * sizeof(uint64_t));
}

void bitvector_set_all(BitVector *vec) {
    if (!vec) return;
    
    // Set all words to all 1s
    for (size_t i = 0; i < vec->word_count; i++) {
        vec->bits[i] = UINT64_MAX;
    }
    
    // Clear any extra bits in the last word
    size_t extra_bits = vec->size % 64;
    if (extra_bits > 0) {
        uint64_t mask = (1ULL << extra_bits) - 1;
        vec->bits[vec->word_count - 1] &= mask;
    }
}

bool bitvector_union(BitVector *dest, const BitVector *src) {
    if (!dest || !src || dest->size != src->size) return false;
    
    for (size_t i = 0; i < dest->word_count; i++) {
        dest->bits[i] |= src->bits[i];
    }
    return true;
}

bool bitvector_intersection(BitVector *dest, const BitVector *src) {
    if (!dest || !src || dest->size != src->size) return false;
    
    for (size_t i = 0; i < dest->word_count; i++) {
        dest->bits[i] &= src->bits[i];
    }
    return true;
}

bool bitvector_difference(BitVector *dest, const BitVector *src) {
    if (!dest || !src || dest->size != src->size) return false;
    
    for (size_t i = 0; i < dest->word_count; i++) {
        dest->bits[i] &= ~src->bits[i];
    }
    return true;
}

bool bitvector_equals(const BitVector *vec1, const BitVector *vec2) {
    if (!vec1 || !vec2 || vec1->size != vec2->size) return false;
    
    return memcmp(vec1->bits, vec2->bits, vec1->word_count * sizeof(uint64_t)) == 0;
} 
