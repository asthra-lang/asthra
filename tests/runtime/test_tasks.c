/**
 * Asthra Programming Language Runtime v1.2
 * Task System Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_common.h"

// =============================================================================
// TASK SYSTEM TESTS
// =============================================================================

static AsthraResult test_task_function(void *args) {
    int *input = (int *)args;
    int *result = asthra_alloc(sizeof(int), ASTHRA_ZONE_GC);
    if (!result) {
        return asthra_result_err("Memory allocation failed", 0, ASTHRA_TYPE_STRING,
                                 ASTHRA_OWNERSHIP_C);
    }

    *result = (*input) * 2;
    asthra_sleep_ms(100); // Simulate some work

    return asthra_result_ok(result, sizeof(int), ASTHRA_TYPE_I32, ASTHRA_OWNERSHIP_GC);
}

int test_task_system(void) {
    int result = asthra_runtime_init(NULL);
    TEST_ASSERT(result == 0, "Runtime initialization failed");

    // Test task spawning
    int input = 21;
    AsthraTaskHandle handle = asthra_spawn_task(test_task_function, &input, sizeof(int));
    TEST_ASSERT(handle.task_id != 0, "Task spawning failed");

    // Test task completion checking
    while (!asthra_task_is_completed(handle)) {
        asthra_sleep_ms(10);
    }

    // Test task awaiting
    AsthraResult task_result = asthra_task_await(handle);
    TEST_ASSERT(asthra_result_is_ok(task_result), "Task should complete successfully");

    int *output = (int *)asthra_result_unwrap_ok(task_result);
    TEST_ASSERT(output != NULL, "Task result should not be NULL");
    TEST_ASSERT(*output == 42, "Task result should be 42");

    asthra_runtime_cleanup();
    TEST_PASS("Task system");
}

// Test function declarations for external use
int test_task_system(void);
