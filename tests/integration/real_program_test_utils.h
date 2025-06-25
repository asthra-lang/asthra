/**
 * Real Program Test Utilities Header
 *
 * Contains utility functions for timestamps, configuration, and common
 * helper functions for the real program test framework.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_UTILS_H
#define REAL_PROGRAM_TEST_UTILS_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

// =============================================================================
// TEST CONFIGURATION
// =============================================================================

typedef struct {
    bool verbose_output;
    bool stop_on_first_failure;
    bool measure_performance;
    bool detect_false_positives;
    size_t max_execution_time_ms;
    size_t max_memory_usage_kb;
    const char *output_format;
    FILE *output_stream;
    bool enable_memory_debugging;
} TestSuiteConfig;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get current timestamp in milliseconds
 */
uint64_t get_timestamp_ms(void);

/**
 * Calculate execution time between two timestamps
 */
double calculate_execution_time(uint64_t start_time, uint64_t end_time);

/**
 * Create default test suite configuration
 */
TestSuiteConfig create_default_config(void);

#endif // REAL_PROGRAM_TEST_UTILS_H