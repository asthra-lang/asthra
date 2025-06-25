/**
 * Test Suite Runner - Common Header
 *
 * Shared definitions and function declarations for test suite runner modules.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef TEST_SUITE_RUNNER_COMMON_H
#define TEST_SUITE_RUNNER_COMMON_H

#include "test_formatters.h"
#include "test_framework.h"
#include "test_suite_runner.h"
#include <dirent.h>
#include <fnmatch.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

// =============================================================================
// SHARED STRUCTURES
// =============================================================================

typedef struct {
    // Test selection
    char **test_patterns; // Patterns to match test names
    size_t pattern_count;
    char **exclude_patterns; // Patterns to exclude
    size_t exclude_pattern_count;
    char **suite_patterns; // Suite patterns to run
    size_t suite_pattern_count;

    // Execution options
    bool parallel_execution;   // Run tests in parallel
    size_t max_parallel_tests; // Max concurrent tests
    bool fail_fast;            // Stop on first failure
    bool randomize_order;      // Randomize test execution order
    bool repeat_tests;         // Repeat tests multiple times
    size_t repeat_count;       // Number of repetitions

    // Output options
    AsthraTestOutputFormat output_format;
    bool verbose;            // Verbose output
    bool quiet;              // Minimal output
    bool show_skipped;       // Show skipped tests
    bool show_duration;      // Show test durations
    bool colored_output;     // Use colored output
    const char *output_file; // Output file path

    // Debugging options
    bool debug_mode;     // Enable debug output
    bool profile_memory; // Profile memory usage
    bool dry_run;        // Don't actually run tests

    // Timeout settings
    uint64_t default_timeout_ns; // Default test timeout
    uint64_t suite_timeout_ns;   // Suite timeout

    // Discovery options
    bool auto_discover;      // Auto-discover test files
    char **test_directories; // Directories to search
    size_t directory_count;
} AsthraTestRunnerConfig;

typedef struct {
    AsthraTestFunction test_func;
    AsthraTestMetadata metadata;
    size_t test_index;
    bool selected;
} AsthraTestEntry;

typedef struct {
    const char *suite_name;
    AsthraTestEntry *tests;
    size_t test_count;
    size_t capacity;
    AsthraTestStatistics *stats;
} AsthraTestSuiteEntry;

struct AsthraTestRunner {
    AsthraTestSuiteEntry *suites;
    size_t suite_count;
    size_t capacity;
    AsthraTestRunnerConfig config;
    AsthraTestStatistics *global_stats;
    bool interrupted;
};

typedef struct {
    AsthraTestRunner *runner;
    AsthraTestSuiteEntry *suite;
    AsthraTestEntry *test;
    AsthraTestContext *context;
} TestThreadData;

// Global runner instance for signal handling
extern AsthraTestRunner *g_runner;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Signal handling
void signal_handler(int sig);
void setup_signal_handlers(void);

// Configuration management
AsthraTestRunnerConfig default_config(void);
void print_usage(const char *program_name);
AsthraTestOutputFormat parse_output_format(const char *format_str);
int parse_command_line(int argc, char **argv, AsthraTestRunnerConfig *config);

// Test discovery and filtering
bool matches_pattern(const char *name, char **patterns, size_t pattern_count);
bool is_excluded(const char *name, char **exclude_patterns, size_t exclude_count);
void filter_tests(AsthraTestRunner *runner);

// Test execution
void *run_test_thread(void *arg);
AsthraTestResult run_single_test(AsthraTestRunner *runner, AsthraTestSuiteEntry *suite,
                                 AsthraTestEntry *test);
int run_test_suite(AsthraTestRunner *runner, AsthraTestSuiteEntry *suite);

#endif // TEST_SUITE_RUNNER_COMMON_H