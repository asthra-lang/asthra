#include "test_memory_safety_common.h"

// Test metadata for lifetime management
static AsthraTestMetadata lifetime_metadata[] = {
    {"test_lifetime_management", __FILE__, __LINE__, "Test lifetime analysis and validation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

AsthraTestResult test_lifetime_management(AsthraTestContext *context) {
    MemorySafetyContext ctx;
    init_memory_safety_context(&ctx);

    // Create nested lifetimes
    Lifetime *outer_lifetime = create_lifetime(&ctx, "outer", 0);
    Lifetime *inner_lifetime = create_lifetime(&ctx, "inner", 1);

    if (!outer_lifetime || !inner_lifetime) {
        return ASTHRA_TEST_FAIL;
    }

    // Allocate memory in outer scope
    void *outer_ptr = safe_allocate(&ctx, 64, "test_lifetime_management:outer");
    if (!outer_ptr) {
        return ASTHRA_TEST_FAIL;
    }

    // Create borrows in different scopes
    BorrowReference *outer_borrow = create_borrow(&ctx, outer_ptr, OWNERSHIP_BORROWED_SHARED,
                                                  outer_lifetime->lifetime_id, "outer_borrow");
    BorrowReference *inner_borrow = create_borrow(&ctx, outer_ptr, OWNERSHIP_BORROWED_SHARED,
                                                  inner_lifetime->lifetime_id, "inner_borrow");

    if (!outer_borrow || !inner_borrow) {
        return ASTHRA_TEST_FAIL;
    }

    MemoryBlock *block = find_memory_block(&ctx, outer_ptr);
    if (!block || block->ref_count != 3) { // Original + 2 borrows
        return ASTHRA_TEST_FAIL;
    }

    // End inner lifetime first (inner scope ends)
    end_lifetime(&ctx, inner_lifetime, 2);

    if (inner_lifetime->is_active) {
        return ASTHRA_TEST_FAIL;
    }

    if (!inner_borrow->is_active) {
        // Inner borrow should be ended
        if (block->ref_count != 2) { // Original + outer borrow
            return ASTHRA_TEST_FAIL;
        }
    }

    // Outer borrow should still be active
    if (!outer_borrow->is_active) {
        return ASTHRA_TEST_FAIL;
    }

    // End outer lifetime
    end_lifetime(&ctx, outer_lifetime, 3);

    if (!outer_borrow->is_active) {
        if (block->ref_count != 1) { // Just original
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test lifetime validation
    void *inner_ptr = safe_allocate(&ctx, 32, "test_lifetime_management:inner");
    Lifetime *new_inner = create_lifetime(&ctx, "new_inner", 4);

    if (!inner_ptr || !new_inner) {
        return ASTHRA_TEST_FAIL;
    }

    BorrowReference *short_lived =
        create_borrow(&ctx, inner_ptr, OWNERSHIP_BORROWED_MUTABLE, new_inner->lifetime_id, "short");

    if (!short_lived) {
        return ASTHRA_TEST_FAIL;
    }

    // End the short lifetime
    end_lifetime(&ctx, new_inner, 5);

    // The borrow should be automatically ended
    if (short_lived->is_active) {
        return ASTHRA_TEST_FAIL;
    }

    MemoryBlock *inner_block = find_memory_block(&ctx, inner_ptr);
    if (!inner_block || inner_block->ref_count != 1) {
        return ASTHRA_TEST_FAIL;
    }

    // Clean up
    safe_deallocate(&ctx, outer_ptr, "cleanup_outer");
    safe_deallocate(&ctx, inner_ptr, "cleanup_inner");

    return ASTHRA_TEST_PASS;
}
