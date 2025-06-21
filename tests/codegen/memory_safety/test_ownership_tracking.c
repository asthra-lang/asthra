#include "test_memory_safety_common.h"

// Test metadata for ownership tracking
static AsthraTestMetadata ownership_metadata[] = {
    {"test_ownership_tracking", __FILE__, __LINE__, "Test ownership and borrowing tracking", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

AsthraTestResult test_ownership_tracking(AsthraTestContext* context) {
    MemorySafetyContext ctx;
    init_memory_safety_context(&ctx);
    
    // Test basic allocation and ownership
    void* ptr1 = safe_allocate(&ctx, 64, "test_ownership_tracking:ptr1");
    void* ptr2 = safe_allocate(&ctx, 128, "test_ownership_tracking:ptr2");
    
    if (!ptr1 || !ptr2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify ownership state
    MemoryBlock* block1 = find_memory_block(&ctx, ptr1);
    MemoryBlock* block2 = find_memory_block(&ctx, ptr2);
    
    if (!block1 || !block2) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (block1->state != MEMORY_STATE_OWNED || block1->ownership != OWNERSHIP_OWNED ||
        block2->state != MEMORY_STATE_OWNED || block2->ownership != OWNERSHIP_OWNED) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (block1->ref_count != 1 || block2->ref_count != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test ownership transfer
    if (!transfer_ownership(&ctx, ptr1, "test_ownership_tracking:transfer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (block1->state != MEMORY_STATE_MOVED) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test use after move detection
    if (transfer_ownership(&ctx, ptr1, "test_ownership_tracking:use_after_move")) {
        return ASTHRA_TEST_FAIL; // Should fail
    }
    
    if (ctx.violations_detected == 0) {
        return ASTHRA_TEST_FAIL; // Should have detected violation
    }
    
    // Test borrowing
    Lifetime* lifetime = create_lifetime(&ctx, "test_lifetime", 0);
    if (!lifetime) {
        return ASTHRA_TEST_FAIL;
    }
    
    BorrowReference* shared_borrow1 = create_borrow(&ctx, ptr2, OWNERSHIP_BORROWED_SHARED, lifetime->lifetime_id, "shared1");
    BorrowReference* shared_borrow2 = create_borrow(&ctx, ptr2, OWNERSHIP_BORROWED_SHARED, lifetime->lifetime_id, "shared2");
    
    if (!shared_borrow1 || !shared_borrow2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Multiple shared borrows should be allowed
    if (block2->ref_count != 3) { // Original + 2 borrows
        return ASTHRA_TEST_FAIL;
    }
    
    // Test mutable borrow conflict
    int initial_violations = ctx.violations_detected;
    BorrowReference* mutable_borrow = create_borrow(&ctx, ptr2, OWNERSHIP_BORROWED_MUTABLE, lifetime->lifetime_id, "mutable");
    
    if (mutable_borrow) {
        return ASTHRA_TEST_FAIL; // Should fail due to existing shared borrows
    }
    
    if (ctx.violations_detected <= initial_violations) {
        return ASTHRA_TEST_FAIL; // Should have detected violation
    }
    
    // End shared borrows
    end_borrow(&ctx, shared_borrow1);
    end_borrow(&ctx, shared_borrow2);
    
    if (block2->ref_count != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Now mutable borrow should work
    mutable_borrow = create_borrow(&ctx, ptr2, OWNERSHIP_BORROWED_MUTABLE, lifetime->lifetime_id, "mutable_after");
    
    if (!mutable_borrow) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (block2->ref_count != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Clean up
    end_borrow(&ctx, mutable_borrow);
    end_lifetime(&ctx, lifetime, 1);
    
    // Test deallocation
    if (!safe_deallocate(&ctx, ptr2, "test_ownership_tracking:free")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (block2->state != MEMORY_STATE_FREED) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test double free detection
    initial_violations = ctx.violations_detected;
    if (safe_deallocate(&ctx, ptr2, "test_ownership_tracking:double_free")) {
        return ASTHRA_TEST_FAIL; // Should fail
    }
    
    if (ctx.violations_detected <= initial_violations) {
        return ASTHRA_TEST_FAIL; // Should have detected violation
    }
    
    return ASTHRA_TEST_PASS;
} 
