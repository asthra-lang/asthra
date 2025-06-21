/**
 * Test file for await operations
 * Tests await operations and future completion
 */

#include "async_await_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_await_operations", __FILE__, __LINE__, "Test await operations and future completion", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

// Test functions
static AsthraTestResult test_await_operations(AsthraTestContext* context) {
    AsyncSystemContext ctx;
    init_async_system_context(&ctx);
    
    // Create async function and futures
    AsyncFunction* async_func = create_async_function(&ctx, "async_operation");
    AsyncFuture* future1 = create_future(&ctx);
    AsyncFuture* future2 = create_future(&ctx);
    AsyncFuture* future3 = create_future(&ctx);
    
    // Test awaiting futures
    await_future(&ctx, async_func, future1);
    await_future(&ctx, async_func, future2);
    
    if (async_func->awaited_count != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!future1->is_awaited || !future2->is_awaited) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (future3->is_awaited) {
        return ASTHRA_TEST_FAIL; // Should not be awaited
    }
    
    if (ctx.total_await_operations != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Complete one of the awaited futures
    const char* result_string = "completed successfully";
    complete_future(&ctx, future1, (void*)result_string);
    
    if (!is_future_complete(future1)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (is_future_complete(future2) || is_future_complete(future3)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test continuations
    add_continuation(future2, 1001);
    add_continuation(future2, 1002);
    add_continuation(future2, 1003);
    
    if (future2->continuation_count != 3 ||
        future2->continuations[0] != 1001 ||
        future2->continuations[1] != 1002 ||
        future2->continuations[2] != 1003) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test pending future count
    int pending = count_pending_futures(&ctx);
    if (pending != 2) { // future2 and future3 should be pending
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Await Operations Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_await_operations(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Await Operations: PASS\n");
        passed++;
    } else {
        printf("❌ Await Operations: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}