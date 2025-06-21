/**
 * Asthra Programming Language Runtime v1.2
 * Cryptographic Primitives Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// CRYPTOGRAPHIC PRIMITIVES TESTS
// =============================================================================

int test_cryptographic_primitives(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");
    
    // Test CSPRNG
    uint8_t random_bytes[32];
    int rand_result = asthra_random_bytes(random_bytes, sizeof(random_bytes));
    TEST_ASSERT(rand_result == 0, "Random byte generation failed");
    
    // Check that bytes are not all zero (very unlikely with proper CSPRNG)
    bool all_zero = true;
    for (int i = 0; i < 32; i++) {
        if (random_bytes[i] != 0) {
            all_zero = false;
            break;
        }
    }
    TEST_ASSERT(!all_zero, "Random bytes should not all be zero");
    
    // Test random uint64
    uint64_t rand_uint64 = asthra_random_uint64();
    printf("  Random uint64: %" PRIu64 "\n", rand_uint64);
    
    // Test secure memory operations
    char sensitive_data[] = "secret password";
    asthra_secure_zero(sensitive_data, strlen(sensitive_data));
    
    // Verify data is zeroed
    for (size_t i = 0; i < strlen("secret password"); i++) {
        TEST_ASSERT(sensitive_data[i] == 0, "Secure zero failed");
    }
    
    // Test constant-time memory comparison
    uint8_t data1[] = {1, 2, 3, 4, 5};
    uint8_t data2[] = {1, 2, 3, 4, 5};
    uint8_t data3[] = {1, 2, 3, 4, 6};
    
    TEST_ASSERT(asthra_constant_time_memcmp(data1, data2, 5) == 0, "Constant-time memcmp should return 0 for equal data");
    TEST_ASSERT(asthra_constant_time_memcmp(data1, data3, 5) != 0, "Constant-time memcmp should return non-zero for different data");
    
    asthra_runtime_cleanup();
    TEST_PASS("Cryptographic primitives");
}

// Test function declarations for external use
int test_cryptographic_primitives(void); 
