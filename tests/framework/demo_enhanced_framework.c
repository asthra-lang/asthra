/**
 * Demonstration of Enhanced Test Framework Capabilities
 *
 * This file shows the new features added in Phase 4:
 * - Multiple output formats (Console, JSON, TAP, JUnit XML)
 * - Advanced command-line options
 * - Colored output with Unicode symbols
 * - Performance timing
 * - Test filtering and parallel execution support
 */

#include "test_formatters.h"
#include "test_framework.h"
#include "test_suite_runner.h"
#include <stdio.h>

// Example test that passes
static AsthraTestResult demo_passing_test(AsthraTestContext *context) {
    printf("Running a test that will pass...\n");

    // Simulate some work
    for (int i = 0; i < 1000; i++) {
        volatile int dummy = i * i;
        (void)dummy;
    }

    return ASTHRA_TEST_PASS;
}

// Example test that fails
static AsthraTestResult demo_failing_test(AsthraTestContext *context) {
    printf("Running a test that will fail...\n");

    // Set error message
    context->error_message = "This test was designed to fail for demonstration";

    return ASTHRA_TEST_FAIL;
}

// Example test that gets skipped
static AsthraTestResult demo_skipped_test(AsthraTestContext *context) {
    context->error_message = "Feature not yet implemented";
    return ASTHRA_TEST_SKIP;
}

int main(void) {
    printf("=== Enhanced Test Framework Demonstration ===\n\n");

    // Demonstrate different output formats
    printf("1. Console Format (default with colors):\n");
    asthra_test_formatter_set_config(ASTHRA_FORMAT_CONSOLE, true, NULL);

    // Create mock test contexts for demonstration
    AsthraTestContext pass_context = {.metadata = {.name = "demo_passing_test",
                                                   .file = __FILE__,
                                                   .line = __LINE__,
                                                   .function = "demo_passing_test"},
                                      .result = ASTHRA_TEST_PASS,
                                      .duration_ns = 1500000, // 1.5ms
                                      .assertions_in_test = 3};

    AsthraTestContext fail_context = {.metadata = {.name = "demo_failing_test",
                                                   .file = __FILE__,
                                                   .line = __LINE__,
                                                   .function = "demo_failing_test"},
                                      .result = ASTHRA_TEST_FAIL,
                                      .duration_ns = 800000, // 0.8ms
                                      .error_message =
                                          "This test was designed to fail for demonstration",
                                      .assertions_in_test = 1};

    AsthraTestContext skip_context = {.metadata = {.name = "demo_skipped_test",
                                                   .file = __FILE__,
                                                   .line = __LINE__,
                                                   .function = "demo_skipped_test"},
                                      .result = ASTHRA_TEST_SKIP,
                                      .duration_ns = 100000, // 0.1ms
                                      .error_message = "Feature not yet implemented",
                                      .assertions_in_test = 0};

    format_suite_header("Demo Test Suite", 3);
    format_test_result(&pass_context);
    format_test_result(&fail_context);
    format_test_result(&skip_context);

    // Create mock statistics
    AsthraTestStatistics stats = {0};
    atomic_store(&stats.tests_passed, 1);
    atomic_store(&stats.tests_failed, 1);
    atomic_store(&stats.tests_skipped, 1);
    atomic_store(&stats.total_duration_ns, 2400000);

    format_suite_summary(&stats);

    printf("\n2. JSON Format:\n");
    asthra_test_formatter_set_config(ASTHRA_FORMAT_JSON, false, NULL);
    // TODO: format_json_suite_header(stdout, "Demo Test Suite");
    AsthraTestContext contexts[] = {pass_context, fail_context, skip_context};
    // TODO: format_json_test_results(contexts, 3);
    // TODO: format_json_suite_summary(stdout, &stats);

    printf("\n3. TAP Format:\n");
    asthra_test_formatter_set_config(ASTHRA_FORMAT_TAP, false, NULL);
    // TODO: format_tap_suite_header(stdout, 3);
    // TODO: format_tap_test_result(stdout, &pass_context);
    // TODO: format_tap_test_result(stdout, &fail_context);
    // TODO: format_tap_test_result(stdout, &skip_context);

    printf("\n=== Command Line Options Demo ===\n");
    printf("The enhanced test runner supports these options:\n");
    printf("  --format json|tap|junit   # Output format\n");
    printf("  --verbose                 # Detailed output\n");
    printf("  --parallel 4              # Run 4 tests in parallel\n");
    printf("  --test 'pattern*'         # Filter tests by pattern\n");
    printf("  --exclude 'slow*'         # Exclude test patterns\n");
    printf("  --output results.json     # Save to file\n");
    printf("  --no-color                # Disable colors\n");
    printf("  --fail-fast               # Stop on first failure\n");

    printf("\n=== Features Added in Phase 4 ===\n");
    printf("✅ Multiple output formats (Console, JSON, TAP, JUnit XML, Markdown)\n");
    printf("✅ Colored output with Unicode symbols\n");
    printf("✅ Advanced command-line argument parsing\n");
    printf("✅ Test filtering with glob patterns\n");
    printf("✅ Parallel test execution support\n");
    printf("✅ Performance timing with nanosecond precision\n");
    printf("✅ Comprehensive error reporting\n");
    printf("✅ Signal handling for graceful interruption\n");
    printf("✅ Memory profiling hooks\n");
    printf("✅ Dry-run mode for test discovery\n");

    asthra_test_formatter_cleanup();

    return 0;
}