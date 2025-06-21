/**
 * Asthra Programming Language v1.2 Security Test Helpers
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared definitions and helper functions for security tests.
 */

#ifndef ASTHRA_TEST_SECURITY_HELPERS_H
#define ASTHRA_TEST_SECURITY_HELPERS_H

#include "../core/test_comprehensive.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SECURITY TEST HELPERS AND MOCK IMPLEMENTATIONS
// =============================================================================

// Mock constant-time comparison function
int mock_constant_time_compare(const uint8_t *a, const uint8_t *b, size_t len);

// Mock variable-time comparison (for testing contrast)
int mock_variable_time_compare(const uint8_t *a, const uint8_t *b, size_t len);

// Mock secure memory zeroing
void mock_secure_zero(volatile void *ptr, size_t size);

// Mock CSPRNG implementation
void mock_csprng_fill(uint8_t *buffer, size_t size);

// RDTSC timing function (architecture-specific)
uint64_t rdtsc(void);

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================
// Note: These functions are declared in test_comprehensive.h with AsthraV12TestContext
// They are implemented in the respective test files

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_SECURITY_HELPERS_H 
