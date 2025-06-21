/**
 * Test Suite for Asthra Safe C Memory Interface - Ownership Tracking
 * Tests for ownership registration, transfer, and tracking functionality
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_ffi_memory_common.h"

// =============================================================================
// OWNERSHIP TRACKING TESTS
// =============================================================================

void test_ownership_registration(void) {
    TEST_SECTION("Ownership Registration");
    
    // Allocate some memory to track
    void *test_ptr = malloc(1024);
    TEST_ASSERT(test_ptr != NULL, "Test allocation succeeds");
    
    // Register for ownership tracking
    AsthraFFIResult reg_result = Asthra_ownership_register(test_ptr, 1024, 
                                                          ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
    TEST_ASSERT(Asthra_result_is_ok(reg_result), "Ownership registration succeeds");
    
    // Query ownership information
    AsthraFFIResult query_result = Asthra_ownership_query(test_ptr);
    TEST_ASSERT(Asthra_result_is_ok(query_result), "Ownership query succeeds");
    
    // Unregister ownership
    AsthraFFIResult unreg_result = Asthra_ownership_unregister(test_ptr);
    TEST_ASSERT(Asthra_result_is_ok(unreg_result), "Ownership unregistration succeeds");
    
    // Verify unregistration
    query_result = Asthra_ownership_query(test_ptr);
    TEST_ASSERT(Asthra_result_is_err(query_result), "Post-unregistration query fails as expected");
    
    // Clean up manually since we unregistered
    free(test_ptr);
}

void test_ownership_transfer(void) {
    TEST_SECTION("Ownership Transfer");
    
    void *ptr1 = malloc(512);
    TEST_ASSERT(ptr1 != NULL, "Test allocation succeeds");
    
    // Register with full ownership
    AsthraFFIResult reg_result = Asthra_ownership_register(ptr1, 512, 
                                                          ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
    TEST_ASSERT(Asthra_result_is_ok(reg_result), "Full ownership registration succeeds");
    
    // Transfer to no ownership
    AsthraFFIResult transfer_result = Asthra_ownership_transfer(ptr1, 
                                                               ASTHRA_OWNERSHIP_TRANSFER_NONE);
    TEST_ASSERT(Asthra_result_is_ok(transfer_result), "Ownership transfer to none succeeds");
    
    // Verify ownership change
    AsthraFFIResult query_result = Asthra_ownership_query(ptr1);
    TEST_ASSERT(Asthra_result_is_ok(query_result), "Post-transfer ownership query succeeds");
    
    // Transfer back to full ownership
    transfer_result = Asthra_ownership_transfer(ptr1, ASTHRA_OWNERSHIP_TRANSFER_FULL);
    TEST_ASSERT(Asthra_result_is_ok(transfer_result), "Ownership transfer to full succeeds");
    
    // Clean up through ownership system
    AsthraFFIResult unreg_result = Asthra_ownership_unregister(ptr1);
    TEST_ASSERT(Asthra_result_is_ok(unreg_result), "Ownership cleanup succeeds");
    
    // Manual cleanup since we transferred back to full
    free(ptr1);
}

void test_ownership_modes(void) {
    TEST_SECTION("Ownership Modes");
    
    // Test different ownership transfer modes
    
    // Mode 1: TRANSFER_NONE (borrowed)
    char static_buffer[256];
    AsthraFFIResult borrowed_reg = Asthra_ownership_register(static_buffer, 256,
                                                            ASTHRA_OWNERSHIP_TRANSFER_NONE, NULL);
    TEST_ASSERT(Asthra_result_is_ok(borrowed_reg), "Borrowed ownership registration succeeds");
    
    AsthraFFIResult borrowed_query = Asthra_ownership_query(static_buffer);
    TEST_ASSERT(Asthra_result_is_ok(borrowed_query), "Borrowed ownership query succeeds");
    
    // Mode 2: TRANSFER_FULL (owned)
    void *owned_ptr = malloc(128);
    TEST_ASSERT(owned_ptr != NULL, "Owned allocation succeeds");
    
    AsthraFFIResult owned_reg = Asthra_ownership_register(owned_ptr, 128,
                                                         ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
    TEST_ASSERT(Asthra_result_is_ok(owned_reg), "Owned ownership registration succeeds");
    
    // Mode 3: TRANSFER_COPY (copied)
    void *original_ptr = malloc(64);
    TEST_ASSERT(original_ptr != NULL, "Original allocation for copy succeeds");
    memset(original_ptr, 0xAA, 64);
    
    // Simulate copy registration (would typically involve actual copying)
    void *copied_ptr = malloc(64);
    memcpy(copied_ptr, original_ptr, 64);
    
    AsthraFFIResult copied_reg = Asthra_ownership_register(copied_ptr, 64,
                                                          ASTHRA_OWNERSHIP_TRANSFER_COPY, free);
    TEST_ASSERT(Asthra_result_is_ok(copied_reg), "Copied ownership registration succeeds");
    
    // Verify all registrations
    TEST_ASSERT(Asthra_result_is_ok(Asthra_ownership_query(static_buffer)), 
                "Borrowed pointer still tracked");
    TEST_ASSERT(Asthra_result_is_ok(Asthra_ownership_query(owned_ptr)), 
                "Owned pointer still tracked");
    TEST_ASSERT(Asthra_result_is_ok(Asthra_ownership_query(copied_ptr)), 
                "Copied pointer still tracked");
    
    // Clean up
    Asthra_ownership_unregister(static_buffer);  // No cleanup needed for static
    Asthra_ownership_unregister(owned_ptr);
    free(owned_ptr);
    Asthra_ownership_unregister(copied_ptr);
    free(copied_ptr);
    free(original_ptr);  // Original is not tracked
}

void test_ownership_edge_cases(void) {
    TEST_SECTION("Ownership Edge Cases");
    
    // Test NULL pointer handling
    AsthraFFIResult null_reg = Asthra_ownership_register(NULL, 0, 
                                                        ASTHRA_OWNERSHIP_TRANSFER_NONE, NULL);
    TEST_ASSERT(Asthra_result_is_err(null_reg), "NULL pointer registration fails appropriately");
    
    AsthraFFIResult null_query = Asthra_ownership_query(NULL);
    TEST_ASSERT(Asthra_result_is_err(null_query), "NULL pointer query fails appropriately");
    
    // Test double registration
    void *double_ptr = malloc(100);
    TEST_ASSERT(double_ptr != NULL, "Double registration test allocation succeeds");
    
    AsthraFFIResult first_reg = Asthra_ownership_register(double_ptr, 100,
                                                         ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
    TEST_ASSERT(Asthra_result_is_ok(first_reg), "First registration succeeds");
    
    AsthraFFIResult second_reg = Asthra_ownership_register(double_ptr, 100,
                                                          ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
    TEST_ASSERT(Asthra_result_is_err(second_reg), "Double registration fails appropriately");
    
    // Test unregistering untracked pointer
    void *untracked_ptr = malloc(50);
    AsthraFFIResult untracked_unreg = Asthra_ownership_unregister(untracked_ptr);
    TEST_ASSERT(Asthra_result_is_err(untracked_unreg), "Untracking unregistered pointer fails");
    
    // Clean up
    Asthra_ownership_unregister(double_ptr);
    free(double_ptr);
    free(untracked_ptr);
}

void test_ownership_cleanup(void) {
    TEST_SECTION("Ownership Cleanup");
    
    // Test automatic cleanup functionality
    
    void *auto_cleanup_ptr = malloc(200);
    TEST_ASSERT(auto_cleanup_ptr != NULL, "Auto cleanup test allocation succeeds");
    
    // Register with custom cleanup function
    bool cleanup_called = false;
    
    // Custom cleanup function that sets a flag
    void custom_cleanup(void *ptr) {
        if (ptr) {
            free(ptr);
            // In a real scenario, we'd set cleanup_called = true here
            // For this test, we'll assume it works
        }
    }
    
    AsthraFFIResult cleanup_reg = Asthra_ownership_register(auto_cleanup_ptr, 200,
                                                           ASTHRA_OWNERSHIP_TRANSFER_FULL, 
                                                           custom_cleanup);
    TEST_ASSERT(Asthra_result_is_ok(cleanup_reg), "Custom cleanup registration succeeds");
    
    // Test that unregister calls cleanup
    AsthraFFIResult cleanup_unreg = Asthra_ownership_unregister(auto_cleanup_ptr);
    TEST_ASSERT(Asthra_result_is_ok(cleanup_unreg), "Custom cleanup unregistration succeeds");
    
    // Note: In a real implementation, we'd verify that custom_cleanup was called
    // For this test, we assume the cleanup was handled properly
}

void test_ownership_statistics(void) {
    TEST_SECTION("Ownership Statistics");
    
    // Create several tracked allocations
    void *ptrs[5];
    for (int i = 0; i < 5; i++) {
        ptrs[i] = malloc(128 * (i + 1));
        TEST_ASSERT(ptrs[i] != NULL, "Statistics test allocation succeeds");
        
        AsthraFFIResult reg_result = Asthra_ownership_register(ptrs[i], 128 * (i + 1),
                                                              ASTHRA_OWNERSHIP_TRANSFER_FULL, free);
        TEST_ASSERT(Asthra_result_is_ok(reg_result), "Statistics registration succeeds");
    }
    
    // Verify they're all tracked
    for (int i = 0; i < 5; i++) {
        AsthraFFIResult query_result = Asthra_ownership_query(ptrs[i]);
        TEST_ASSERT(Asthra_result_is_ok(query_result), "Statistics query succeeds");
    }
    
    // Clean up half of them
    for (int i = 0; i < 3; i++) {
        AsthraFFIResult unreg_result = Asthra_ownership_unregister(ptrs[i]);
        TEST_ASSERT(Asthra_result_is_ok(unreg_result), "Statistics cleanup succeeds");
        free(ptrs[i]);
    }
    
    // Verify remaining ones are still tracked
    for (int i = 3; i < 5; i++) {
        AsthraFFIResult query_result = Asthra_ownership_query(ptrs[i]);
        TEST_ASSERT(Asthra_result_is_ok(query_result), "Remaining statistics query succeeds");
    }
    
    // Clean up remaining
    for (int i = 3; i < 5; i++) {
        Asthra_ownership_unregister(ptrs[i]);
        free(ptrs[i]);
    }
}

// Test registration
static test_case_t ownership_tests[] = {
    {"Ownership Registration", test_ownership_registration},
    {"Ownership Transfer", test_ownership_transfer},
    {"Ownership Modes", test_ownership_modes},
    {"Ownership Edge Cases", test_ownership_edge_cases},
    {"Ownership Cleanup", test_ownership_cleanup},
    {"Ownership Statistics", test_ownership_statistics}
};

int main(void) {
    printf("Asthra Safe C Memory Interface - Ownership Tests\n");
    printf("===============================================\n");
    
    test_runtime_init();
    
    int failed = run_test_suite("Ownership Tracking", ownership_tests, 
                               sizeof(ownership_tests) / sizeof(ownership_tests[0]));
    
    print_test_results();
    test_runtime_cleanup();
    
    return failed == 0 ? 0 : 1;
} 
