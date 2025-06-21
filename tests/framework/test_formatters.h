/**
 * Asthra Programming Language - Enhanced Test Formatters
 * Header file for comprehensive test output formatting
 *
 * Phase 4: Test Framework Enhancement
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_TEST_FORMATTERS_H
#define ASTHRA_TEST_FORMATTERS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include "test_framework.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OUTPUT FORMAT TYPES
// =============================================================================

typedef enum {
    ASTHRA_FORMAT_CONSOLE,      // Colored console output
    ASTHRA_FORMAT_PLAIN,        // Plain text output
    ASTHRA_FORMAT_JSON,         // JSON structured output
    ASTHRA_FORMAT_XML,          // XML structured output
    ASTHRA_FORMAT_TAP,          // Test Anything Protocol
    ASTHRA_FORMAT_JUNIT,        // JUnit XML format
    ASTHRA_FORMAT_MARKDOWN      // Markdown format
} AsthraTestOutputFormat;

// =============================================================================
// FORMATTER CONFIGURATION
// =============================================================================

/**
 * Configure the test formatter with output format and options
 */
void asthra_test_formatter_set_config(AsthraTestOutputFormat format, 
                                     bool use_colors, 
                                     const char* output_file);

/**
 * Enable or disable verbose output
 */
void asthra_test_formatter_set_verbose(bool verbose);

/**
 * Enable or disable timing information
 */
void asthra_test_formatter_set_timing(bool show_timing);

/**
 * Cleanup formatter resources
 */
void asthra_test_formatter_cleanup(void);

// =============================================================================
// TEST RESULT FORMATTING
// =============================================================================

/**
 * Format a single test result
 */
void format_test_result(const AsthraTestContext* context);

/**
 * Format test suite header
 */
void format_suite_header(const char* suite_name, size_t total_tests);

/**
 * Format test suite summary
 */
void format_suite_summary(const AsthraTestStatistics* stats);

/**
 * Format multiple test results as JSON array
 */
void format_json_test_results(const AsthraTestContext* contexts, size_t count);

// =============================================================================
// LEGACY COMPATIBILITY
// =============================================================================

/**
 * Legacy simple test result formatter
 */
void format_test_result_legacy(const char *name, int passed);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_FORMATTERS_H