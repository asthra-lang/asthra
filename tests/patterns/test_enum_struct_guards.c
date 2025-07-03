/*
 * =============================================================================
 * ENUM AND STRUCT GUARD EXPRESSION TESTS
 * =============================================================================
 *
 * This file contains tests for guard expressions applied to enum and
 * struct patterns, including value extraction and field access.
 *
 * Part of Phase 3.2: Advanced Pattern Matching
 *
 * Test Categories:
 * - Guard expressions with enum values
 * - Complex enum guards with multiple conditions
 * - Guard expressions with struct fields
 * - Mixed enum and struct guard patterns
 *
 * =============================================================================
 */

#include "test_guard_common.h"
#include "test_pattern_common.h"

// =============================================================================
// GUARD EXPRESSIONS WITH ENUMS
// =============================================================================

/**
 * Test guard expressions with enum values
 */
static bool test_enum_guards(void) {
    TEST_START("Guard expressions with enums");

    const char *source =
        "enum Option<T> {\n"
        "    Some(T),\n"
        "    None\n"
        "}\n"
        "\n"
        "fn process_optional_number(opt: Option<i32>) -> String {\n"
        "    match opt {\n"
        "        Option.Some(x) if x > 100 => \"large value: \" + x.to_string(),\n"
        "        Option.Some(x) if x > 0 => \"positive value: \" + x.to_string(),\n"
        "        Option.Some(x) if x == 0 => \"zero value\",\n"
        "        Option.Some(x) if x < 0 => \"negative value: \" + x.to_string(),\n"
        "        Option.None => \"no value\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 4);

    // Verify enum guard compilation
    ASSERT_TRUE(pattern_result->has_enum_guards);
    ASSERT_TRUE(pattern_result->extracts_enum_values);

    // Test large value
    TaggedUnionValue some_large =
        create_tagged_union_value("Option", "Some", create_i32_value(150));
    String result_str = execute_pattern_match_with_value(pattern_result, &some_large);
    ASSERT_STRING_EQ(result_str.data, "large value: 150");

    // Test positive value
    TaggedUnionValue some_positive =
        create_tagged_union_value("Option", "Some", create_i32_value(42));
    result_str = execute_pattern_match_with_value(pattern_result, &some_positive);
    ASSERT_STRING_EQ(result_str.data, "positive value: 42");

    // Test zero
    TaggedUnionValue some_zero = create_tagged_union_value("Option", "Some", create_i32_value(0));
    result_str = execute_pattern_match_with_value(pattern_result, &some_zero);
    ASSERT_STRING_EQ(result_str.data, "zero value");

    // Test None
    TaggedUnionValue none_value = create_tagged_union_value("Option", "None", NULL);
    result_str = execute_pattern_match_with_value(pattern_result, &none_value);
    ASSERT_STRING_EQ(result_str.data, "no value");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test complex enum guards with multiple conditions
 */
static bool test_complex_enum_guards(void) {
    TEST_START("Complex enum guards");

    const char *source =
        "enum Result<T, E> {\n"
        "    Ok(T),\n"
        "    Err(E)\n"
        "}\n"
        "\n"
        "fn analyze_result(res: Result<i32, String>) -> String {\n"
        "    match res {\n"
        "        Result.Ok(x) if x >= 0 && x <= 100 => \"valid percentage: \" + x.to_string(),\n"
        "        Result.Ok(x) if x > 100 => \"over 100: \" + x.to_string(),\n"
        "        Result.Ok(x) if x < 0 => \"negative: \" + x.to_string(),\n"
        "        Result.Err(msg) if msg.length() > 50 => \"long error message\",\n"
        "        Result.Err(msg) if msg.contains(\"timeout\") => \"timeout error\",\n"
        "        Result.Err(msg) if msg.contains(\"network\") => \"network error\",\n"
        "        Result.Err(_) => \"other error\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 6);

    // Test valid percentage
    TaggedUnionValue ok_valid = create_tagged_union_value("Result", "Ok", create_i32_value(75));
    String result_str = execute_pattern_match_with_value(pattern_result, &ok_valid);
    ASSERT_STRING_EQ(result_str.data, "valid percentage: 75");

    // Test timeout error
    TaggedUnionValue err_timeout =
        create_tagged_union_value("Result", "Err", create_string_value("connection timeout"));
    result_str = execute_pattern_match_with_value(pattern_result, &err_timeout);
    ASSERT_STRING_EQ(result_str.data, "timeout error");

    // Test network error
    TaggedUnionValue err_network =
        create_tagged_union_value("Result", "Err", create_string_value("network unreachable"));
    result_str = execute_pattern_match_with_value(pattern_result, &err_network);
    ASSERT_STRING_EQ(result_str.data, "network error");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

// =============================================================================
// GUARD EXPRESSIONS WITH STRUCTS
// =============================================================================

/**
 * Test guard expressions with struct fields
 */
static bool test_struct_guards(void) {
    TEST_START("Guard expressions with structs");

    const char *source =
        "struct Point {\n"
        "    x: f64,\n"
        "    y: f64\n"
        "}\n"
        "\n"
        "struct Rectangle {\n"
        "    top_left: Point,\n"
        "    width: f64,\n"
        "    height: f64\n"
        "}\n"
        "\n"
        "fn analyze_rectangle(rect: Rectangle) -> String {\n"
        "    match rect {\n"
        "        Rectangle { top_left: Point { x, y }, width: w, height: h } if w == h => "
        "\"square\",\n"
        "        Rectangle { top_left: Point { x, y }, width: w, height: h } if w > h => \"wide "
        "rectangle\",\n"
        "        Rectangle { top_left: Point { x, y }, width: w, height: h } if h > w => \"tall "
        "rectangle\",\n"
        "        Rectangle { top_left: Point { x, y }, width: w, height: h } if x < 0.0 || y < 0.0 "
        "=> \"negative position\",\n"
        "        Rectangle { width: w, height: h, .. } if w * h > 100.0 => \"large area\",\n"
        "        _ => \"small rectangle\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 5);

    // Verify struct guard compilation
    ASSERT_TRUE(pattern_result->has_struct_guards);
    ASSERT_TRUE(pattern_result->extracts_nested_fields);

    // Test square
    StructValue point_fields = create_struct_value(
        (StructValue[]){{"x", create_f64_value(10.0)}, {"y", create_f64_value(20.0)}}, 2);

    StructValue rect_fields =
        create_struct_value((StructValue[]){{"top_left", &point_fields},
                                            {"width", create_f64_value(5.0)},
                                            {"height", create_f64_value(5.0)}},
                            3);

    String result_str = execute_pattern_match_with_value(pattern_result, &rect_fields);
    ASSERT_STRING_EQ(result_str.data, "square");

    // Test wide rectangle
    rect_fields = create_struct_value((StructValue[]){{"top_left", &point_fields},
                                                      {"width", create_f64_value(10.0)},
                                                      {"height", create_f64_value(5.0)}},
                                      3);

    result_str = execute_pattern_match_with_value(pattern_result, &rect_fields);
    ASSERT_STRING_EQ(result_str.data, "wide rectangle");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test mixed enum and struct guards
 */
static bool test_mixed_enum_struct_guards(void) {
    TEST_START("Mixed enum and struct guards");

    const char *source =
        "struct User {\n"
        "    name: String,\n"
        "    age: i32,\n"
        "    score: f64\n"
        "}\n"
        "\n"
        "enum UserResult {\n"
        "    Valid(User),\n"
        "    Invalid { reason: String, attempted_user: User }\n"
        "}\n"
        "\n"
        "fn process_user_result(result: UserResult) -> String {\n"
        "    match result {\n"
        "        UserResult::Valid(User { name, age, score }) if age >= 18 && score >= 80.0 => "
        "\"adult high performer\",\n"
        "        UserResult::Valid(User { name, age, score }) if age >= 18 => \"adult user\",\n"
        "        UserResult::Valid(User { name, age, score }) if age < 18 && score >= 90.0 => "
        "\"young achiever\",\n"
        "        UserResult::Valid(User { name, age, score }) if name.length() == 0 => \"anonymous "
        "user\",\n"
        "        UserResult::Valid(_) => \"regular user\",\n"
        "        UserResult::Invalid { reason, attempted_user: User { age, .. } } if age < 0 => "
        "\"invalid age\",\n"
        "        UserResult::Invalid { reason, .. } if reason.contains(\"name\") => \"name "
        "validation error\",\n"
        "        UserResult::Invalid { .. } => \"general validation error\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 7);

    // Verify mixed guard compilation
    ASSERT_TRUE(pattern_result->has_enum_guards);
    ASSERT_TRUE(pattern_result->has_struct_guards);
    ASSERT_TRUE(pattern_result->extracts_enum_values);
    ASSERT_TRUE(pattern_result->extracts_nested_fields);

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
    printf("ENUM AND STRUCT GUARD EXPRESSION TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.2: Advanced Pattern Matching\n\n");

    setup_guard_test_environment();

    bool all_tests_passed = true;

    // Guard expressions with enums
    if (!test_enum_guards())
        all_tests_passed = false;
    if (!test_complex_enum_guards())
        all_tests_passed = false;

    // Guard expressions with structs
    if (!test_struct_guards())
        all_tests_passed = false;
    if (!test_mixed_enum_struct_guards())
        all_tests_passed = false;

    cleanup_guard_test_environment();

    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL ENUM AND STRUCT GUARD EXPRESSION TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Enum/Struct Guard Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME ENUM AND STRUCT GUARD EXPRESSION TESTS FAILED!\n");
        return 1;
    }
}
