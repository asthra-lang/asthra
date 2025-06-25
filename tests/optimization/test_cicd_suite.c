/**
 * Asthra Programming Language
 * CI/CD Compatibility Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for CI/CD integration and non-interactive environment compatibility
 */

#include "test_optimization_common.h"

// Forward declarations
static AsthraTestResult test_ci_cd_compatibility(AsthraTestContext *context);
static AsthraTestResult test_optimization_test_suite_target(AsthraTestContext *context);

static AsthraTestResult test_ci_cd_compatibility(AsthraTestContext *context) {
    // Test that optimization targets work in non-interactive environments

    // Set environment variables for non-interactive mode
    setenv("CI", "true", 1);
    setenv("TERM", "dumb", 1);

    CommandResult result = execute_command("make sanitizer-asan", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Sanitizer should work in CI environment: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test PGO in CI mode
    result = execute_command("make pgo-generate", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "PGO should work in CI environment: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Clean up environment
    unsetenv("CI");
    unsetenv("TERM");

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_optimization_test_suite_target(AsthraTestContext *context) {
    // Test the new test-optimization target
    CommandResult result = execute_command("make test-optimization", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "test-optimization target should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Export test functions and metadata for the main test runner
AsthraTestFunction cicd_test_functions[] = {test_ci_cd_compatibility,
                                            test_optimization_test_suite_target};

AsthraTestMetadata cicd_test_metadata[] = {
    {"CI/CD Compatibility", __FILE__, __LINE__, "test_ci_cd_compatibility",
     ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Optimization Test Suite Target", __FILE__, __LINE__, "test_optimization_test_suite_target",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}};

const size_t cicd_test_count = sizeof(cicd_test_functions) / sizeof(cicd_test_functions[0]);
