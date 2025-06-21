/**
 * Asthra Programming Language v1.2 Comprehensive Test Suite - Security Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Security validation function declarations.
 */

#ifndef ASTHRA_TEST_COMPREHENSIVE_SECURITY_H
#define ASTHRA_TEST_COMPREHENSIVE_SECURITY_H

#include "test_comprehensive_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SECURITY VALIDATION FUNCTION DECLARATIONS
// =============================================================================

// Constant-time verification
bool asthra_v12_verify_constant_time(AsthraV12TestContext *ctx,
                                    void (*operation)(void*),
                                    void *data,
                                    size_t iterations);

// Memory security
bool asthra_v12_verify_secure_memory_zeroing(void *ptr, size_t size);
void asthra_v12_secure_memory_zero(void *ptr, size_t size);

// Entropy and randomness testing
bool asthra_v12_verify_entropy_quality(const uint8_t *data, size_t size);
bool asthra_v12_test_csprng_quality(AsthraV12TestContext *ctx, 
                                   void (*rng_function)(uint8_t*, size_t),
                                   size_t test_size);

// Side-channel resistance
bool asthra_v12_verify_side_channel_resistance(AsthraV12TestContext *ctx,
                                              void (*operation)(void*),
                                              void *data1,
                                              void *data2,
                                              size_t iterations);

// Results reporting
void asthra_v12_security_print_results(const AsthraV12TestContext *ctx);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_COMPREHENSIVE_SECURITY_H 
