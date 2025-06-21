/**
 * Test file for async function creation
 * Tests creating async functions and basic structure
 */

#include "async_await_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_async_function_creation", __FILE__, __LINE__, "Test creating async functions and basic structure", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions
static AsthraTestResult test_async_function_creation(AsthraTestContext* context) {
    AsyncSystemContext ctx;
    init_async_system_context(&ctx);
    
    // Create an async function
    AsyncFunction* async_func = create_async_function(&ctx, "fetch_data");
    if (!async_func) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!async_func->is_async || strcmp(async_func->function_name, "fetch_data") != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (async_func->current_state != ASYNC_STATE_PENDING) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (async_func->function_id == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create another async function
    AsyncFunction* async_func2 = create_async_function(&ctx, "process_data");
    if (!async_func2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify unique function IDs
    if (async_func->function_id == async_func2->function_id) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test generator flag
    async_func2->is_generator = true;
    if (!async_func2->is_generator) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test yield points
    add_yield_point(async_func2, 100);
    add_yield_point(async_func2, 200);
    add_yield_point(async_func2, 300);
    
    if (async_func2->yield_count != 3 ||
        async_func2->yield_points[0] != 100 ||
        async_func2->yield_points[1] != 200 ||
        async_func2->yield_points[2] != 300) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify context state
    if (ctx.async_function_count != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Async Function Creation Tests (Minimal Framework) ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_async_function_creation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Async Function Creation: PASS\n");
        passed++;
    } else {
        printf("❌ Async Function Creation: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
}