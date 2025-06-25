/**
 * Real Program Validation Tests - Valid Program Test Cases
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_validation_common.h"

// =============================================================================
// VALID PROGRAM TEST CASES
// =============================================================================

// Test 1: Minimal Valid Program
static const char *MINIMAL_PROGRAM = "package main;\n"
                                     "\n"
                                     "pub fn main(none) -> void {\n"
                                     "    let x: int = 42;\n"
                                     "}";

// Test 2: Basic Type Usage
static const char *TYPE_USAGE_PROGRAM = "package main;\n"
                                        "\n"
                                        "pub fn main(none) -> void {\n"
                                        "    let number: int = 42;\n"
                                        "    let text: string = \"hello\";\n"
                                        "    let flag: bool = true;\n"
                                        "}";

// Test 3: Function with Parameters
static const char *FUNCTION_WITH_PARAMS_PROGRAM = "package main;\n"
                                                  "\n"
                                                  "priv fn add(a: int, b: int) -> int {\n"
                                                  "    return a + b;\n"
                                                  "}\n"
                                                  "\n"
                                                  "pub fn main(none) -> void {\n"
                                                  "    let result: int = add(5, 3);\n"
                                                  "}";

// Test 4: Control Flow (if/else)
static const char *CONTROL_FLOW_PROGRAM = "package main;\n"
                                          "\n"
                                          "pub fn main(none) -> void {\n"
                                          "    let x: int = 42;\n"
                                          "    if x > 0 {\n"
                                          "        let positive: bool = true;\n"
                                          "    } else {\n"
                                          "        let negative: bool = false;\n"
                                          "    }\n"
                                          "}";

// Test 5: Multi-line Strings in Context
static const char *MULTILINE_STRING_PROGRAM = "package main;\n"
                                              "\n"
                                              "pub fn main(none) -> void {\n"
                                              "    let config: string = r\"\"\"{\n"
                                              "        \"database\": \"localhost\",\n"
                                              "        \"port\": 5432\n"
                                              "    }\"\"\";\n"
                                              "    \n"
                                              "    let processed: string = \"\"\"Line 1\n"
                                              "Line 2\n"
                                              "Line 3\"\"\";\n"
                                              "}";

// Test 6: Array Operations with Types
static const char *ARRAY_PROGRAM = "package main;\n"
                                   "\n"
                                   "pub fn main(none) -> void {\n"
                                   "    let arr: []int = [1, 2, 3, 4, 5];\n"
                                   "    let first: int = arr[0];\n"
                                   "    let empty: []string = [];\n"
                                   "}";

// Test 7: If-let Statements in Functions
static const char *IF_LET_PROGRAM = "package main;\n"
                                    "\n"
                                    "priv fn process_option(opt: Option<int>) -> void {\n"
                                    "    if let Option.Some(value) = opt {\n"
                                    "        let doubled: int = value * 2;\n"
                                    "    }\n"
                                    "}\n"
                                    "\n"
                                    "pub fn main(none) -> void {\n"
                                    "    process_option(Option.Some(42));\n"
                                    "}";

// Test 8: Mixed Feature Program
static const char *MIXED_FEATURE_PROGRAM =
    "package main;\n"
    "\n"
    "priv fn calculate(numbers: []int) -> int {\n"
    "    let sum: int = 0;\n"
    "    for number in numbers {\n"
    "        sum = sum + number;\n"
    "    }\n"
    "    return sum;\n"
    "}\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let data: []int = [1, 2, 3, 4, 5];\n"
    "    let result: int = calculate(data);\n"
    "    \n"
    "    let message: string = r\"\"\"Calculation complete:\n"
    "    Result = \" + string(result);\"\"\";\n"
    "}";

// Test 9: Variable Declarations with Complex Types
static const char *COMPLEX_TYPES_PROGRAM =
    "package main;\n"
    "\n"
    "pub fn main(none) -> void {\n"
    "    let simple: int = 42;\n"
    "    let text: string = \"hello world\";\n"
    "    let flag: bool = true;\n"
    "    let numbers: []int = [10, 20, 30];\n"
    "    let words: []string = [\"one\", \"two\", \"three\"];\n"
    "    let flags: []bool = [true, false, true];\n"
    "}";

// Test 10: Nested Structures
static const char *NESTED_STRUCTURES_PROGRAM = "package main;\n"
                                               "\n"
                                               "pub fn main(none) -> void {\n"
                                               "    if true {\n"
                                               "        let x: int = 1;\n"
                                               "        if x > 0 {\n"
                                               "            let y: int = 2;\n"
                                               "            if y > x {\n"
                                               "                let z: int = x + y;\n"
                                               "            }\n"
                                               "        }\n"
                                               "    }\n"
                                               "}";

// =============================================================================
// VALID PROGRAM TEST SUITE CREATION
// =============================================================================

/**
 * Create test suite for valid programs that should parse successfully
 */
RealProgramTestSuite *create_valid_program_test_suite(void) {
    RealProgramTestSuite *suite = create_real_program_test_suite(
        "Valid Program Tests", "Tests that validate complete programs parse successfully");

    if (!suite) {
        return NULL;
    }

    // Add all valid program tests
    RealProgramTest tests[] = {{.name = "minimal_program",
                                .source_code = MINIMAL_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "type_usage_program",
                                .source_code = TYPE_USAGE_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "function_with_params",
                                .source_code = FUNCTION_WITH_PARAMS_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "control_flow_program",
                                .source_code = CONTROL_FLOW_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "multiline_string_program",
                                .source_code = MULTILINE_STRING_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "array_program",
                                .source_code = ARRAY_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "if_let_program",
                                .source_code = IF_LET_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "mixed_feature_program",
                                .source_code = MIXED_FEATURE_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1500,
                                .max_total_time_ms = 3000},
                               {.name = "complex_types_program",
                                .source_code = COMPLEX_TYPES_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000},
                               {.name = "nested_structures_program",
                                .source_code = NESTED_STRUCTURES_PROGRAM,
                                .should_parse = true,
                                .should_compile = true,
                                .should_execute = true,
                                .expected_error_pattern = NULL,
                                .max_parse_time_ms = 1000,
                                .max_total_time_ms = 2000}};

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