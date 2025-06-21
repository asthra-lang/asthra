/**
 * Asthra Programming Language
 * Makefile Integration Test Suite
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Tests for Makefile targets and build system integration
 */

#include "test_optimization_common.h"

// Forward declarations
static AsthraTestResult test_makefile_help_updated(AsthraTestContext *context);
static AsthraTestResult test_makefile_optimize_analyze_target(AsthraTestContext *context);
static AsthraTestResult test_makefile_legacy_deprecation(AsthraTestContext *context);

static AsthraTestResult test_makefile_help_updated(AsthraTestContext *context) {
    CommandResult result = execute_command("make help", 30);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, 
                               "Make help should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Check for new optimization targets in help output
    const char *expected_targets[] = {
        "pgo-optimize",
        "sanitizer-comprehensive", 
        "gperf-keywords",
        "optimize-analyze"
    };
    
    for (size_t i = 0; i < sizeof(expected_targets) / sizeof(expected_targets[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, strstr(result.output, expected_targets[i]) != NULL, 
                               "Help should mention target: %s", expected_targets[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_makefile_optimize_analyze_target(AsthraTestContext *context) {
    // Test the main optimize-analyze target that runs the complete modern workflow
    CommandResult result = execute_command("make optimize-analyze", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, 
                               "optimize-analyze target should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify that the target produced expected artifacts
    const char *expected_artifacts[] = {
        "bin/asthra-pgo-optimized",
        "optimization/asthra_keywords_hash.c"
    };
    
    for (size_t i = 0; i < sizeof(expected_artifacts) / sizeof(expected_artifacts[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, file_exists(expected_artifacts[i]), 
                               "optimize-analyze should create: %s", expected_artifacts[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }
    
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_makefile_legacy_deprecation(AsthraTestContext *context) {
    // Test that legacy targets still work but show deprecation warnings
    CommandResult result = execute_command("make profile 2>&1", 60);
    
    // Legacy target should still work
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, 
                               "Legacy profile target should still work: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Should show deprecation warning
    if (!ASTHRA_TEST_ASSERT(context, 
                           strstr(result.output, "deprecated") != NULL || 
                           strstr(result.error, "deprecated") != NULL, 
                           "Legacy target should show deprecation warning")) {
        printf("Output: %s\nError: %s\n", result.output, result.error);
    }
    
    return ASTHRA_TEST_PASS;
}

// Export test functions and metadata for the main test runner
AsthraTestFunction makefile_test_functions[] = {
    test_makefile_help_updated,
    test_makefile_optimize_analyze_target,
    test_makefile_legacy_deprecation
};

AsthraTestMetadata makefile_test_metadata[] = {
    {"Makefile Help Updated", __FILE__, __LINE__, "test_makefile_help_updated", ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL},
    {"Makefile Optimize-Analyze Target", __FILE__, __LINE__, "test_makefile_optimize_analyze_target", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
    {"Makefile Legacy Deprecation", __FILE__, __LINE__, "test_makefile_legacy_deprecation", ASTHRA_TEST_SEVERITY_LOW, 0, false, NULL}
};

const size_t makefile_test_count = sizeof(makefile_test_functions) / sizeof(makefile_test_functions[0]); 
