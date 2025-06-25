/**
 * Test Suite Runner - Test Execution
 *
 * Test execution functionality for test suite runner.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_suite_runner_common.h"

// =============================================================================
// TEST EXECUTION
// =============================================================================

void *run_test_thread(void *arg) {
    TestThreadData *data = (TestThreadData *)arg;

    if (data->runner->interrupted) {
        return NULL;
    }

    // Create test context
    data->context = asthra_test_context_create(&data->test->metadata, data->suite->stats);
    if (!data->context) {
        return NULL;
    }

    // Run the test
    asthra_test_context_start(data->context);
    AsthraTestResult result = data->test->test_func(data->context);
    asthra_test_context_end(data->context, result);

    return NULL;
}

AsthraTestResult run_single_test(AsthraTestRunner *runner, AsthraTestSuiteEntry *suite,
                                 AsthraTestEntry *test) {
    if (runner->config.dry_run) {
        printf("Would run: %s::%s\n", suite->suite_name, test->metadata.name);
        return ASTHRA_TEST_PASS;
    }

    TestThreadData thread_data = {.runner = runner, .suite = suite, .test = test, .context = NULL};

    pthread_t thread;
    int create_result = pthread_create(&thread, NULL, run_test_thread, &thread_data);
    if (create_result != 0) {
        printf("Failed to create test thread for %s\n", test->metadata.name);
        return ASTHRA_TEST_ERROR;
    }

    // Wait for completion (simplified - no timeout for now on macOS)
    int join_result = pthread_join(thread, NULL);
    if (join_result != 0) {
        printf("Test %s failed to join\n", test->metadata.name);
        return ASTHRA_TEST_ERROR;
    }

    AsthraTestResult result = thread_data.context ? thread_data.context->result : ASTHRA_TEST_ERROR;

    // Format output
    if (thread_data.context) {
        format_test_result(thread_data.context);
        asthra_test_context_destroy(thread_data.context);
    }

    return result;
}

int run_test_suite(AsthraTestRunner *runner, AsthraTestSuiteEntry *suite) {
    if (!runner->config.quiet) {
        format_suite_header(suite->suite_name, suite->test_count);
    }

    size_t tests_run = 0;
    size_t tests_passed = 0;
    size_t tests_failed = 0;

    for (size_t i = 0; i < suite->test_count; i++) {
        if (runner->interrupted) {
            break;
        }

        AsthraTestEntry *test = &suite->tests[i];
        if (!test->selected) {
            continue;
        }

        tests_run++;
        AsthraTestResult result = run_single_test(runner, suite, test);

        switch (result) {
        case ASTHRA_TEST_PASS:
            tests_passed++;
            break;
        case ASTHRA_TEST_FAIL:
        case ASTHRA_TEST_ERROR:
        case ASTHRA_TEST_TIMEOUT:
            tests_failed++;
            if (runner->config.fail_fast) {
                goto suite_complete;
            }
            break;
        case ASTHRA_TEST_SKIP:
            // Skipped tests don't count as failures
            break;
        case ASTHRA_TEST_RESULT_COUNT:
            // This is not a valid test result, treat as error
            tests_failed++;
            break;
        }
    }

suite_complete:
    if (!runner->config.quiet) {
        format_suite_summary(suite->stats);
    }

    return tests_failed > 0 ? 1 : 0;
}