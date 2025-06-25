/**
 * Real Program Test Suite Management Implementation
 *
 * Contains core functions for managing test suites, test results,
 * and test execution.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_suite.h"
#include "semantic_analyzer_core.h"
#include "semantic_core.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// CORE TEST SUITE FUNCTIONS
// =============================================================================

RealProgramTestSuite *create_real_program_test_suite(const char *suite_name,
                                                     const char *description) {
    if (!suite_name || !description) {
        return NULL;
    }

    RealProgramTestSuite *suite = malloc(sizeof(RealProgramTestSuite));
    if (!suite) {
        return NULL;
    }

    // Initialize the test suite
    suite->tests = NULL;
    suite->test_count = 0;
    suite->passed = 0;
    suite->failed = 0;
    suite->skipped = 0;
    suite->stop_on_failure = false;
    suite->total_parse_time_ms = 0.0;
    suite->average_parse_time_ms = 0.0;

    // Allocate and copy strings
    size_t name_len = strlen(suite_name) + 1;
    size_t desc_len = strlen(description) + 1;

    char *name_copy = malloc(name_len);
    char *desc_copy = malloc(desc_len);

    if (!name_copy || !desc_copy) {
        free(suite);
        free(name_copy);
        free(desc_copy);
        return NULL;
    }

    strcpy(name_copy, suite_name);
    strcpy(desc_copy, description);

    suite->suite_name = name_copy;
    suite->description = desc_copy;

    return suite;
}

bool add_test_to_suite(RealProgramTestSuite *suite, const RealProgramTest *test) {
    if (!suite || !test) {
        return false;
    }

    // Reallocate test array
    size_t new_size = (suite->test_count + 1) * sizeof(RealProgramTest);
    RealProgramTest *new_tests = realloc(suite->tests, new_size);
    if (!new_tests) {
        return false;
    }

    suite->tests = new_tests;

    // Copy the test (shallow copy for now)
    suite->tests[suite->test_count] = *test;
    suite->test_count++;

    return true;
}

void cleanup_test_suite(RealProgramTestSuite *suite) {
    if (!suite) {
        return;
    }

    // Free test array
    if (suite->tests) {
        free(suite->tests);
    }

    // Free strings (cast away const)
    if (suite->suite_name) {
        free((void *)suite->suite_name);
    }
    if (suite->description) {
        free((void *)suite->description);
    }

    free(suite);
}

void cleanup_test_result(RealProgramTestResult *result) {
    if (!result) {
        return;
    }

    // Free error message if allocated
    if (result->error_message) {
        free((void *)result->error_message);
    }

    // Free failure reason if allocated
    if (result->failure_reason) {
        free((void *)result->failure_reason);
    }

    // Clean up parse result if present
    if (result->parse_result) {
        cleanup_parse_result(result->parse_result);
        free(result->parse_result);
    }

    // Free AST if present
    if (result->ast) {
        ast_free_node(result->ast);
    }
}

// =============================================================================
// CORE VALIDATION FUNCTIONS
// =============================================================================

RealProgramTestResult validate_complete_program(const char *source, const char *test_name,
                                                const TestSuiteConfig *config) {
    RealProgramTestResult result = {0};

    if (!source || !test_name) {
        result.success = false;
        result.error_message = strdup("Invalid input: source or test_name is NULL");
        return result;
    }

    uint64_t start_time = get_timestamp_ms();

    if (config && config->verbose_output) {
        fprintf(config->output_stream, "Testing program: %s\n", test_name);
        fprintf(config->output_stream, "Source code:\n%s\n", source);
        fprintf(config->output_stream, "---\n");
    }

    // Parse the program using the real parser
    ParseResult parse_result = parse_string(source);

    uint64_t end_time = get_timestamp_ms();
    result.execution_time_ms = calculate_execution_time(start_time, end_time);

    // Analyze the parse result
    result.success = parse_result.success;
    result.ast = parse_result.ast;

    // Copy parse result for detailed analysis
    result.parse_result = malloc(sizeof(ParseResult));
    if (result.parse_result) {
        *result.parse_result = parse_result;
    }

    // Fill performance metrics
    result.metrics.parse_time_ms = (size_t)result.execution_time_ms;
    result.metrics.lines_of_code = 0;
    result.metrics.errors_detected = (size_t)parse_result.error_count;

    // Count lines of code
    const char *ptr = source;
    while (*ptr) {
        if (*ptr == '\n') {
            result.metrics.lines_of_code++;
        }
        ptr++;
    }
    result.metrics.lines_of_code++; // Count last line even if no newline

    // Calculate parse speed
    if (result.metrics.parse_time_ms > 0) {
        result.metrics.parse_speed_loc_per_ms =
            (double)result.metrics.lines_of_code / result.metrics.parse_time_ms;
    }

    // Set error message if parsing failed
    if (!result.success) {
        if (parse_result.error_count > 0 && parse_result.errors && parse_result.errors[0]) {
            result.error_message = strdup(parse_result.errors[0]);
        } else {
            result.error_message = strdup("Unknown parsing error");
        }
    }

    return result;
}

bool run_test_suite(RealProgramTestSuite *suite, const TestSuiteConfig *config) {
    if (!suite || !config) {
        return false;
    }

    fprintf(config->output_stream, "=== Running Test Suite: %s ===\n", suite->suite_name);
    fprintf(config->output_stream, "Description: %s\n", suite->description);
    fprintf(config->output_stream, "Total tests: %zu\n\n", suite->test_count);

    suite->passed = 0;
    suite->failed = 0;
    suite->skipped = 0;
    suite->total_parse_time_ms = 0.0;

    uint64_t suite_start_time = get_timestamp_ms();

    for (size_t i = 0; i < suite->test_count; i++) {
        RealProgramTest *test = &suite->tests[i];

        fprintf(config->output_stream, "Running test %zu/%zu: %s\n", i + 1, suite->test_count,
                test->name);

        RealProgramTestResult result =
            validate_complete_program(test->source_code, test->name, config);

        suite->total_parse_time_ms += result.execution_time_ms;

        // Check if result matches expectations
        bool test_passed = false;

        if (test->should_parse) {
            test_passed = result.success;
            if (!test_passed && config->verbose_output) {
                fprintf(config->output_stream, "  Expected success but failed: %s\n",
                        result.error_message ? result.error_message : "Unknown error");
            }
        } else {
            test_passed = !result.success;
            if (test->expected_error_pattern && result.error_message) {
                // Check if error message contains expected pattern
                if (strstr(result.error_message, test->expected_error_pattern) == NULL) {
                    test_passed = false;
                    if (config->verbose_output) {
                        fprintf(config->output_stream,
                                "  Expected error pattern '%s' not found in '%s'\n",
                                test->expected_error_pattern, result.error_message);
                    }
                }
            }
        }

        // Check performance requirements
        if (test_passed && test->max_parse_time_ms > 0) {
            if (result.execution_time_ms > test->max_parse_time_ms) {
                test_passed = false;
                if (config->verbose_output) {
                    fprintf(config->output_stream,
                            "  Performance requirement failed: %.2fms > %zums\n",
                            result.execution_time_ms, test->max_parse_time_ms);
                }
            }
        }

        if (test_passed) {
            suite->passed++;
            fprintf(config->output_stream, "  ✅ PASS (%.2fms)\n", result.execution_time_ms);
        } else {
            suite->failed++;
            fprintf(config->output_stream, "  ❌ FAIL\n");
            if (config->stop_on_first_failure) {
                cleanup_test_result(&result);
                break;
            }
        }

        cleanup_test_result(&result);
    }

    uint64_t suite_end_time = get_timestamp_ms();
    double suite_execution_time = calculate_execution_time(suite_start_time, suite_end_time);

    if (suite->test_count > 0) {
        suite->average_parse_time_ms = suite->total_parse_time_ms / suite->test_count;
    }

    fprintf(config->output_stream, "\n=== Test Suite Results ===\n");
    fprintf(config->output_stream, "Tests passed: %zu\n", suite->passed);
    fprintf(config->output_stream, "Tests failed: %zu\n", suite->failed);
    fprintf(config->output_stream, "Tests skipped: %zu\n", suite->skipped);
    fprintf(config->output_stream, "Success rate: %.1f%%\n",
            (double)suite->passed / suite->test_count * 100.0);
    fprintf(config->output_stream, "Total parse time: %.2f ms\n", suite->total_parse_time_ms);
    fprintf(config->output_stream, "Average parse time: %.2f ms\n", suite->average_parse_time_ms);
    fprintf(config->output_stream, "Suite execution time: %.2f ms\n", suite_execution_time);
    fprintf(config->output_stream, "==========================\n\n");

    return suite->failed == 0;
}

void print_test_suite_results(const RealProgramTestSuite *suite, const TestSuiteConfig *config) {
    if (!suite || !config) {
        return;
    }

    FILE *out = config->output_stream;

    fprintf(out, "\n=== DETAILED TEST RESULTS ===\n");
    fprintf(out, "Suite: %s\n", suite->suite_name);
    fprintf(out, "Description: %s\n", suite->description);
    fprintf(out, "Total tests: %zu\n", suite->test_count);
    fprintf(out, "Passed: %zu\n", suite->passed);
    fprintf(out, "Failed: %zu\n", suite->failed);
    fprintf(out, "Skipped: %zu\n", suite->skipped);

    double success_rate = 0.0;
    if (suite->test_count > 0) {
        success_rate = (double)suite->passed / suite->test_count * 100.0;
    }

    fprintf(out, "Success rate: %.1f%%\n", success_rate);
    fprintf(out, "Average parse time: %.2f ms\n", suite->average_parse_time_ms);

    // Quality assessment
    if (success_rate >= 90.0) {
        fprintf(out, "Quality level: ✅ EXCELLENT\n");
    } else if (success_rate >= 80.0) {
        fprintf(out, "Quality level: ✅ GOOD\n");
    } else if (success_rate >= 60.0) {
        fprintf(out, "Quality level: ⚠️ FAIR\n");
    } else {
        fprintf(out, "Quality level: ❌ POOR\n");
    }

    fprintf(out, "=============================\n\n");
}