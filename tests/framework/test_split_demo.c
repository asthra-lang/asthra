#include "test_framework_new.h"
#include <stdio.h>

// Define a simple test using the split framework
ASTHRA_TEST_DEFINE(test_basic_functionality, ASTHRA_TEST_SEVERITY_MEDIUM) {
    // Test basic assertions
    asthra_test_assert_int_eq(context, 42, 42, "Basic equality test should pass");
    ASTHRA_TEST_ASSERT_EQ(context, "hello", "hello", "String equality test should pass");
    ASTHRA_TEST_ASSERT_RANGE(context, 50, 1, 100, "Range test should pass");

    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_failure_case, ASTHRA_TEST_SEVERITY_LOW) {
    // This test intentionally fails to demonstrate error handling
    asthra_test_assert_int_eq(context, 1, 2, "This assertion should fail");
    ASTHRA_TEST_PASS(context);
}

ASTHRA_TEST_DEFINE(test_skip_case, ASTHRA_TEST_SEVERITY_LOW) {
    ASTHRA_TEST_SKIP(context, "This test is intentionally skipped for demo");
}

int main(void) {
    printf("=== Asthra Test Framework Split Demo ===\n\n");

    // Create test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        printf("Failed to create test statistics\n");
        return 1;
    }

    // Configure test suite
    AsthraTestSuiteConfig config = asthra_test_suite_config_create(
        "Split Framework Demo Suite", "Demonstration of the split test framework modules");
    config.statistics = stats;
    config.verbose_output = true;
    config.stop_on_failure = false; // Continue even if tests fail

    // Set up test arrays
    AsthraTestFunction tests[] = {test_basic_functionality, test_failure_case, test_skip_case};

    AsthraTestMetadata metadata[] = {test_basic_functionality_metadata, test_failure_case_metadata,
                                     test_skip_case_metadata};

    // Run the test suite
    printf("Running test suite: %s\n", config.name);
    printf("Description: %s\n\n", config.description);

    AsthraTestResult suite_result =
        asthra_test_run_suite(tests, metadata, sizeof(tests) / sizeof(tests[0]), &config);

    // Print detailed statistics
    printf("\n");
    asthra_test_statistics_print(stats, false);

    // Print JSON format as well
    printf("\nJSON Format:\n");
    asthra_test_statistics_print(stats, true);

    // Clean up
    asthra_test_statistics_destroy(stats);

    printf("\nSuite result: %s\n", (suite_result == ASTHRA_TEST_PASS) ? "PASS" : "FAIL");

    printf("\n=== Demo Complete ===\n");
    return 0;
}
