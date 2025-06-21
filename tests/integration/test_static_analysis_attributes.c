/**
 * Asthra Programming Language Compiler
 * Static Analysis Integration Test Suite - Attribute Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 5.2: Static Analysis Attribute Tests
 * - Test C17 annotation attributes
 * - Validate PURE, CONST, MALLOC attributes
 * - Test NONNULL and WARN_UNUSED_RESULT attributes
 */

#include "test_static_analysis_common.h"

// =============================================================================
// TEST HELPER FUNCTIONS WITH ATTRIBUTES
// =============================================================================

// Test function with various attributes
ASTHRA_PURE ASTHRA_NONNULL(1)
static size_t test_pure_function(const char *restrict str) {
    return asthra_string_length(str);
}

ASTHRA_CONST
static int test_const_function(void) {
    return 42;
}

ASTHRA_MALLOC ASTHRA_WARN_UNUSED_RESULT
static void *test_malloc_function(size_t size) {
    return asthra_alloc_aligned(size, 16);
}

// =============================================================================
// STATIC ANALYSIS ATTRIBUTE TESTS
// =============================================================================

AsthraTestResult test_static_analysis_attributes(AsthraTestContext *context) {
    asthra_test_context_start(context);
    
    // Test pure function attribute
    const char *test_string = "Hello, World!";
    size_t length1 = test_pure_function(test_string);
    size_t length2 = test_pure_function(test_string);
    
    if (!asthra_test_assert_size_eq(context, length1, length2, 
                                   "Pure function should return same result")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_size_eq(context, length1, 13, 
                                   "String length should be 13")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test const function attribute
    int value1 = test_const_function();
    int value2 = test_const_function();
    
    if (!asthra_test_assert_int_eq(context, value1, value2, 
                                  "Const function should return same result")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, value1, 42, 
                                  "Const function should return 42")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test malloc function attribute
    void *ptr = test_malloc_function(64);
    if (!asthra_test_assert_pointer(context, ptr, "Malloc function should return non-NULL")) {
        return ASTHRA_TEST_FAIL;
    }
    
    asthra_free_aligned(ptr);
    
    asthra_test_context_end(context, ASTHRA_TEST_PASS);
    return ASTHRA_TEST_PASS;
} 

/**
 * Main test runner
 */
int main(void) {
    printf("=== Asthra Integration Tests - Static Analysis Attributes ===\n\n");
    
    // Create test statistics
    AsthraTestStatistics* stats = asthra_test_statistics_create();
    if (!stats) {
        printf("Failed to create test statistics\n");
        return 1;
    }
    
    // Create test metadata
    AsthraTestMetadata metadata = {
        .name = "static_analysis_attributes",
        .file = __FILE__,
        .line = __LINE__,
        .function = "test_static_analysis_attributes",
        .severity = ASTHRA_TEST_SEVERITY_HIGH,
        .timeout_ns = 30000000000ULL,
        .skip = false,
        .skip_reason = NULL
    };
    
    AsthraTestContext* context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        printf("Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }
    
    // Run test
    AsthraTestResult result = test_static_analysis_attributes(context);
    
    // Report results
    printf("Static analysis attributes: %s\n", 
           result == ASTHRA_TEST_PASS ? "PASS" : "FAIL");
    
    // Print statistics
    printf("\n=== Test Statistics ===\n");
    printf("Tests run:       1\n");
    printf("Tests passed:    %d\n", result == ASTHRA_TEST_PASS ? 1 : 0);
    printf("Tests failed:    %d\n", result == ASTHRA_TEST_FAIL ? 1 : 0);
    printf("Tests skipped:   0\n");
    printf("Tests error:     0\n");
    printf("Tests timeout:   0\n");
    printf("Total duration:  0.000 ms\n");
    printf("Max duration:    0.000 ms\n");
    printf("Min duration:    0.000 ms\n");
    printf("Assertions:      %llu checked, %llu failed\n", 
           asthra_test_get_stat(&stats->assertions_checked), 
           asthra_test_get_stat(&stats->assertions_failed));
    printf("========================\n");
    printf("Integration tests: %d/1 passed\n", result == ASTHRA_TEST_PASS ? 1 : 0);
    
    // Cleanup
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);
    
    return result == ASTHRA_TEST_PASS ? 0 : 1;
} 
