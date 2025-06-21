/**
 * Test Suite Runner - Main Interface
 * 
 * Main runner interface and management functions.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_suite_runner_common.h"

// =============================================================================
// MAIN RUNNER INTERFACE
// =============================================================================

AsthraTestRunner* asthra_test_runner_create(void) {
    AsthraTestRunner* runner = calloc(1, sizeof(AsthraTestRunner));
    if (!runner) return NULL;
    
    runner->config = default_config();
    runner->global_stats = asthra_test_statistics_create();
    if (!runner->global_stats) {
        free(runner);
        return NULL;
    }
    
    return runner;
}

void asthra_test_runner_destroy(AsthraTestRunner* runner) {
    if (!runner) return;
    
    // Cleanup suites
    for (size_t i = 0; i < runner->suite_count; i++) {
        AsthraTestSuiteEntry* suite = &runner->suites[i];
        if (suite->stats) {
            asthra_test_statistics_destroy(suite->stats);
        }
        free(suite->tests);
    }
    free(runner->suites);
    
    // Cleanup configuration
    for (size_t i = 0; i < runner->config.pattern_count; i++) {
        free(runner->config.test_patterns[i]);
    }
    free(runner->config.test_patterns);
    
    for (size_t i = 0; i < runner->config.exclude_pattern_count; i++) {
        free(runner->config.exclude_patterns[i]);
    }
    free(runner->config.exclude_patterns);
    
    for (size_t i = 0; i < runner->config.suite_pattern_count; i++) {
        free(runner->config.suite_patterns[i]);
    }
    free(runner->config.suite_patterns);
    
    for (size_t i = 0; i < runner->config.directory_count; i++) {
        free(runner->config.test_directories[i]);
    }
    free(runner->config.test_directories);
    
    if (runner->config.output_file) {
        free((void*)runner->config.output_file);
    }
    
    if (runner->global_stats) {
        asthra_test_statistics_destroy(runner->global_stats);
    }
    
    free(runner);
}

int asthra_test_runner_add_suite(AsthraTestRunner* runner, 
                                const char* suite_name,
                                AsthraTestFunction* tests,
                                const AsthraTestMetadata* metadata_array,
                                size_t test_count) {
    if (!runner || !suite_name || !tests || !metadata_array) {
        return -1;
    }
    
    // Expand suites array if needed
    if (runner->suite_count >= runner->capacity) {
        size_t new_capacity = runner->capacity == 0 ? 4 : runner->capacity * 2;
        AsthraTestSuiteEntry* new_suites = realloc(runner->suites, 
            new_capacity * sizeof(AsthraTestSuiteEntry));
        if (!new_suites) return -1;
        
        runner->suites = new_suites;
        runner->capacity = new_capacity;
    }
    
    AsthraTestSuiteEntry* suite = &runner->suites[runner->suite_count];
    suite->suite_name = strdup(suite_name);
    suite->test_count = test_count;
    suite->capacity = test_count;
    suite->tests = calloc(test_count, sizeof(AsthraTestEntry));
    suite->stats = asthra_test_statistics_create();
    
    if (!suite->tests || !suite->stats) {
        return -1;
    }
    
    // Copy test functions and metadata
    for (size_t i = 0; i < test_count; i++) {
        suite->tests[i].test_func = tests[i];
        suite->tests[i].metadata = metadata_array[i];
        suite->tests[i].test_index = i;
        suite->tests[i].selected = true; // Will be filtered later
    }
    
    runner->suite_count++;
    return 0;
}

int asthra_test_runner_execute(AsthraTestRunner* runner) {
    if (!runner) return -1;
    
    // Setup signal handling
    g_runner = runner;
    setup_signal_handlers();
    
    // Configure formatter
    asthra_test_formatter_set_config(runner->config.output_format, 
                                    runner->config.colored_output,
                                    runner->config.output_file);
    asthra_test_formatter_set_verbose(runner->config.verbose);
    asthra_test_formatter_set_timing(runner->config.show_duration);
    
    // Filter tests based on patterns
    filter_tests(runner);
    
    // Execute test suites
    int overall_result = 0;
    for (size_t i = 0; i < runner->suite_count; i++) {
        if (runner->interrupted) {
            break;
        }
        
        int suite_result = run_test_suite(runner, &runner->suites[i]);
        if (suite_result != 0) {
            overall_result = suite_result;
            if (runner->config.fail_fast) {
                break;
            }
        }
    }
    
    // Print final summary
    if (!runner->config.quiet) {
        format_suite_summary(runner->global_stats);
    }
    
    // Cleanup formatter
    asthra_test_formatter_cleanup();
    
    g_runner = NULL;
    return overall_result;
}