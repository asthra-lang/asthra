/**
 * Test Suite Runner - Test Discovery and Filtering
 * 
 * Test filtering and discovery functionality for test suite runner.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_suite_runner_common.h"

// =============================================================================
// TEST DISCOVERY AND FILTERING
// =============================================================================

bool matches_pattern(const char* name, char** patterns, size_t pattern_count) {
    if (pattern_count == 0) return true;
    
    for (size_t i = 0; i < pattern_count; i++) {
        if (fnmatch(patterns[i], name, 0) == 0) {
            return true;
        }
    }
    return false;
}

bool is_excluded(const char* name, char** exclude_patterns, size_t exclude_count) {
    return matches_pattern(name, exclude_patterns, exclude_count);
}

void filter_tests(AsthraTestRunner* runner) {
    for (size_t s = 0; s < runner->suite_count; s++) {
        AsthraTestSuiteEntry* suite = &runner->suites[s];
        
        // Check if suite matches suite patterns
        if (!matches_pattern(suite->suite_name, 
                            runner->config.suite_patterns, 
                            runner->config.suite_pattern_count)) {
            // Mark all tests in this suite as not selected
            for (size_t t = 0; t < suite->test_count; t++) {
                suite->tests[t].selected = false;
            }
            continue;
        }
        
        for (size_t t = 0; t < suite->test_count; t++) {
            AsthraTestEntry* test = &suite->tests[t];
            
            // Check test name patterns
            bool matches = matches_pattern(test->metadata.name,
                                         runner->config.test_patterns,
                                         runner->config.pattern_count);
            
            // Check exclude patterns
            bool excluded = is_excluded(test->metadata.name,
                                      runner->config.exclude_patterns,
                                      runner->config.exclude_pattern_count);
            
            test->selected = matches && !excluded;
        }
    }
}