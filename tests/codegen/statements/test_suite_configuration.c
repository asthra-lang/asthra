/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Suite Configuration
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test suite configuration utilities for statement generation tests
 */

#include "expression_oriented_test_utils.h"
#include "statement_generation_test_fixtures.h"

// =============================================================================
// TEST SUITE CONFIGURATION
// =============================================================================

/**
 * Create standard test suite configuration for statement generation tests
 */
AsthraTestSuiteConfig create_statement_test_suite_config(const char *suite_name,
                                                         const char *description,
                                                         AsthraTestStatistics *stats) {
    AsthraTestSuiteConfig config = {.name = suite_name,
                                    .description = description,
                                    .parallel_execution = false,
                                    .stop_on_failure = false,
                                    .verbose_output = true,
                                    .json_output = false,
                                    .default_timeout_ns = 10000000000ULL, // 10 seconds
                                    .max_parallel_tests = 1,
                                    .statistics = stats};

    return config;
}

/**
 * Standard test metadata template for statement generation tests
 */
AsthraTestMetadata create_statement_test_metadata(const char *test_name,
                                                  AsthraTestSeverity severity,
                                                  uint64_t timeout_ns) {
    AsthraTestMetadata metadata = {.name = test_name,
                                   .file = __FILE__,
                                   .line = __LINE__,
                                   .function = test_name,
                                   .severity = severity,
                                   .timeout_ns = timeout_ns,
                                   .skip = false,
                                   .skip_reason = NULL};

    return metadata;
}