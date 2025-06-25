/**
 * Asthra Programming Language
 * Test Framework - Test Suite Management Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test suite configuration and execution
 * Enhanced for Testing Framework Standardization Plan Phase 1
 */

#include "test_suite.h"
#include "test_context.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// ASTHRA TEST SUITE STRUCTURE
// =============================================================================

#define MAX_SUITE_TESTS 256

typedef struct {
    char *name;
    char *description;
    AsthraTestFunction func;
    AsthraTestSeverity severity;
    void *user_data; // NEW: For adapter pattern support
} SuiteTestEntry;

struct AsthraTestSuite {
    char *name;
    char *description;
    AsthraTestFunction setup_func;
    AsthraTestFunction teardown_func;
    SuiteTestEntry tests[MAX_SUITE_TESTS];
    size_t test_count;
    AsthraTestStatistics stats;
    AsthraTestSuiteConfig config;             // NEW: Store configuration
    AsthraAnalysisCallback analysis_callback; // NEW: Custom analysis callback
    bool config_set;                          // NEW: Track if config has been explicitly set
};

// =============================================================================
// ENHANCED TEST SUITE MANAGEMENT
// =============================================================================

AsthraTestSuite *asthra_test_suite_create(const char *name, const char *description) {
    AsthraTestSuite *suite = malloc(sizeof(AsthraTestSuite));
    if (!suite)
        return NULL;

    memset(suite, 0, sizeof(AsthraTestSuite));

    suite->name = strdup(name ? name : "Unnamed Suite");
    suite->description = strdup(description ? description : "No description");

    if (!suite->name || !suite->description) {
        free(suite->name);
        free(suite->description);
        free(suite);
        return NULL;
    }

    // Initialize with default configuration
    suite->config = asthra_test_suite_config_default();
    suite->config_set = false;
    suite->analysis_callback = NULL;

    // Initialize statistics
    memset(&suite->stats, 0, sizeof(AsthraTestStatistics));

    return suite;
}

// NEW: Lightweight mode suite creation
AsthraTestSuite *asthra_test_suite_create_lightweight(const char *name) {
    AsthraTestSuite *suite = asthra_test_suite_create(name, "Lightweight test suite");
    if (!suite)
        return NULL;

    // Configure for lightweight mode
    suite->config = asthra_test_suite_config_lightweight(name);
    suite->config_set = true;

    return suite;
}

// NEW: Detailed mode suite creation
AsthraTestSuite *asthra_test_suite_create_detailed(const char *name, const char *description) {
    AsthraTestSuite *suite = asthra_test_suite_create(name, description);
    if (!suite)
        return NULL;

    // Configure for detailed mode
    suite->config = asthra_test_suite_config_detailed(name, description);
    suite->config_set = true;

    return suite;
}

void asthra_test_suite_destroy(AsthraTestSuite *suite) {
    if (!suite)
        return;

    free(suite->name);
    free(suite->description);

    // Free test names and descriptions
    for (size_t i = 0; i < suite->test_count; i++) {
        free(suite->tests[i].name);
        free(suite->tests[i].description);
    }

    free(suite);
}

// NEW: Enhanced configuration
void asthra_test_suite_set_config(AsthraTestSuite *suite, const AsthraTestSuiteConfig *config) {
    if (!suite || !config)
        return;

    suite->config = *config;
    suite->config_set = true;
}

// NEW: Set analysis callback
void asthra_test_suite_set_analysis_callback(AsthraTestSuite *suite,
                                             AsthraAnalysisCallback callback) {
    if (!suite)
        return;
    suite->analysis_callback = callback;
}

void asthra_test_suite_set_setup(AsthraTestSuite *suite, AsthraTestFunction setup_func) {
    if (suite)
        suite->setup_func = setup_func;
}

void asthra_test_suite_set_teardown(AsthraTestSuite *suite, AsthraTestFunction teardown_func) {
    if (suite)
        suite->teardown_func = teardown_func;
}

void asthra_test_suite_add_test(AsthraTestSuite *suite, const char *test_name,
                                const char *description, AsthraTestFunction test_func) {
    asthra_test_suite_add_test_with_data(suite, test_name, description, test_func, NULL);
}

