/**
 * Asthra Programming Language
 * Automation Scripts Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for optimization automation scripts and utilities
 */

#include "test_optimization_common.h"

// Forward declarations
static AsthraTestResult test_instruments_profiler_script(AsthraTestContext *context);
static AsthraTestResult test_sanitizer_runner_script(AsthraTestContext *context);
static AsthraTestResult test_optimization_script_integration(AsthraTestContext *context);

static AsthraTestResult test_instruments_profiler_script(AsthraTestContext *context) {
    if (!file_exists("optimization/instruments-profiler.sh")) {
        return ASTHRA_TEST_SKIP;
    }

    // Test script availability check
    CommandResult result = execute_command("bash optimization/instruments-profiler.sh check", 30);
    // This should succeed regardless of whether Instruments is available
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Instruments profiler check should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test help output
    result = execute_command("bash optimization/instruments-profiler.sh help", 30);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Instruments profiler help should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, strstr(result.output, "Usage:") != NULL,
                            "Help output should contain usage information")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_sanitizer_runner_script(AsthraTestContext *context) {
    if (!file_exists("optimization/sanitizer-runner.sh")) {
        return ASTHRA_TEST_SKIP;
    }

    // Test script help
    CommandResult result = execute_command("bash optimization/sanitizer-runner.sh --help", 30);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Sanitizer runner help should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test dry run mode
    result = execute_command("bash optimization/sanitizer-runner.sh --dry-run", 60);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Sanitizer runner dry run should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_optimization_script_integration(AsthraTestContext *context) {
    if (!file_exists("optimization/optimize.sh")) {
        return ASTHRA_TEST_SKIP;
    }

    // Test the main optimization script
    CommandResult result = execute_command("bash optimization/optimize.sh --help", 30);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Optimization script help should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test validation mode
    result = execute_command("bash optimization/optimize.sh --validate", 60);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Optimization script validation should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Export test functions and metadata for the main test runner
AsthraTestFunction automation_test_functions[] = {test_instruments_profiler_script,
                                                  test_sanitizer_runner_script,
                                                  test_optimization_script_integration};

AsthraTestMetadata automation_test_metadata[] = {
    {"Instruments Profiler Script", __FILE__, __LINE__, "test_instruments_profiler_script",
     ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL},
    {"Sanitizer Runner Script", __FILE__, __LINE__, "test_sanitizer_runner_script",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Optimization Script Integration", __FILE__, __LINE__, "test_optimization_script_integration",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}};

const size_t automation_test_count =
    sizeof(automation_test_functions) / sizeof(automation_test_functions[0]);
