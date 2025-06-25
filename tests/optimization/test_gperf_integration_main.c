/**
 * Asthra Programming Language
 * Gperf Integration Test Suite - Main Runner
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This test suite validates the enhanced Gperf integration for
 * perfect hash function generation for Asthra language keywords.
 */

#include "test_gperf_common.h"

// Import test modules
extern AsthraTestFunction gperf_file_generation_tests[];
extern AsthraTestMetadata gperf_file_generation_metadata[];
extern const size_t gperf_file_generation_test_count;

extern AsthraTestFunction gperf_keyword_extraction_tests[];
extern AsthraTestMetadata gperf_keyword_extraction_metadata[];
extern const size_t gperf_keyword_extraction_test_count;

extern AsthraTestFunction gperf_hash_correctness_tests[];
extern AsthraTestMetadata gperf_hash_correctness_metadata[];
extern const size_t gperf_hash_correctness_test_count;

extern AsthraTestFunction gperf_performance_tests[];
extern AsthraTestMetadata gperf_performance_metadata[];
extern const size_t gperf_performance_test_count;

// Integration tests
static AsthraTestResult test_gperf_makefile_integration(AsthraTestContext *context) {
    // Test that gperf integration works with the Makefile

    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command), "make gperf-keywords 2>/dev/null");

    int exit_code = run_command_with_timeout(command, 60.0);
    if (!ASTHRA_TEST_ASSERT_EQ(context, exit_code, 0,
                               "Makefile gperf-keywords target should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify that all expected files were generated/updated
    const char *expected_files[] = {"optimization/asthra_keywords.gperf",
                                    "optimization/asthra_keywords_hash.c",
                                    "optimization/asthra_keywords_hash.h"};

    for (size_t i = 0; i < sizeof(expected_files) / sizeof(expected_files[0]); i++) {
        if (!ASTHRA_TEST_ASSERT(context, gperf_file_exists(expected_files[i]),
                                "Makefile should generate/update: %s", expected_files[i])) {
            return ASTHRA_TEST_FAIL;
        }
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_gperf_dependency_tracking(AsthraTestContext *context) {
    // Test that gperf files are regenerated when source changes

    const char *gperf_file = "optimization/asthra_keywords.gperf";
    const char *hash_file = "optimization/asthra_keywords_hash.c";

    struct stat gperf_stat, hash_stat;

    if (stat(gperf_file, &gperf_stat) != 0 || stat(hash_file, &hash_stat) != 0) {
        return ASTHRA_TEST_SKIP;
    }

    // Hash file should be newer than or equal to gperf file
    if (!ASTHRA_TEST_ASSERT(context, hash_stat.st_mtime >= gperf_stat.st_mtime,
                            "Generated hash file should be up-to-date with gperf input")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_gperf_build_system_integration(AsthraTestContext *context) {
    // Test that gperf-generated files integrate properly with the build system

    char command[MAX_COMMAND_LENGTH];
    snprintf(command, sizeof(command),
             "make clean >/dev/null 2>&1 && make gperf-keywords >/dev/null 2>&1 && make >/dev/null "
             "2>&1");

    int exit_code = run_command_with_timeout(command, 120.0);
    if (!ASTHRA_TEST_ASSERT_EQ(context, exit_code, 0,
                               "Full build with gperf integration should succeed")) {
        return ASTHRA_TEST_FAIL;
    }

    // Check that the main binary was built successfully
    if (!ASTHRA_TEST_ASSERT(context, gperf_file_exists("bin/asthra"),
                            "Main binary should be built with gperf integration")) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Helper function to run a test suite
static bool run_test_suite(const char *suite_name, AsthraTestFunction *tests,
                           AsthraTestMetadata *metadata, size_t test_count,
                           AsthraTestSuiteConfig *config) {
    printf("\n--- Running %s Tests ---\n", suite_name);

    AsthraTestResult suite_result = asthra_test_run_suite(tests, metadata, test_count, config);

    printf("%s Tests: %s\n", suite_name,
           (suite_result == ASTHRA_TEST_PASS)   ? "PASSED"
           : (suite_result == ASTHRA_TEST_SKIP) ? "SKIPPED"
                                                : "FAILED");

    return suite_result == ASTHRA_TEST_PASS;
}

int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return EXIT_FAILURE;
    }

    // Configure test suite
    AsthraTestSuiteConfig config = asthra_test_suite_config_default();
    config.name = "Gperf Integration Test Suite";
    config.description = "Validates enhanced Gperf integration for keyword hash generation";
    config.verbose_output = true;
    config.stop_on_failure = false;
    config.default_timeout_ns = 60 * 1000000000ULL; // 60 seconds
    config.statistics = stats;

    printf("=============================================================================\n");
    printf("Asthra Gperf Integration Test Suite\n");
    printf("Testing: Keyword extraction, hash generation, and performance\n");
    printf("=============================================================================\n");

    bool all_passed = true;

    // Run all test suites
    all_passed &=
        run_test_suite("File Generation", gperf_file_generation_tests,
                       gperf_file_generation_metadata, gperf_file_generation_test_count, &config);

    all_passed &= run_test_suite("Keyword Extraction", gperf_keyword_extraction_tests,
                                 gperf_keyword_extraction_metadata,
                                 gperf_keyword_extraction_test_count, &config);

    all_passed &=
        run_test_suite("Hash Correctness", gperf_hash_correctness_tests,
                       gperf_hash_correctness_metadata, gperf_hash_correctness_test_count, &config);

    all_passed &= run_test_suite("Performance", gperf_performance_tests, gperf_performance_metadata,
                                 gperf_performance_test_count, &config);

    // Run integration tests
    AsthraTestFunction integration_tests[] = {test_gperf_makefile_integration,
                                              test_gperf_dependency_tracking,
                                              test_gperf_build_system_integration};

    AsthraTestMetadata integration_metadata[] = {
        {"Gperf Makefile Integration", __FILE__, __LINE__, "test_gperf_makefile_integration",
         ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL},
        {"Gperf Dependency Tracking", __FILE__, __LINE__, "test_gperf_dependency_tracking",
         ASTHRA_TEST_SEVERITY_MEDIUM, 0, false, NULL},
        {"Gperf Build System Integration", __FILE__, __LINE__,
         "test_gperf_build_system_integration", ASTHRA_TEST_SEVERITY_HIGH, 0, false, NULL}};

    size_t integration_test_count = sizeof(integration_tests) / sizeof(integration_tests[0]);

    all_passed &= run_test_suite("Integration", integration_tests, integration_metadata,
                                 integration_test_count, &config);

    // Print final results
    printf("\n=============================================================================\n");
    printf("Gperf Integration Test Results\n");
    printf("=============================================================================\n");
    asthra_test_statistics_print(stats, false);

    printf("\nOverall Result: %s\n", all_passed ? "PASSED" : "FAILED");

    // Cleanup
    asthra_test_statistics_destroy(stats);

    return all_passed ? EXIT_SUCCESS : EXIT_FAILURE;
}
