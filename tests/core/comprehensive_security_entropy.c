/**
 * Comprehensive Security Tests - Entropy Testing
 * 
 * CSPRNG quality testing and entropy validation functions.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "comprehensive_security_common.h"

// =============================================================================
// ENTROPY TESTING FUNCTIONS
// =============================================================================

bool asthra_v12_verify_entropy_quality(const uint8_t *data, size_t size) {
    if (!data || size == 0) return false;

    // Simple entropy test - check for patterns
    uint32_t byte_counts[256] = {0};

    for (size_t i = 0; i < size; i++) {
        byte_counts[data[i]]++;
    }

    // Check for runs (consecutive identical bytes)
    int longest_run = 1;
    int current_run = 1;

    for (size_t i = 1; i < size; i++) {
        if (data[i] == data[i-1]) {
            current_run++;
        } else {
            if (current_run > longest_run) {
                longest_run = current_run;
            }
            current_run = 1;
        }
    }

    // Longest run should not be too long
    double sqrt_size = sqrt((double)size);
    if (longest_run > (int)sqrt_size) {
        return false;
    }

    return true;
}

bool asthra_v12_test_csprng_quality(AsthraV12TestContext *ctx, 
                                   void (*rng_function)(uint8_t*, size_t),
                                   size_t test_size) {
    if (!ctx || !rng_function || test_size == 0) return false;

    uint8_t *random_data = malloc(test_size);
    if (!random_data) return false;

    // Generate random data
    rng_function(random_data, test_size);

    // Test entropy quality
    bool entropy_ok = asthra_v12_verify_entropy_quality(random_data, test_size);
    ctx->security.entropy_sufficient = entropy_ok;

    // Additional tests could include:
    // - Frequency test
    // - Runs test
    // - Serial test
    // - Poker test

    free(random_data);
    return entropy_ok;
}