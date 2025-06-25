/**
 * Asthra Programming Language
 * Gperf Integration Test Suite
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for enhanced Gperf integration and keyword hash generation
 */

#include "test_optimization_common.h"

// Forward declarations
static AsthraTestResult test_gperf_makefile_targets(AsthraTestContext *context);
static AsthraTestResult test_gperf_keyword_extraction(AsthraTestContext *context);
static AsthraTestResult test_gperf_hash_function_quality(AsthraTestContext *context);

static AsthraTestResult test_gperf_makefile_targets(AsthraTestContext *context) {
    // Test gperf-keywords target
    CommandResult result = execute_command("make gperf-keywords", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "gperf-keywords target should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify generated files exist
    const char *expected_gperf_files[] = {"optimization/asthra_keywords_hash.c",
                                          "optimization/asthra_keywords_hash.h",
                                          "optimization/asthra_keywords.gperf"};

    for (size_t i = 0; i < sizeof(expected_gperf_files) / sizeof(expected_gperf_files[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, file_exists(expected_gperf_files[i]),
                                "Gperf file should exist: %s", expected_gperf_files[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    // Test gperf-test target
    result = execute_command("make gperf-test", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0, "gperf-test target should succeed: %s",
                               result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_gperf_keyword_extraction(AsthraTestContext *context) {
    // Test the keyword extraction script
    if (!file_exists("optimization/extract_keywords_main.sh")) {
        return ASTHRA_TEST_SKIP;
    }

    CommandResult result =
        execute_command("bash optimization/extract_keywords_main.sh", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Keyword extraction script should succeed: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify the script generated valid keyword files
    if (!ASTHRA_TEST_ASSERT(context, file_exists("optimization/asthra_keywords.gperf"),
                            "Keyword extraction should generate gperf input file")) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that the gperf file contains keywords
    FILE *gperf_file = fopen("optimization/asthra_keywords.gperf", "r");
    if (gperf_file) {
        char line[256];
        bool found_keywords = false;
        while (fgets(line, sizeof(line), gperf_file)) {
            if (strstr(line, "fn") || strstr(line, "let") || strstr(line, "if")) {
                found_keywords = true;
                break;
            }
        }
        fclose(gperf_file);

        if (!ASTHRA_TEST_ASSERT(context, found_keywords,
                                "Gperf file should contain Asthra keywords")) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_gperf_hash_function_quality(AsthraTestContext *context) {
    if (!file_exists("optimization/asthra_keywords_hash.c")) {
        return ASTHRA_TEST_SKIP;
    }

    // Test the generated hash function by compiling and running validation
    CommandResult result = execute_command("make optimization/test_keywords", TEST_TIMEOUT_SECONDS);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Gperf hash function compilation should succeed: %s",
                               result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    // Run the keyword validation test
    result = execute_command("optimization/test_keywords", 30);
    if (!ASTHRA_TEST_ASSERT_EQ(context, result.exit_code, 0,
                               "Gperf hash function validation should pass: %s", result.error)) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Export test functions and metadata for the main test runner
AsthraTestFunction gperf_test_functions[] = {
    test_gperf_makefile_targets, test_gperf_keyword_extraction, test_gperf_hash_function_quality};

AsthraTestMetadata gperf_test_metadata[] = {
    {"Gperf Makefile Targets", __FILE__, __LINE__, "test_gperf_makefile_targets",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Gperf Keyword Extraction", __FILE__, __LINE__, "test_gperf_keyword_extraction",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
    {"Gperf Hash Function Quality", __FILE__, __LINE__, "test_gperf_hash_function_quality",
     ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL}};

const size_t gperf_test_count = sizeof(gperf_test_functions) / sizeof(gperf_test_functions[0]);
