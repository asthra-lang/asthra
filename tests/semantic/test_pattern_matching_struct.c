/**
 * Asthra Programming Language
 * Pattern Matching Tests - Struct Pattern Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_pattern_matching_common.h"

// =============================================================================
// STRUCT PATTERN DESTRUCTURING TESTS
// =============================================================================

AsthraTestResult test_struct_pattern_destructuring(AsthraTestContext* context) {
    // Valid struct destructuring
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub fn test_destructure(p: Point) -> i32 {\n"
        "    match p {\n"
        "        Point { x, y } => x + y\n"
        "    }\n"
        "}\n",
        "valid_struct_destructuring"
    ), "Valid struct destructuring");
    
    // Struct pattern with specific value
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub fn test_specific_value(p: Point) -> i32 {\n"
        "    match p {\n"
        "        Point { x: 0, y } => y,\n"
        "        Point { x, y } => x + y\n"
        "    }\n"
        "}\n",
        "struct_pattern_specific_value"
    ), "Struct pattern with specific value");
    
    // Invalid field in pattern
    ASTHRA_TEST_ASSERT(context, test_pattern_error(
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub fn test_invalid_field(p: Point) -> i32 {\n"
        "    match p {\n"
        "        Point { x, z } => x  // No field 'z'\n"
        "    }\n"
        "}\n",
        SEMANTIC_ERROR_INVALID_STRUCT_FIELD,
        "invalid_field_in_pattern"
    ), "Invalid field in pattern should fail");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// NESTED PATTERN MATCHING TESTS
// =============================================================================

AsthraTestResult test_nested_pattern_matching(AsthraTestContext* context) {
    // Valid nested patterns
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub enum Shape {\n"
        "    Circle(Point, i32),\n"
        "    Rectangle(Point, Point)\n"
        "}\n"
        "pub fn test_nested(shape: Shape) -> i32 {\n"
        "    match shape {\n"
        "        Circle(Point { x: 0, y: 0 }, radius) => radius,\n"
        "        Circle(center, radius) => radius * 2,\n"
        "        Rectangle(p1, p2) => p1.x + p2.x\n"
        "    }\n"
        "}\n",
        "valid_nested_patterns"
    ), "Valid nested patterns");
    
    // Complex nested patterns with Option
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "pub fn test_complex_nested(r: Result<TestOption<i32>, string>) -> i32 {\n"
        "    match r {\n"
        "        Result.Ok(TestOption.Some(value)) => value,\n"
        "        Result.Ok(TestOption.None) => 0,\n"
        "        Result.Err(_) => -1\n"
        "    }\n"
        "}\n",
        "complex_nested_patterns"
    ), "Complex nested patterns");
    
    return ASTHRA_TEST_PASS;
}