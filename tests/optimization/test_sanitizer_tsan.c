/**
 * Asthra Programming Language
 * ThreadSanitizer (TSan) Integration Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This module contains tests for ThreadSanitizer detection capabilities.
 */

#include "test_sanitizer_common.h"

AsthraTestResult test_tsan_race_condition_detection(AsthraTestContext *context) {
    // Test race condition detection with proper synchronization

    pthread_t threads[THREAD_COUNT];
    int thread_ids[THREAD_COUNT];

    // Reset global counter
    global_counter = 0;

    // Create threads
    for (int i = 0; i < THREAD_COUNT; i++) {
        thread_ids[i] = i;
        int result = pthread_create(&threads[i], NULL, thread_increment_function, &thread_ids[i]);
        if (!ASTHRA_TEST_ASSERT_EQ(context, result, 0,
                                   "Thread creation should succeed for thread %d", i)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Wait for all threads to complete
    for (int i = 0; i < THREAD_COUNT; i++) {
        int result = pthread_join(threads[i], NULL);
        if (!ASTHRA_TEST_ASSERT_EQ(context, result, 0, "Thread join should succeed for thread %d",
                                   i)) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Verify final counter value
    int expected_value = THREAD_COUNT * ITERATIONS;
    if (!ASTHRA_TEST_ASSERT_EQ(context, global_counter, expected_value,
                               "Counter should have correct final value")) {
        return ASTHRA_TEST_FAIL;
    }

    printf("Thread synchronization test completed successfully\n");
    return ASTHRA_TEST_PASS;
}

AsthraTestResult test_tsan_data_race_infrastructure(AsthraTestContext *context) {
    // Test that TSAN infrastructure is available for data race detection

    pthread_t thread1, thread2;

    // Reset counter
    global_counter = 0;

    // Create two threads that would race (but we make them safe for testing)
    int result1 = pthread_create(&thread1, NULL, thread_unsafe_increment_function, NULL);
    int result2 = pthread_create(&thread2, NULL, thread_unsafe_increment_function, NULL);

    if (!asthra_test_assert_int_eq(context, result1, 0, "First thread creation should succeed") ||
        !asthra_test_assert_int_eq(context, result2, 0, "Second thread creation should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Wait for threads
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);

    // Verify we can detect the completion
    if (!ASTHRA_TEST_ASSERT(context, global_counter == 200,
                            "Both threads should complete their work")) {
        return ASTHRA_TEST_FAIL;
    }

    printf("Data race detection infrastructure verified\n");
    return ASTHRA_TEST_PASS;
}

int main(void) {
    printf("=== Asthra ThreadSanitizer Test ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Create test metadata for context
    AsthraTestMetadata metadata = {.name = "tsan_tests",
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = "main",
                                   .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                   .timeout_ns = 30000000000ULL, // 30 seconds
                                   .skip = false,
                                   .skip_reason = NULL};

    // Create test context
    AsthraTestContext *context = asthra_test_context_create(&metadata, stats);
    if (!context) {
        fprintf(stderr, "Failed to create test context\n");
        asthra_test_statistics_destroy(stats);
        return 1;
    }

    // Run tests
    AsthraTestResult results[] = {test_tsan_race_condition_detection(context),
                                  test_tsan_data_race_infrastructure(context)};

    // Print results
    const char *test_names[] = {"Race Condition Detection", "Data Race Infrastructure"};

    size_t test_count = sizeof(results) / sizeof(results[0]);
    size_t passed = 0;

    printf("\n=== Test Results ===\n");
    for (size_t i = 0; i < test_count; i++) {
        printf("[%s] %s\n", results[i] == ASTHRA_TEST_PASS ? "PASS" : "FAIL", test_names[i]);
        if (results[i] == ASTHRA_TEST_PASS) {
            passed++;
        }
    }

    printf("\n=== Summary ===\n");
    printf("Tests run: %zu\n", test_count);
    printf("Passed: %zu\n", passed);
    printf("Failed: %zu\n", test_count - passed);
    printf("Pass rate: %.1f%%\n", (double)passed / (double)test_count * 100.0);

    // Cleanup
    asthra_test_context_destroy(context);
    asthra_test_statistics_destroy(stats);

    return (passed == test_count) ? 0 : 1;
}
