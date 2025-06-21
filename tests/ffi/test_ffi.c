/**
 * Asthra Programming Language v1.2 Enhanced FFI Tests
 * Main FFI Test Coordinator
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main coordinator for enhanced FFI capabilities tests. This file includes
 * all split FFI test modules and provides a unified test runner.
 */

#include "../core/test_comprehensive.h"
#include "test_ffi_enhanced.h"
#include <stdio.h>

// =============================================================================
// FFI TEST SUITE RUNNER
// =============================================================================

/**
 * Run all enhanced FFI tests in organized groups
 */
AsthraTestResult run_ffi_enhanced_test_suite(AsthraV12TestContext *ctx) {
    printf("Running Enhanced FFI Test Suite...\n");
    
    // Enhanced FFI Capabilities Tests
    printf("\n=== Enhanced FFI Capabilities Tests ===\n");
    
    if (test_ffi_enhanced_function_calls(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Enhanced function calls test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Enhanced function calls test\n");
    
    if (test_ffi_bidirectional_calls(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Bidirectional calls test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Bidirectional calls test\n");
    
    if (test_ffi_complex_data_structures(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Complex data structures test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Complex data structures test\n");
    
    // Ownership Transfer Tests
    printf("\n=== Ownership Transfer Tests ===\n");
    
    if (test_ffi_ownership_borrow(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Ownership borrow test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Ownership borrow test\n");
    
    if (test_ffi_ownership_move(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Ownership move test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Ownership move test\n");
    
    if (test_ffi_ownership_copy(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Ownership copy test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Ownership copy test\n");
    
    // Memory Safety Tests
    printf("\n=== Memory Safety Tests ===\n");
    
    if (test_ffi_memory_safety_bounds_checking(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Memory safety bounds checking test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Memory safety bounds checking test\n");
    
    if (test_ffi_memory_safety_leak_prevention(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Memory safety leak prevention test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Memory safety leak prevention test\n");
    
    if (test_ffi_memory_safety_double_free_prevention(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: Memory safety double-free prevention test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: Memory safety double-free prevention test\n");
    
    // C17 Compliance Tests
    printf("\n=== C17 Compliance Tests ===\n");
    
    if (test_ffi_c17_compliance_features(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: C17 compliance features test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: C17 compliance features test\n");
    
    if (test_ffi_c17_compliance_types(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: C17 compliance types test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: C17 compliance types test\n");
    
    if (test_ffi_c17_compliance_functions(ctx) != ASTHRA_TEST_PASS) {
        printf("FAILED: C17 compliance functions test\n");
        return ASTHRA_TEST_FAIL;
    }
    printf("PASSED: C17 compliance functions test\n");
    
    printf("\n=== All Enhanced FFI Tests Passed! ===\n");
    return ASTHRA_TEST_PASS;
}

/**
 * Main entry point for FFI tests (for backward compatibility)
 */
AsthraTestResult test_ffi_enhanced_capabilities(AsthraV12TestContext *ctx) {
    return run_ffi_enhanced_test_suite(ctx);
} 

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(void) {
    printf("Enhanced FFI Test Suite\n");
    printf("========================\n\n");
    
    // Create test context
    AsthraV12TestContext ctx = {0};
    
    // Run the test suite
    AsthraTestResult result = run_ffi_enhanced_test_suite(&ctx);
    
    if (result == ASTHRA_TEST_PASS) {
        printf("\n=== All FFI Tests Passed! ===\n");
        return 0;
    } else {
        printf("\n=== Some FFI Tests Failed ===\n");
        return 1;
    }
} 
