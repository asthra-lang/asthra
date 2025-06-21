/**
 * Real Program Validation Tests - Performance Test Cases
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_validation_common.h"

// =============================================================================
// STRESS TEST TEMPLATES
// =============================================================================

// Test 1: Large Program with Many Variables
static const char* LARGE_VARIABLE_PROGRAM_TEMPLATE = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n";

// Test 2: Deeply Nested Control Flow
static const char* DEEP_NESTING_TEMPLATE = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n";

// =============================================================================
// PERFORMANCE TEST SUITE CREATION
// =============================================================================

/**
 * Create test suite for performance and stress testing
 */
RealProgramTestSuite* create_performance_test_suite(void) {
    RealProgramTestSuite* suite = create_real_program_test_suite(
        "Performance Test Suite", 
        "Tests that validate parser performance with large and complex programs");
    
    if (!suite) {
        return NULL;
    }
    
    // Generate performance test programs
    char* large_program = generate_large_program(100);
    char* complex_type_program = generate_complex_type_program();
    char* deep_nesting_program = generate_deep_nesting_program(10);
    
    if (!large_program || !complex_type_program || !deep_nesting_program) {
        fprintf(stderr, "Failed to generate performance test programs\n");
        cleanup_test_suite(suite);
        free(large_program);
        free(complex_type_program);
        free(deep_nesting_program);
        return NULL;
    }
    
    RealProgramTest tests[] = {
        {
            .name = "large_program_performance",
            .source_code = large_program,
            .should_parse = true,
            .should_compile = true,
            .should_execute = true,
            .expected_error_pattern = NULL,
            .max_parse_time_ms = 5000,  // Allow longer for large programs
            .max_total_time_ms = 10000
        },
        {
            .name = "complex_types_performance",
            .source_code = complex_type_program,
            .should_parse = true,
            .should_compile = true,
            .should_execute = true,
            .expected_error_pattern = NULL,
            .max_parse_time_ms = 2000,
            .max_total_time_ms = 4000
        },
        {
            .name = "deep_nesting_performance",
            .source_code = deep_nesting_program,
            .should_parse = true,
            .should_compile = true,
            .should_execute = true,
            .expected_error_pattern = NULL,
            .max_parse_time_ms = 3000,
            .max_total_time_ms = 6000
        }
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < test_count; i++) {
        if (!add_test_to_suite(suite, &tests[i])) {
            fprintf(stderr, "Failed to add test %s to suite\n", tests[i].name);
            cleanup_test_suite(suite);
            // Note: Memory for generated programs will be freed when suite is cleaned up
            return NULL;
        }
    }
    
    return suite;
}