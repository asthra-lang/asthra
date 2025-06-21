/**
 * Test file for future management
 * Tests future creation and state management
 */

#include "async_await_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_future_management", __FILE__, __LINE__, "Test future creation and state management", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions
static AsthraTestResult test_future_management(AsthraTestContext* context) {
    AsyncSystemContext ctx;
    init_async_system_context(&ctx);
    
    // Create futures
    AsyncFuture* future1 = create_future(&ctx);
    AsyncFuture* future2 = create_future(&ctx);
    AsyncFuture* future3 = create_future(&ctx);
    
    if (!future1 || !future2 || !future3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify initial state
    if (future1->state != ASYNC_STATE_PENDING || 
        future2->state != ASYNC_STATE_PENDING ||
        future3->state != ASYNC_STATE_PENDING) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify unique IDs
    if (future1->future_id == future2->future_id || 
        future2->future_id == future3->future_id || 
        future1->future_id == future3->future_id) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify timestamps
    if (future1->created_timestamp == 0 || 
        future2->created_timestamp <= future1->created_timestamp ||
        future3->created_timestamp <= future2->created_timestamp) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test future completion
    int result_value = 42;
    complete_future(&ctx, future1, &result_value);
    
    if (future1->state != ASYNC_STATE_COMPLETED ||
        future1->result_value != &result_value ||
        future1->completed_timestamp == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test future cancellation
    cancel_future(&ctx, future2, "Operation cancelled by user");
    
    if (future2->state != ASYNC_STATE_CANCELLED ||
        strcmp(future2->error_message, "Operation cancelled by user") != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test future error
    error_future(&ctx, future3, "Network timeout");
    
    if (future3->state != ASYNC_STATE_ERROR ||
        strcmp(future3->error_message, "Network timeout") != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify statistics
    if (ctx.completed_futures != 1 || 
        ctx.cancelled_futures != 1 || 
        ctx.error_futures != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Future Management Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_future_management(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Future Management: PASS\n");
        passed++;
    } else {
        printf("❌ Future Management: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}