// NEW: Add test with user data for adapter pattern
void asthra_test_suite_add_test_with_data(AsthraTestSuite *suite, const char *test_name,
                                          const char *description, AsthraTestFunction test_func,
                                          void *user_data) {
    if (!suite || !test_name || !test_func || suite->test_count >= MAX_SUITE_TESTS) {
        return;
    }

    size_t index = suite->test_count++;
    suite->tests[index].name = strdup(test_name);
    suite->tests[index].description = strdup(description ? description : "");
    suite->tests[index].func = test_func;
    suite->tests[index].severity = ASTHRA_TEST_SEVERITY_HIGH; // Default severity
    suite->tests[index].user_data = user_data;
}

AsthraTestResult asthra_test_suite_run(AsthraTestSuite *suite) {
    if (!suite)
        return ASTHRA_TEST_ERROR;

    // Output based on reporting level
    if (suite->config.reporting_level >= ASTHRA_TEST_REPORTING_STANDARD) {
        printf("Running test suite: %s\n", suite->name);
        if (suite->description && strlen(suite->description) > 0) {
            printf("Description: %s\n", suite->description);
        }
        if (suite->config.reporting_level >= ASTHRA_TEST_REPORTING_DETAILED) {
            printf("Mode: %s\n", suite->config.lightweight_mode      ? "Lightweight"
                                 : suite->config.statistics_tracking ? "Detailed"
                                                                     : "Standard");
            printf("Timeout: %llu ns\n", (unsigned long long)suite->config.default_timeout_ns);
        }
        printf("\n");
    }

    bool all_passed = true;
    size_t passed = 0, failed = 0, skipped = 0, errors = 0;

    for (size_t i = 0; i < suite->test_count; i++) {
        // Create metadata for the test
        AsthraTestMetadata metadata = {.name = suite->tests[i].name,
                                       .file = "suite_test",
                                       .line = 0,
                                       .function = suite->tests[i].name,
                                       .severity = suite->tests[i].severity,
                                       .timeout_ns = suite->config.default_timeout_ns,
                                       .skip = false,
                                       .skip_reason = NULL};

        // Run setup if provided
        if (suite->setup_func) {
            AsthraTestContext *setup_context = asthra_test_context_create(&metadata, &suite->stats);
            if (setup_context) {
                // Pass user data to setup if available
                if (suite->tests[i].user_data) {
                    setup_context->user_data = suite->tests[i].user_data;
                }
                suite->setup_func(setup_context);
                asthra_test_context_destroy(setup_context);
            }
        }

        // Run the test
        AsthraTestContext *test_context = asthra_test_context_create(&metadata, &suite->stats);
        AsthraTestResult result = ASTHRA_TEST_ERROR;

        if (test_context) {
            // Pass user data to test if available
            if (suite->tests[i].user_data) {
                test_context->user_data = suite->tests[i].user_data;
            }
            result = suite->tests[i].func(test_context);
            asthra_test_context_destroy(test_context);
        }

        // Run teardown if provided
        if (suite->teardown_func) {
            AsthraTestContext *teardown_context =
                asthra_test_context_create(&metadata, &suite->stats);
            if (teardown_context) {
                // Pass user data to teardown if available
                if (suite->tests[i].user_data) {
                    teardown_context->user_data = suite->tests[i].user_data;
                }
                suite->teardown_func(teardown_context);
                asthra_test_context_destroy(teardown_context);
            }
        }

        // Update counters
        switch (result) {
        case ASTHRA_TEST_PASS:
            passed++;
            break;
        case ASTHRA_TEST_FAIL:
            failed++;
            all_passed = false;
            break;
        case ASTHRA_TEST_SKIP:
            skipped++;
            break;
        case ASTHRA_TEST_ERROR:
        case ASTHRA_TEST_TIMEOUT:
            errors++;
            all_passed = false;
            break;
        case ASTHRA_TEST_RESULT_COUNT:
            // This is not a valid test result, treat as error
            errors++;
            all_passed = false;
            break;
        }

        // Output based on reporting level
        if (suite->config.reporting_level >= ASTHRA_TEST_REPORTING_MINIMAL) {
            const char *result_str = (result == ASTHRA_TEST_PASS)      ? "PASS"
                                     : (result == ASTHRA_TEST_FAIL)    ? "FAIL"
                                     : (result == ASTHRA_TEST_SKIP)    ? "SKIP"
                                     : (result == ASTHRA_TEST_ERROR)   ? "ERROR"
                                     : (result == ASTHRA_TEST_TIMEOUT) ? "TIMEOUT"
                                                                       : "UNKNOWN";

            if (suite->config.reporting_level == ASTHRA_TEST_REPORTING_JSON) {
                printf("{\"test\":\"%s\",\"result\":\"%s\"}\n", suite->tests[i].name, result_str);
            } else {
                printf("[%s] %s", result_str, suite->tests[i].name);
                if (suite->config.reporting_level >= ASTHRA_TEST_REPORTING_DETAILED &&
                    suite->tests[i].description && strlen(suite->tests[i].description) > 0) {
                    printf(" - %s", suite->tests[i].description);
                }
                printf("\n");
            }
        }

        // Stop on failure if configured
        if (suite->config.stop_on_failure && result != ASTHRA_TEST_PASS &&
            result != ASTHRA_TEST_SKIP) {
            if (suite->config.reporting_level >= ASTHRA_TEST_REPORTING_STANDARD) {
                printf("Stopping on first failure as configured.\n");
            }
            break;
        }
    }

    // Update suite statistics
    suite->stats.total_tests = suite->test_count;
    suite->stats.passed_tests = passed;
    suite->stats.failed_tests = failed;
    suite->stats.skipped_tests = skipped;
    suite->stats.error_tests = errors;

    // Output results based on reporting level
    if (suite->config.reporting_level >= ASTHRA_TEST_REPORTING_MINIMAL) {
        if (suite->config.reporting_level == ASTHRA_TEST_REPORTING_JSON) {
            printf("{\"suite\":\"%s\",\"total\":%zu,\"passed\":%zu,\"failed\":%zu,\"skipped\":%zu,"
                   "\"errors\":%zu}\n",
                   suite->name, suite->test_count, passed, failed, skipped, errors);
        } else {
            printf("\nTest Results: %zu/%zu passed", passed, suite->test_count);
            if (failed > 0)
                printf(", %zu failed", failed);
            if (skipped > 0)
                printf(", %zu skipped", skipped);
            if (errors > 0)
                printf(", %zu errors", errors);
            printf("\n");
        }
    }

    // Run custom analysis if provided
    if (suite->analysis_callback) {
        suite->analysis_callback(suite, &suite->stats);
    }

    return all_passed ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

// NEW: Convenience function that runs and exits
int asthra_test_suite_run_and_exit(AsthraTestSuite *suite) {
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}

// =============================================================================
// ENHANCED TEST SUITE CONFIGURATION
// =============================================================================

AsthraTestSuiteConfig asthra_test_suite_config_default(void) {
    // C17 designated initializer for default configuration
    return (AsthraTestSuiteConfig){.name = "Default Test Suite",
                                   .description = "Asthra test suite with default configuration",
                                   .lightweight_mode = false,
                                   .custom_main = false,
                                   .statistics_tracking = true,
                                   .reporting_level = ASTHRA_TEST_REPORTING_STANDARD,
                                   .default_timeout_ns = 30000000000ULL, // 30 seconds
                                   .parallel_execution = false,
                                   .stop_on_failure = false,
                                   .verbose_output = false,
                                   .json_output = false,
                                   .max_parallel_tests = 4,
                                   .statistics = NULL};
}

AsthraTestSuiteConfig asthra_test_suite_config_create(const char *name, const char *description) {
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = name ? name : "Unnamed Test Suite";
    config.description = description ? description : "No description provided";
    return config;
}

// NEW: Lightweight mode configuration
AsthraTestSuiteConfig asthra_test_suite_config_lightweight(const char *name) {
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = name ? name : "Lightweight Test Suite";
    config.description = "Lightweight test suite with minimal overhead";
    config.lightweight_mode = true;
    config.statistics_tracking = false;
    config.reporting_level = ASTHRA_TEST_REPORTING_MINIMAL;
    config.default_timeout_ns = 10000000000ULL; // 10 seconds
    config.verbose_output = false;
    return config;
}

// NEW: Detailed mode configuration
AsthraTestSuiteConfig asthra_test_suite_config_detailed(const char *name, const char *description) {
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = name ? name : "Detailed Test Suite";
    config.description =
        description ? description : "Detailed test suite with comprehensive reporting";
    config.lightweight_mode = false;
    config.statistics_tracking = true;
    config.reporting_level = ASTHRA_TEST_REPORTING_DETAILED;
    config.default_timeout_ns = 60000000000ULL; // 60 seconds
    config.verbose_output = true;
    return config;
}

// =============================================================================
// TEST EXECUTION (Implementation preserved from original)
// =============================================================================

AsthraTestResult asthra_test_run_single(AsthraTestFunction test_func,
                                        const AsthraTestMetadata *metadata,
                                        AsthraTestStatistics *stats) {
    printf("DEBUG: asthra_test_run_single called for %s\n", metadata ? metadata->name : "NULL");
    fflush(stdout);

    if (!test_func || !metadata) {
        return ASTHRA_TEST_ERROR;
    }

    printf("DEBUG: Creating context...\n");
    fflush(stdout);

    AsthraTestContext *context = asthra_test_context_create(metadata, stats);
    if (!context) {
        return ASTHRA_TEST_ERROR;
    }

    printf("DEBUG: Starting context...\n");
    fflush(stdout);

    // Start test context for statistics tracking
    asthra_test_context_start(context);

    printf("DEBUG: Calling test function...\n");
    fflush(stdout);

    AsthraTestResult result = test_func(context);

    printf("DEBUG: Test function returned %d\n", result);
    fflush(stdout);

    // End test context for statistics tracking
    asthra_test_context_end(context, result);

    printf("DEBUG: Destroying context...\n");
    fflush(stdout);

    asthra_test_context_destroy(context);

    printf("DEBUG: asthra_test_run_single completed\n");
    fflush(stdout);

    return result;
}

AsthraTestResult asthra_test_run_suite(AsthraTestFunction *tests,
                                       const AsthraTestMetadata *metadata_array, size_t test_count,
                                       const AsthraTestSuiteConfig *config) {
    printf("DEBUG: asthra_test_run_suite called with %zu tests\n", test_count);
    fflush(stdout);

    if (!tests || !metadata_array || test_count == 0) {
        return ASTHRA_TEST_ERROR;
    }

    // Use statistics from config if available, otherwise create local stats
    AsthraTestStatistics *stats = (config && config->statistics) ? config->statistics : NULL;
    AsthraTestStatistics local_stats = {0};
    if (!stats) {
        stats = &local_stats;
    }
    bool all_passed = true;

    printf("DEBUG: About to start test loop\n");
    fflush(stdout);

    for (size_t i = 0; i < test_count; i++) {
        printf("DEBUG: About to run test %zu: %s\n", i, metadata_array[i].name);
        fflush(stdout);

        AsthraTestResult result = asthra_test_run_single(tests[i], &metadata_array[i], stats);

        printf("DEBUG: Test %zu completed with result %d\n", i, result);
        fflush(stdout);

        if (result == ASTHRA_TEST_FAIL || result == ASTHRA_TEST_ERROR ||
            result == ASTHRA_TEST_TIMEOUT) {
            all_passed = false;
            if (config && config->stop_on_failure) {
                break;
            }
        }
        // Note: ASTHRA_TEST_SKIP does not cause all_passed to become false
    }

    printf("DEBUG: Test loop completed\n");
    fflush(stdout);

    return all_passed ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

AsthraTestResult asthra_test_run_suite_wrapper(AsthraTestSuite *suite) {
    return asthra_test_suite_run(suite);
}
