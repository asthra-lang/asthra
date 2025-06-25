/*
 * =============================================================================
 * COMPLEX GUARD CONDITION TESTS
 * =============================================================================
 *
 * This file contains tests for complex guard expressions including
 * boolean logic, mathematical operations, and function calls.
 *
 * Part of Phase 3.2: Advanced Pattern Matching
 *
 * Test Categories:
 * - Complex boolean guard expressions
 * - Mathematical guard expressions
 * - Function calls in guards
 *
 * =============================================================================
 */

#include "test_guard_common.h"
#include "test_pattern_common.h"

// =============================================================================
// COMPLEX GUARD CONDITION TESTS
// =============================================================================

/**
 * Test complex boolean guard expressions
 */
static bool test_complex_boolean_guards(void) {
    TEST_START("Complex boolean guard expressions");

    const char *source =
        "fn validate_age_and_score(age: i32, score: f64) -> String {\n"
        "    match (age, score) {\n"
        "        (a, s) if a >= 18 && s >= 90.0 => \"adult high achiever\",\n"
        "        (a, s) if a >= 18 && s >= 70.0 => \"adult good performance\",\n"
        "        (a, s) if a < 18 && s >= 95.0 => \"young prodigy\",\n"
        "        (a, s) if a < 18 && s >= 80.0 => \"young high achiever\",\n"
        "        (a, s) if (a >= 65 || a < 13) && s < 50.0 => \"special consideration\",\n"
        "        _ => \"standard case\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 5);

    // Verify complex guard compilation
    ASSERT_TRUE(pattern_result->has_complex_guards);
    ASSERT_TRUE(pattern_result->has_logical_operators);

    // Test adult high achiever
    TupleValue test_tuple =
        create_tuple_value((Value[]){create_i32_value(25), create_f64_value(92.5)}, 2);
    String result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "adult high achiever");

    // Test young prodigy
    test_tuple = create_tuple_value((Value[]){create_i32_value(16), create_f64_value(97.0)}, 2);
    result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "young prodigy");

    // Test special consideration
    test_tuple = create_tuple_value((Value[]){create_i32_value(70), create_f64_value(45.0)}, 2);
    result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "special consideration");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test mathematical guard expressions
 */
static bool test_mathematical_guards(void) {
    TEST_START("Mathematical guard expressions");

    const char *source = "fn analyze_coordinates(x: f64, y: f64) -> String {\n"
                         "    match (x, y) {\n"
                         "        (a, b) if a * a + b * b <= 1.0 => \"inside unit circle\",\n"
                         "        (a, b) if abs(a) + abs(b) <= 1.0 => \"inside unit diamond\",\n"
                         "        (a, b) if max(abs(a), abs(b)) <= 1.0 => \"inside unit square\",\n"
                         "        (a, b) if a * a + b * b <= 4.0 => \"inside radius 2 circle\",\n"
                         "        (a, b) if a > 0.0 && b > 0.0 => \"first quadrant\",\n"
                         "        (a, b) if a < 0.0 && b > 0.0 => \"second quadrant\",\n"
                         "        (a, b) if a < 0.0 && b < 0.0 => \"third quadrant\",\n"
                         "        (a, b) if a > 0.0 && b < 0.0 => \"fourth quadrant\",\n"
                         "        _ => \"on axis\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 8);

    // Verify mathematical guard compilation
    ASSERT_TRUE(pattern_result->has_mathematical_guards);
    ASSERT_TRUE(pattern_result->has_function_calls_in_guards);

    // Test inside unit circle
    TupleValue test_tuple =
        create_tuple_value((Value[]){create_f64_value(0.5), create_f64_value(0.5)}, 2);
    String result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "inside unit circle");

    // Test first quadrant
    test_tuple = create_tuple_value((Value[]){create_f64_value(3.0), create_f64_value(2.0)}, 2);
    result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "first quadrant");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test multiple guard conditions on same pattern
 */
static bool test_multiple_guard_conditions(void) {
    TEST_START("Multiple guard conditions");

    const char *source =
        "fn complex_classification(x: i32, y: i32, z: i32) -> String {\n"
        "    match (x, y, z) {\n"
        "        (a, b, c) if a == b && b == c => \"all equal\",\n"
        "        (a, b, c) if a == b || b == c || a == c => \"two equal\",\n"
        "        (a, b, c) if a + b + c == 0 => \"sum to zero\",\n"
        "        (a, b, c) if a * b * c > 0 => \"all same sign\",\n"
        "        (a, b, c) if (a > 0) + (b > 0) + (c > 0) == 2 => \"two positive\",\n"
        "        (a, b, c) if abs(a) > abs(b) && abs(a) > abs(c) => \"a dominates\",\n"
        "        (a, b, c) if abs(b) > abs(a) && abs(b) > abs(c) => \"b dominates\",\n"
        "        (a, b, c) if abs(c) > abs(a) && abs(c) > abs(b) => \"c dominates\",\n"
        "        _ => \"no pattern\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 8);

    // Verify multiple guard compilation
    ASSERT_TRUE(pattern_result->has_multiple_guards);
    ASSERT_TRUE(pattern_result->optimizes_guard_evaluation);

    // Test all equal
    TupleValue test_tuple = create_tuple_value(
        (Value[]){create_i32_value(5), create_i32_value(5), create_i32_value(5)}, 3);
    String result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "all equal");

    // Test sum to zero
    test_tuple = create_tuple_value(
        (Value[]){create_i32_value(3), create_i32_value(-1), create_i32_value(-2)}, 3);
    result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "sum to zero");

    // Test a dominates
    test_tuple = create_tuple_value(
        (Value[]){create_i32_value(10), create_i32_value(2), create_i32_value(-3)}, 3);
    result_str = execute_pattern_match_with_value(pattern_result, &test_tuple);
    ASSERT_STRING_EQ(result_str.data, "a dominates");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test guard expressions with function calls
 */
static bool test_function_call_guards(void) {
    TEST_START("Function call guards");

    const char *source = "fn analyze_list(items: List<i32>) -> String {\n"
                         "    match items {\n"
                         "        list if list.is_empty() => \"empty list\",\n"
                         "        list if list.length() == 1 => \"single item\",\n"
                         "        list if list.all(|x| x > 0) => \"all positive\",\n"
                         "        list if list.any(|x| x < 0) => \"has negative\",\n"
                         "        list if list.sum() > 100 => \"large sum\",\n"
                         "        _ => \"regular list\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 5);

    // Verify function call guard compilation
    ASSERT_TRUE(pattern_result->has_function_calls_in_guards);
    ASSERT_TRUE(pattern_result->has_complex_guards);

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
    printf("COMPLEX GUARD CONDITION TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.2: Advanced Pattern Matching\n\n");

    setup_guard_test_environment();

    bool all_tests_passed = true;

    // Complex guard condition tests
    if (!test_complex_boolean_guards())
        all_tests_passed = false;
    if (!test_mathematical_guards())
        all_tests_passed = false;
    if (!test_multiple_guard_conditions())
        all_tests_passed = false;
    if (!test_function_call_guards())
        all_tests_passed = false;

    cleanup_guard_test_environment();

    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL COMPLEX GUARD CONDITION TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Complex Guard Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME COMPLEX GUARD CONDITION TESTS FAILED!\n");
        return 1;
    }
}
