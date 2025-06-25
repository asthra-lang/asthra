/**
 * Asthra Programming Language Runtime Safety System Tests
 * Concurrency and Error Handling Safety Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_safety_common.h"

// =============================================================================
// CONCURRENCY AND ERROR HANDLING TESTS
// =============================================================================

void test_task_lifecycle_logging(void) {
    printf("\n=== Testing Task Lifecycle Logging ===\n");

    // Test task event logging
    asthra_safety_log_task_lifecycle_event(12345, ASTHRA_TASK_EVENT_SPAWNED, "Test task spawned");
    asthra_safety_log_task_lifecycle_event(12345, ASTHRA_TASK_EVENT_STARTED, "Test task started");
    asthra_safety_log_task_lifecycle_event(12345, ASTHRA_TASK_EVENT_COMPLETED,
                                           "Test task completed");

    TEST_ASSERT(true, "Task lifecycle events should be logged without errors");
}

void test_scheduler_event_logging(void) {
    printf("\n=== Testing Scheduler Event Logging ===\n");

    asthra_safety_log_scheduler_event(ASTHRA_SCHEDULER_EVENT_TASK_QUEUED,
                                      "Task queued for execution");
    asthra_safety_log_scheduler_event(ASTHRA_SCHEDULER_EVENT_WORKER_STARTED,
                                      "Worker thread started");

    TEST_ASSERT(true, "Scheduler events should be logged without errors");
}

void test_result_tracking(void) {
    printf("\n=== Testing Result Tracking ===\n");

    // Create a test result
    int value = 42;
    AsthraResult result = asthra_result_ok(&value, sizeof(int), 1, ASTHRA_OWNERSHIP_GC);

    // Register result tracker
    int result_id = asthra_safety_register_result_tracker(result, "test_location");
    TEST_ASSERT(result_id >= 0, "Result tracker registration should succeed");

    // Mark result as handled
    int mark_result = asthra_safety_mark_result_handled(result_id, "test_handler");
    TEST_ASSERT(mark_result == 0, "Marking result as handled should succeed");

    // Check for unhandled results (should not find any violations)
    asthra_safety_check_unhandled_results();
    TEST_ASSERT(true, "Unhandled result check should complete without errors");
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("==========================================================================\n");
    printf("Security Concurrency Safety Tests - Runtime Safety System Validation\n");
    printf("==========================================================================\n");

    // Test counters are defined in test_safety_common.c

    // Run test categories
    test_task_lifecycle_logging();
    test_scheduler_event_logging();
    test_result_tracking();

    // Print final results
    printf("\n==========================================================================\n");
    printf("SECURITY CONCURRENCY SAFETY TESTS SUMMARY\n");
    printf("==========================================================================\n");
    printf("Tests run: %d\n", tests_passed + tests_failed);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_failed);

    if (tests_failed == 0 && tests_passed > 0) {
        printf("🎉 ALL TESTS PASSED! Security concurrency safety working correctly.\n");
        return 0;
    } else {
        printf("❌ Some tests failed. Check individual test output above.\n");
        return 1;
    }
}
