/**
 * Asthra Programming Language
 * Pattern Matching Tests - Advanced Pattern Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_pattern_matching_common.h"

// =============================================================================
// GUARD CONDITIONS TESTS
// =============================================================================

AsthraTestResult test_guard_conditions(AsthraTestContext* context) {
    // Valid guard conditions
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_guards(opt: TestOption<i32>) -> string {\n"
        "    match opt {\n"
        "        TestOption.Some(x) if x > 0 => \"positive\",\n"
        "        TestOption.Some(x) if x < 0 => \"negative\",\n"
        "        TestOption.Some(0) => \"zero\",\n"
        "        TestOption.None => \"none\"\n"
        "    }\n"
        "}\n",
        "valid_guard_conditions"
    ), "Valid guard conditions");
    
    // Guard with non-bool type
    ASTHRA_TEST_ASSERT(context, test_pattern_error(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_bad_guard(opt: TestOption<i32>) -> i32 {\n"
        "    match opt {\n"
        "        TestOption.Some(x) if \"not bool\" => x,  // Guard must be bool\n"
        "        _ => 0\n"
        "    }\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "non_bool_guard_condition"
    ), "Non-bool guard condition should fail");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// IF-LET STATEMENT TESTS
// =============================================================================

AsthraTestResult test_if_let_statements(AsthraTestContext* context) {
    // Valid if-let
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_if_let(opt: TestOption<i32>) -> i32 {\n"
        "    if let TestOption.Some(value) = opt {\n"
        "        return value + 1;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
        "valid_if_let"
    ), "Valid if-let statement");
    
    // Type mismatch in if-let
    ASTHRA_TEST_ASSERT(context, test_pattern_error(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_type_mismatch() -> i32 {\n"
        "    if let TestOption.Some(x) = \"not an option\" {  // Type mismatch\n"
        "        return x;\n"
        "    }\n"
        "    return 0;\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "if_let_type_mismatch"
    ), "Type mismatch in if-let should fail");
    
    // Nested if-let
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
        "pub fn test_nested_if_let(r: Result<TestOption<i32>, string>) -> i32 {\n"
        "    if let Result.Ok(TestOption.Some(value)) = r {\n"
        "        return value;\n"
        "    }\n"
        "    return -1;\n"
        "}\n",
        "nested_if_let"
    ), "Nested if-let pattern");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// PATTERN VARIABLE BINDING TESTS
// =============================================================================

AsthraTestResult test_pattern_variable_binding(AsthraTestContext* context) {
    // Valid variable binding
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_binding(opt: TestOption<i32>) -> i32 {\n"
        "    match opt {\n"
        "        TestOption.Some(x) => {\n"
        "            let y = x;  // x is bound in this scope\n"
        "            return y + 1;\n"
        "        },\n"
        "        TestOption.None => 0\n"
        "    }\n"
        "}\n",
        "valid_pattern_binding"
    ), "Valid pattern variable binding");
    
    // Variable not bound in arm
    ASTHRA_TEST_ASSERT(context, test_pattern_error(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_unbound_var(opt: TestOption<i32>) -> i32 {\n"
        "    match opt {\n"
        "        TestOption.Some(x) => x,\n"
        "        TestOption.None => {\n"
        "            let z = x;  // Error: x not bound in this arm\n"
        "            return z;\n"
        "        }\n"
        "    }\n"
        "}\n",
        SEMANTIC_ERROR_UNDEFINED_SYMBOL,
        "unbound_pattern_variable"
    ), "Unbound pattern variable should fail");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// WILDCARD AND LITERAL PATTERN TESTS
// =============================================================================

AsthraTestResult test_wildcard_and_literal_patterns(AsthraTestContext* context) {
    // Wildcard patterns
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub fn test_wildcard(n: i32) -> string {\n"
        "    match n {\n"
        "        0 => \"zero\",\n"
        "        1 => \"one\",\n"
        "        _ => \"other\"\n"
        "    }\n"
        "}\n",
        "valid_wildcard_pattern"
    ), "Valid wildcard pattern");
    
    // Range patterns
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub fn test_range(n: i32) -> string {\n"
        "    match n {\n"
        "        0 => \"zero\",\n"
        "        1..=10 => \"small\",\n"
        "        11..=100 => \"medium\",\n"
        "        _ => \"large\"\n"
        "    }\n"
        "}\n",
        "valid_range_patterns"
    ), "Valid range patterns");
    
    // String literal patterns
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub fn test_string_patterns(s: string) -> i32 {\n"
        "    match s {\n"
        "        \"hello\" => 1,\n"
        "        \"world\" => 2,\n"
        "        _ => 0\n"
        "    }\n"
        "}\n",
        "valid_string_patterns"
    ), "Valid string literal patterns");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TYPE COMPATIBILITY IN PATTERNS TESTS
// =============================================================================

AsthraTestResult test_type_compatibility_in_patterns(AsthraTestContext* context) {
    // All arms must have compatible types
    ASTHRA_TEST_ASSERT(context, test_pattern_error(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_incompatible_arms(opt: TestOption<i32>) -> void {\n"
        "    match opt {\n"
        "        TestOption.Some(x) => x,        // Returns i32\n"
        "        TestOption.None => \"default\"    // Returns string - ERROR\n"
        "    }\n"
        "}\n",
        SEMANTIC_ERROR_INCOMPATIBLE_TYPES,
        "incompatible_match_arms"
    ), "Incompatible match arm types should fail");
    
    // Fixed version with compatible types
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_compatible_arms(opt: TestOption<i32>) -> string {\n"
        "    match opt {\n"
        "        TestOption.Some(x) => x.to_string(),\n"
        "        TestOption.None => \"default\".to_string()\n"
        "    }\n"
        "}\n",
        "compatible_match_arms"
    ), "Compatible match arm types");
    
    // Type inference across match arms
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_type_inference(opt: TestOption<i32>) -> i32 {\n"
        "    let result = match opt {\n"
        "        TestOption.Some(x) => x * 2,\n"
        "        TestOption.None => 0\n"
        "    };\n"
        "    return result;\n"
        "}\n",
        "match_type_inference"
    ), "Type inference across match arms");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MATCH EXPRESSION VS STATEMENT TESTS
// =============================================================================

AsthraTestResult test_match_expression_vs_statement(AsthraTestContext* context) {
    // Match as expression
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_match_expression(opt: TestOption<i32>) -> i32 {\n"
        "    let value = match opt {\n"
        "        TestOption.Some(x) => x + 1,\n"
        "        TestOption.None => 0\n"
        "    };\n"
        "    return value * 2;\n"
        "}\n",
        "match_as_expression"
    ), "Match as expression");
    
    // Match as statement (no semicolon)
    ASTHRA_TEST_ASSERT(context, test_pattern_success(
        "package test;\n"
        "pub enum TestOption<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "pub fn test_match_statement(opt: TestOption<i32>) {\n"
        "    match opt {\n"
        "        TestOption.Some(x) => { print(x); },\n"
        "        TestOption.None => { print(\"none\"); }\n"
        "    }\n"
        "}\n",
        "match_as_statement"
    ), "Match as statement");
    
    return ASTHRA_TEST_PASS;
}