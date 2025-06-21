/**
 * Test file for future chaining
 * Tests complex future chaining and async patterns
 */

#include "async_await_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_future_chaining", __FILE__, __LINE__, "Test complex future chaining and async patterns", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions
static AsthraTestResult test_future_chaining(AsthraTestContext* context) {
    AsyncSystemContext ctx;
    init_async_system_context(&ctx);
    
    // Create multiple async functions for chaining
    AsyncFunction* func1 = create_async_function(&ctx, "step1");
    AsyncFunction* func2 = create_async_function(&ctx, "step2");
    AsyncFunction* func3 = create_async_function(&ctx, "step3");
    
    // Create futures for each step
    AsyncFuture* future1 = create_future(&ctx);
    AsyncFuture* future2 = create_future(&ctx);
    AsyncFuture* future3 = create_future(&ctx);
    
    // Set return futures
    func1->return_future = future1;
    func2->return_future = future2;
    func3->return_future = future3;
    
    // Chain the operations: func2 awaits func1, func3 awaits func2
    await_future(&ctx, func2, future1);
    await_future(&ctx, func3, future2);
    
    // Add continuations to simulate future chaining
    add_continuation(future1, func2->function_id);
    add_continuation(future2, func3->function_id);
    
    if (future1->continuation_count != 1 || future2->continuation_count != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Complete the chain step by step
    int step1_result = 100;
    complete_future(&ctx, future1, &step1_result);
    
    if (!is_future_complete(future1)) {
        return ASTHRA_TEST_FAIL;
    }
    
    int step2_result = 200;
    complete_future(&ctx, future2, &step2_result);
    
    if (!is_future_complete(future2)) {
        return ASTHRA_TEST_FAIL;
    }
    
    int step3_result = 300;
    complete_future(&ctx, future3, &step3_result);
    
    if (!is_future_complete(future3)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify all futures completed successfully
    if (ctx.completed_futures != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify no pending futures
    if (count_pending_futures(&ctx) != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test complex await scenario
    AsyncFunction* complex_func = create_async_function(&ctx, "complex_operation");
    AsyncFuture* dep1 = create_future(&ctx);
    AsyncFuture* dep2 = create_future(&ctx);
    AsyncFuture* dep3 = create_future(&ctx);
    
    // Function awaits multiple dependencies
    await_future(&ctx, complex_func, dep1);
    await_future(&ctx, complex_func, dep2);
    await_future(&ctx, complex_func, dep3);
    
    if (complex_func->awaited_count != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Complete all dependencies
    complete_future(&ctx, dep1, NULL);
    complete_future(&ctx, dep2, NULL);
    complete_future(&ctx, dep3, NULL);
    
    // Verify final state
    if (ctx.async_function_count != 4 || ctx.future_count != 6) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Future Chaining Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_future_chaining(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Future Chaining: PASS\n");
        passed++;
    } else {
        printf("❌ Future Chaining: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}