#include "test_memory_safety_common.h"

// Test metadata for bounds checking
static AsthraTestMetadata bounds_checking_metadata[] = {
    {"test_bounds_checking", __FILE__, __LINE__, "Test array bounds and buffer overflow protection",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

AsthraTestResult test_bounds_checking(AsthraTestContext *context) {
    MemorySafetyContext ctx;
    init_memory_safety_context(&ctx);

    // Create arrays of different sizes
    int *int_array = (int *)safe_allocate_array(&ctx, sizeof(int), 10, "bounds_test:int_array");
    char *char_array = (char *)safe_allocate_array(&ctx, sizeof(char), 5, "bounds_test:char_array");

    if (!int_array || !char_array) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify array properties
    MemoryBlock *int_block = find_memory_block(&ctx, int_array);
    MemoryBlock *char_block = find_memory_block(&ctx, char_array);

    if (!int_block || !char_block) {
        return ASTHRA_TEST_FAIL;
    }

    if (!int_block->is_array || int_block->array_length != 10) {
        return ASTHRA_TEST_FAIL;
    }

    if (!char_block->is_array || char_block->array_length != 5) {
        return ASTHRA_TEST_FAIL;
    }

    // Test valid array accesses
    if (!check_array_bounds(&ctx, int_array, 0, "bounds_test:int[0]")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!check_array_bounds(&ctx, int_array, 9, "bounds_test:int[9]")) {
        return ASTHRA_TEST_FAIL;
    }

    if (!check_array_bounds(&ctx, char_array, 4, "bounds_test:char[4]")) {
        return ASTHRA_TEST_FAIL;
    }

    // Test invalid array accesses
    int initial_violations = ctx.violations_detected;

    if (check_array_bounds(&ctx, int_array, 10, "bounds_test:int[10]")) {
        return ASTHRA_TEST_FAIL; // Should fail - out of bounds
    }

    if (check_array_bounds(&ctx, char_array, 5, "bounds_test:char[5]")) {
        return ASTHRA_TEST_FAIL; // Should fail - out of bounds
    }

    if (ctx.violations_detected <= initial_violations) {
        return ASTHRA_TEST_FAIL; // Should have detected violations
    }

    // Test array access on non-array memory
    void *regular_ptr = safe_allocate(&ctx, 64, "bounds_test:regular");
    if (!regular_ptr) {
        return ASTHRA_TEST_FAIL;
    }

    initial_violations = ctx.violations_detected;
    if (check_array_bounds(&ctx, regular_ptr, 0, "bounds_test:regular[0]")) {
        return ASTHRA_TEST_FAIL; // Should fail - not an array
    }

    if (ctx.violations_detected <= initial_violations) {
        return ASTHRA_TEST_FAIL; // Should have detected violation
    }

    // Test bounds checking after memory state changes
    if (!transfer_ownership(&ctx, int_array, "bounds_test:move_array")) {
        return ASTHRA_TEST_FAIL;
    }

    initial_violations = ctx.violations_detected;
    if (check_array_bounds(&ctx, int_array, 0, "bounds_test:moved_array[0]")) {
        return ASTHRA_TEST_FAIL; // Should fail - moved memory
    }

    if (ctx.violations_detected <= initial_violations) {
        return ASTHRA_TEST_FAIL; // Should have detected violation
    }

    // Test bounds checking after free
    if (!safe_deallocate(&ctx, char_array, "bounds_test:free_array")) {
        return ASTHRA_TEST_FAIL;
    }

    initial_violations = ctx.violations_detected;
    if (check_array_bounds(&ctx, char_array, 0, "bounds_test:freed_array[0]")) {
        return ASTHRA_TEST_FAIL; // Should fail - freed memory
    }

    if (ctx.violations_detected <= initial_violations) {
        return ASTHRA_TEST_FAIL; // Should have detected violation
    }

    // Clean up
    safe_deallocate(&ctx, regular_ptr, "bounds_test:cleanup");

    return ASTHRA_TEST_PASS;
}
