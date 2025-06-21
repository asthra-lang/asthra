/**
 * Comprehensive Security Tests - Common Header
 * 
 * Shared definitions and function declarations for the modular
 * comprehensive security test suite.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef COMPREHENSIVE_SECURITY_COMMON_H
#define COMPREHENSIVE_SECURITY_COMMON_H

#include "test_comprehensive_security.h"
#include "test_comprehensive.h"
#include "../../runtime/crypto/asthra_runtime_crypto.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Timing analysis functions
bool asthra_v12_verify_constant_time(AsthraV12TestContext *ctx,
                                    void (*operation)(void*),
                                    void *data,
                                    size_t iterations);

bool asthra_v12_verify_side_channel_resistance(AsthraV12TestContext *ctx,
                                              void (*operation)(void*),
                                              void *data1,
                                              void *data2,
                                              size_t iterations);

// Memory security functions
bool asthra_v12_verify_secure_memory_zeroing(void *ptr, size_t size);
void asthra_v12_secure_memory_zero(void *ptr, size_t size);

// Entropy testing functions
bool asthra_v12_verify_entropy_quality(const uint8_t *data, size_t size);
bool asthra_v12_test_csprng_quality(AsthraV12TestContext *ctx, 
                                   void (*rng_function)(uint8_t*, size_t),
                                   size_t test_size);

// Test implementation functions
AsthraTestResult test_security_constant_time_verification(AsthraV12TestContext *ctx);
AsthraTestResult test_security_memory_zeroing(AsthraV12TestContext *ctx);
AsthraTestResult test_security_csprng_functionality(AsthraV12TestContext *ctx);
AsthraTestResult test_security_side_channel_resistance(AsthraV12TestContext *ctx);
AsthraTestResult test_security_entropy_testing(AsthraV12TestContext *ctx);

// Utility functions
void asthra_v12_security_print_results(const AsthraV12TestContext *ctx);
void asthra_v12_security_print_separator(void);

// Wrapper functions
static void constant_time_memcmp_wrapper(void *data);

#endif // COMPREHENSIVE_SECURITY_COMMON_H