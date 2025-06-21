#include "test_memory_safety_common.h"

// Test metadata for memory leak detection
static AsthraTestMetadata leak_detection_metadata[] = {
    {"test_memory_leak_detection", __FILE__, __LINE__, "Test memory leak detection and prevention", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

AsthraTestResult test_memory_leak_detection(AsthraTestContext* context) {
    MemorySafetyContext ctx;
    init_memory_safety_context(&ctx);
    
    // Allocate some memory
    void* ptr1 = safe_allocate(&ctx, 64, "leak_test:ptr1");
    void* ptr2 = safe_allocate(&ctx, 128, "leak_test:ptr2");
    void* ptr3 = safe_allocate(&ctx, 256, "leak_test:ptr3");
    
    if (!ptr1 || !ptr2 || !ptr3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Initially should have 3 potential leaks
    int initial_leaks = count_memory_leaks(&ctx);
    if (initial_leaks != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Free one pointer
    if (!safe_deallocate(&ctx, ptr2, "leak_test:free_ptr2")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should now have 2 potential leaks
    int after_free = count_memory_leaks(&ctx);
    if (after_free != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Transfer ownership of another (simulating move)
    if (!transfer_ownership(&ctx, ptr3, "leak_test:move_ptr3")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should still count as potential leak (moved but not freed)
    int after_move = count_memory_leaks(&ctx);
    if (after_move != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create some borrows that don't affect leak counting
    Lifetime* lifetime = create_lifetime(&ctx, "leak_test", 0);
    BorrowReference* borrow = create_borrow(&ctx, ptr1, OWNERSHIP_BORROWED_SHARED, lifetime->lifetime_id, "leak_borrow");
    
    if (!lifetime || !borrow) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should still show same leak count
    int with_borrows = count_memory_leaks(&ctx);
    if (with_borrows != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // End borrows and free remaining memory
    end_borrow(&ctx, borrow);
    end_lifetime(&ctx, lifetime, 1);
    
    if (!safe_deallocate(&ctx, ptr1, "leak_test:free_ptr1")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should now have 1 leak (ptr3 was moved but never freed)
    int final_leaks = count_memory_leaks(&ctx);
    if (final_leaks != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify allocation/deallocation statistics
    if (ctx.allocations_made != 3 || ctx.deallocations_made != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (ctx.ownership_transfers != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
} 
