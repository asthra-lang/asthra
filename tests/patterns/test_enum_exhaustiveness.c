/*
 * =============================================================================
 * ENUM PATTERN EXHAUSTIVENESS TESTS
 * =============================================================================
 *
 * This file contains tests for enum pattern matching exhaustiveness checking,
 * including complete and incomplete pattern coverage scenarios.
 *
 * Part of Phase 3.1: Advanced Pattern Matching
 *
 * Test Categories:
 * - Complete exhaustiveness checking
 * - Incomplete pattern detection
 * - Missing pattern reporting
 *
 * =============================================================================
 */

#include "test_pattern_common.h"

// =============================================================================
// ENUM PATTERN EXHAUSTIVENESS TESTS
// =============================================================================

/**
 * Test complete enum pattern exhaustiveness checking
 */
static bool test_complete_enum_exhaustiveness(void) {
    TEST_START("Complete enum exhaustiveness");

    const char *source = "enum Status {\n"
                         "    PENDING,\n"
                         "    RUNNING,\n"
                         "    COMPLETED,\n"
                         "    FAILED\n"
                         "}\n"
                         "\n"
                         "fn check_status(status: Status) -> String {\n"
                         "    match status {\n"
                         "        Status::PENDING => \"pending\",\n"
                         "        Status::RUNNING => \"running\",\n"
                         "        Status::COMPLETED => \"completed\",\n"
                         "        Status::FAILED => \"failed\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_enum_patterns(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_TRUE(pattern_result->is_exhaustive);
    ASSERT_EQ(pattern_result->missing_patterns, 0);
    ASSERT_EQ(pattern_result->pattern_count, 4);

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test incomplete enum pattern exhaustiveness checking
 */
static bool test_incomplete_enum_exhaustiveness(void) {
    TEST_START("Incomplete enum exhaustiveness");

    const char *source = "enum Status {\n"
                         "    PENDING,\n"
                         "    RUNNING,\n"
                         "    COMPLETED,\n"
                         "    FAILED\n"
                         "}\n"
                         "\n"
                         "fn check_status_incomplete(status: Status) -> String {\n"
                         "    match status {\n"
                         "        Status::PENDING => \"pending\",\n"
                         "        Status::RUNNING => \"running\"\n"
                         "        // Missing COMPLETED and FAILED\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    // This should fail semantic analysis due to incomplete patterns
    ASSERT_FALSE(result->success);

    PatternCompilationResult *pattern_result = compile_enum_patterns(ast);
    ASSERT_FALSE(pattern_result->success);
    ASSERT_FALSE(pattern_result->is_exhaustive);
    ASSERT_EQ(pattern_result->missing_patterns, 2);

    // Verify missing pattern names
    ASSERT_STRING_EQ(pattern_result->missing_pattern_names[0], "Status::COMPLETED");
    ASSERT_STRING_EQ(pattern_result->missing_pattern_names[1], "Status::FAILED");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test exhaustiveness with wildcard patterns
 */
static bool test_exhaustiveness_with_wildcard(void) {
    TEST_START("Exhaustiveness with wildcard patterns");

    const char *source = "enum Color {\n"
                         "    RED,\n"
                         "    GREEN,\n"
                         "    BLUE,\n"
                         "    YELLOW,\n"
                         "    PURPLE,\n"
                         "    ORANGE\n"
                         "}\n"
                         "\n"
                         "fn categorize_color(color: Color) -> String {\n"
                         "    match color {\n"
                         "        Color::RED => \"primary\",\n"
                         "        Color::GREEN => \"primary\",\n"
                         "        Color::BLUE => \"primary\",\n"
                         "        _ => \"secondary\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_enum_patterns(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_TRUE(pattern_result->is_exhaustive);
    ASSERT_EQ(pattern_result->missing_patterns, 0);

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test exhaustiveness with tagged union enums
 */
static bool test_tagged_union_exhaustiveness(void) {
    TEST_START("Tagged union exhaustiveness");

    const char *source = "enum Option<T> {\n"
                         "    Some(T),\n"
                         "    None\n"
                         "}\n"
                         "\n"
                         "fn handle_option_complete(opt: Option<i32>) -> String {\n"
                         "    match opt {\n"
                         "        Option::Some(value) => \"Value: \" + value.to_string(),\n"
                         "        Option::None => \"No value\"\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "fn handle_option_incomplete(opt: Option<i32>) -> String {\n"
                         "    match opt {\n"
                         "        Option::Some(value) => \"Value: \" + value.to_string()\n"
                         "        // Missing None case\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    // Test the complete function
    SemanticResult *complete_result = analyze_semantics(ast);
    ASSERT_TRUE(complete_result->success);

    PatternCompilationResult *complete_pattern = compile_enum_patterns(ast);
    ASSERT_TRUE(complete_pattern->success);
    ASSERT_TRUE(complete_pattern->is_exhaustive);

    cleanup_pattern_compilation_result(complete_pattern);
    cleanup_semantic_result(complete_result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test exhaustiveness with nested enum patterns
 */
static bool test_nested_enum_exhaustiveness(void) {
    TEST_START("Nested enum exhaustiveness");

    const char *source =
        "enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "\n"
        "fn handle_nested_complete(nested: Option<Result<i32, String>>) -> String {\n"
        "    match nested {\n"
        "        Option::Some(Result::Ok(value)) => \"Success: \" + value.to_string(),\n"
        "        Option::Some(Result::Err(error)) => \"Error: \" + error,\n"
        "        Option::None => \"No value\"\n"
        "    }\n"
        "}\n"
        "\n"
        "fn handle_nested_incomplete(nested: Option<Result<i32, String>>) -> String {\n"
        "    match nested {\n"
        "        Option::Some(Result::Ok(value)) => \"Success: \" + value.to_string(),\n"
        "        Option::None => \"No value\"\n"
        "        // Missing Option::Some(Result::Err(_))\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    // The incomplete function should cause semantic analysis to fail
    ASSERT_FALSE(result->success);

    PatternCompilationResult *pattern_result = compile_enum_patterns(ast);
    // Should detect missing patterns in nested structure
    ASSERT_FALSE(pattern_result->is_exhaustive);
    ASSERT_EQ(pattern_result->missing_patterns, 1);
    ASSERT_STRING_EQ(pattern_result->missing_pattern_names[0], "Option::Some(Result::Err(_))");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("ENUM PATTERN EXHAUSTIVENESS TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3: Advanced Pattern Matching Implementation\n\n");

    bool all_tests_passed = true;

    // Exhaustiveness tests
    if (!test_complete_enum_exhaustiveness())
        all_tests_passed = false;
    if (!test_incomplete_enum_exhaustiveness())
        all_tests_passed = false;
    if (!test_exhaustiveness_with_wildcard())
        all_tests_passed = false;
    if (!test_tagged_union_exhaustiveness())
        all_tests_passed = false;
    if (!test_nested_enum_exhaustiveness())
        all_tests_passed = false;

    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL ENUM PATTERN EXHAUSTIVENESS TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Exhaustiveness Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME ENUM PATTERN EXHAUSTIVENESS TESTS FAILED!\n");
        return 1;
    }
}
