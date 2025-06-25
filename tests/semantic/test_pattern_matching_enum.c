/**
 * Asthra Programming Language
 * Pattern Matching Tests - Enum Pattern Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_pattern_matching_common.h"

// =============================================================================
// BASIC ENUM PATTERN MATCHING TESTS
// =============================================================================

AsthraTestResult test_basic_enum_pattern_matching(AsthraTestContext *context) {
    // Valid exhaustive match
    ASTHRA_TEST_ASSERT(context,
                       test_pattern_success("package test;\n"
                                            "pub enum TestOption<T> {\n"
                                            "    Some(T),\n"
                                            "    None\n"
                                            "}\n"
                                            "pub fn test_match(opt: TestOption<i32>) -> i32 {\n"
                                            "    match opt {\n"
                                            "        TestOption.Some(value) => value + 1,\n"
                                            "        TestOption.None => 0\n"
                                            "    }\n"
                                            "}\n",
                                            "valid_exhaustive_enum_match"),
                       "Valid exhaustive enum match");

    // Non-exhaustive match
    ASTHRA_TEST_ASSERT(context,
                       test_pattern_error("package test;\n"
                                          "pub enum TestOption<T> {\n"
                                          "    Some(T),\n"
                                          "    None\n"
                                          "}\n"
                                          "pub fn test_match(opt: TestOption<i32>) -> i32 {\n"
                                          "    match opt {\n"
                                          "        TestOption.Some(value) => value + 1\n"
                                          "        // Missing None case\n"
                                          "    }\n"
                                          "}\n",
                                          SEMANTIC_ERROR_NON_EXHAUSTIVE_MATCH,
                                          "non_exhaustive_enum_match"),
                       "Non-exhaustive enum match should fail");

    // Unknown variant
    ASTHRA_TEST_ASSERT(context,
                       test_pattern_error("package test;\n"
                                          "pub enum TestOption<T> {\n"
                                          "    Some(T),\n"
                                          "    None\n"
                                          "}\n"
                                          "pub fn test_match(opt: TestOption<i32>) -> i32 {\n"
                                          "    match opt {\n"
                                          "        TestOption.Some(value) => value,\n"
                                          "        Nothing => 0  // Wrong variant name\n"
                                          "    }\n"
                                          "}\n",
                                          SEMANTIC_ERROR_UNDEFINED_SYMBOL, "unknown_enum_variant"),
                       "Unknown enum variant should fail");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// EXHAUSTIVENESS COMPLEX CASE TESTS
// =============================================================================

AsthraTestResult test_exhaustiveness_complex_cases(AsthraTestContext *context) {
    // Missing multiple cases
    ASTHRA_TEST_ASSERT(context,
                       test_pattern_error("package test;\n"
                                          "pub enum Color {\n"
                                          "    Red,\n"
                                          "    Green,\n"
                                          "    Blue\n"
                                          "}\n"
                                          "pub fn test_missing_cases(c: Color) -> string {\n"
                                          "    match c {\n"
                                          "        Red => \"red\"\n"
                                          "        // Missing Green and Blue\n"
                                          "    }\n"
                                          "}\n",
                                          SEMANTIC_ERROR_NON_EXHAUSTIVE_MATCH,
                                          "missing_multiple_cases"),
                       "Missing multiple cases should fail");

    // Exhaustive with wildcard
    ASTHRA_TEST_ASSERT(
        context,
        test_pattern_success("package test;\n"
                             "pub enum Color {\n"
                             "    Red,\n"
                             "    Green,\n"
                             "    Blue\n"
                             "}\n"
                             "pub fn test_wildcard_exhaustive(c: Color) -> string {\n"
                             "    match c {\n"
                             "        Red => \"red\",\n"
                             "        _ => \"other\"  // Wildcard covers remaining\n"
                             "    }\n"
                             "}\n",
                             "exhaustive_with_wildcard"),
        "Exhaustive match with wildcard");

    // Complex enum with multiple constructors
    ASTHRA_TEST_ASSERT(context,
                       test_pattern_success("package test;\n"
                                            "pub enum Message {\n"
                                            "    Quit,\n"
                                            "    Move { x: i32, y: i32 },\n"
                                            "    Write(string),\n"
                                            "    ChangeColor(i32, i32, i32)\n"
                                            "}\n"
                                            "pub fn test_complex_enum(msg: Message) -> i32 {\n"
                                            "    match msg {\n"
                                            "        Quit => 0,\n"
                                            "        Move { x, y } => x + y,\n"
                                            "        Write(text) => text.len(),\n"
                                            "        ChangeColor(r, g, b) => r + g + b\n"
                                            "    }\n"
                                            "}\n",
                                            "complex_enum_exhaustive"),
                       "Complex enum exhaustive match");

    return ASTHRA_TEST_PASS;
}