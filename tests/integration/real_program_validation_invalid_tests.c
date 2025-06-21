/**
 * Real Program Validation Tests - Invalid Program Test Cases
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_validation_common.h"

// =============================================================================
// INVALID PROGRAM TEST CASES
// =============================================================================

// Test 1: Type Mismatch (should fail with specific error)
static const char* TYPE_MISMATCH_PROGRAM = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let x: int = \"not a number\";\n"
    "}";

// Test 2: Invalid Syntax (should fail with parse error)
static const char* SYNTAX_ERROR_PROGRAM = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let x: int = 42\n"  // Missing semicolon - syntax error
    "}";

// Test 3: Missing Package Declaration
static const char* MISSING_PACKAGE_PROGRAM = 
    "pub fn main(none) -> void {\n"
    "    let x: int = 42;\n"
    "}";

// Test 4: Invalid Type Annotation
static const char* INVALID_TYPE_PROGRAM = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let arr: [] = [1, 2, 3];\n"  // Missing type in array type - parse error
    "}";

// Test 5: Unclosed Block
static const char* UNCLOSED_BLOCK_PROGRAM = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let x: int = 42;\n"
    "    if x > 0 {\n"
    "        let y: int = 1;\n"
    // Missing closing brace
    "}";

// Test 6: Invalid Function Call
static const char* INVALID_FUNCTION_CALL_PROGRAM = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let result: int = undefined_function(42);\n"
    "}";

// Test 7: Array Index Out of Bounds
static const char* ARRAY_BOUNDS_PROGRAM = 
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let arr: []int = [1, 2, 3];\n"
    "    let value: int = arr[10];\n"  // This should potentially be caught
    "}";

// =============================================================================
// INVALID PROGRAM TEST SUITE CREATION
// =============================================================================

/**
 * Create test suite for invalid programs that should fail with specific errors
 */
RealProgramTestSuite* create_invalid_program_test_suite(void) {
    RealProgramTestSuite* suite = create_real_program_test_suite(
        "Invalid Program Tests", 
        "Tests that validate error detection for invalid programs");
    
    if (!suite) {
        return NULL;
    }
    
    // Add all invalid program tests
    RealProgramTest tests[] = {
        {
            .name = "type_mismatch_error",
            .source_code = TYPE_MISMATCH_PROGRAM,
            .should_parse = true,  // This is valid syntax, should fail in semantic analysis
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = "type",
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        },
        {
            .name = "syntax_error",
            .source_code = SYNTAX_ERROR_PROGRAM,
            .should_parse = false,
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = "syntax",
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        },
        {
            .name = "missing_package_error",
            .source_code = MISSING_PACKAGE_PROGRAM,
            .should_parse = false,
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = "package",
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        },
        {
            .name = "invalid_type_error",
            .source_code = INVALID_TYPE_PROGRAM,
            .should_parse = true,  // This is valid syntax, should fail in semantic analysis
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = "type",
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        },
        {
            .name = "unclosed_block_error",
            .source_code = UNCLOSED_BLOCK_PROGRAM,
            .should_parse = false,
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = "block",
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        },
        {
            .name = "invalid_function_call_error",
            .source_code = INVALID_FUNCTION_CALL_PROGRAM,
            .should_parse = true,  // This is valid syntax, should fail in semantic analysis
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = "function",
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        },
        {
            .name = "array_bounds_warning",
            .source_code = ARRAY_BOUNDS_PROGRAM,
            .should_parse = true,  // This might parse but should warn
            .should_compile = false,
            .should_execute = false,
            .expected_error_pattern = NULL,
            .max_parse_time_ms = 1000,
            .max_total_time_ms = 2000
        }
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    for (size_t i = 0; i < test_count; i++) {
        if (!add_test_to_suite(suite, &tests[i])) {
            fprintf(stderr, "Failed to add test %s to suite\n", tests[i].name);
            cleanup_test_suite(suite);
            return NULL;
        }
    }
    
    return suite;
}