/**
 * Asthra Programming Language v1.2 Test Reporting
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test reporting and AI feedback generation
 */

#ifndef ASTHRA_TEST_REPORTING_H
#define ASTHRA_TEST_REPORTING_H

#include "test_registry.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// REPORTING FUNCTIONS
// =============================================================================

/**
 * Generate AI feedback report in Markdown format
 * @param summary Test execution summary
 * @param output_file Path to output file
 */
void generate_ai_feedback_report(const TestSummary *summary, const char *output_file);

/**
 * Generate JSON test results report
 * @param summary Test execution summary
 * @param output_file Path to output file
 */
void generate_json_report(const TestSummary *summary, const char *output_file);

/**
 * Print help information for command line options
 * @param program_name Name of the program (argv[0])
 */
void print_help(const char *program_name);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_REPORTING_H
