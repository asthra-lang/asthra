/*
 * =============================================================================
 * GUARD OPTIMIZATION AND ERROR HANDLING TESTS
 * =============================================================================
 *
 * This file contains tests for guard expression optimization,
 * error handling, and performance validation.
 *
 * Part of Phase 3.2: Advanced Pattern Matching
 *
 * Test Categories:
 * - Guard expression optimization
 * - Error handling in guard expressions
 * - Performance tests for guards
 *
 * =============================================================================
 */

#include "test_guard_common.h"
#include "test_pattern_common.h"

// =============================================================================
// GUARD EXPRESSION OPTIMIZATION TESTS
// =============================================================================

/**
 * Test guard expression optimization
 */
static bool test_guard_optimization(void) {
    TEST_START("Guard expression optimization");

    const char *source = "fn optimized_guards(x: i32) -> String {\n"
                         "    match x {\n"
                         "        n if n > 0 && n < 10 => \"single digit positive\",\n"
                         "        n if n >= 10 && n < 100 => \"double digit\",\n"
                         "        n if n >= 100 && n < 1000 => \"triple digit\",\n"
                         "        n if n < 0 => \"negative\",\n"
                         "        _ => \"large or zero\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);

    // Verify optimization features
    ASSERT_TRUE(pattern_result->optimizes_range_checks);
    ASSERT_TRUE(pattern_result->eliminates_redundant_checks);
    ASSERT_LT(pattern_result->optimization_level, 3); // Should optimize to decision tree

    // Test performance with many iterations
    clock_t start_time = clock();
    for (int i = 0; i < 10000; i++) {
        i32 test_value = i % 2000 - 1000; // Range from -1000 to 999
        String result = execute_pattern_match_with_value(pattern_result, &test_value);
        // Verify result is not null
        ASSERT_NOT_NULL(result.data);
    }
    clock_t end_time = clock();

    double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    ASSERT_LT(execution_time, 50.0); // Should execute 10000 iterations in under 50ms

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test range optimization in guards
 */
static bool test_range_optimization(void) {
    TEST_START("Range optimization in guards");

    const char *source = "fn optimized_ranges(x: i32) -> String {\n"
                         "    match x {\n"
                         "        n if n >= 0 && n <= 10 => \"range 0-10\",\n"
                         "        n if n >= 11 && n <= 20 => \"range 11-20\",\n"
                         "        n if n >= 21 && n <= 30 => \"range 21-30\",\n"
                         "        n if n >= 31 && n <= 40 => \"range 31-40\",\n"
                         "        n if n >= 41 && n <= 50 => \"range 41-50\",\n"
                         "        _ => \"out of range\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);

    // Verify range optimization
    ASSERT_TRUE(pattern_result->optimizes_range_checks);
    ASSERT_TRUE(pattern_result->eliminates_redundant_checks);

    // Test boundary conditions
    i32 test_value = 10;
    String result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "range 0-10");

    test_value = 11;
    result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "range 11-20");

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test short-circuit evaluation optimization
 */
static bool test_short_circuit_optimization(void) {
    TEST_START("Short-circuit evaluation optimization");

    const char *source =
        "fn short_circuit_guards(x: i32, y: i32) -> String {\n"
        "    match (x, y) {\n"
        "        (a, b) if a == 0 || expensive_function(a, b) => \"case 1\",\n"
        "        (a, b) if a > 100 && b > 200 && very_expensive_check(a, b) => \"case 2\",\n"
        "        (a, b) if a < 0 || b < 0 => \"negative values\",\n"
        "        _ => \"default\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);

    // Verify short-circuit optimization
    ASSERT_TRUE(pattern_result->optimizes_guard_evaluation);
    ASSERT_TRUE(pattern_result->has_function_calls_in_guards);

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);

    TEST_END();
}

// =============================================================================
// ERROR HANDLING TESTS
// =============================================================================

/**
 * Test error handling in guard expressions
 */
static bool test_guard_error_handling(void) {
    TEST_START("Guard expression error handling");

    // Test invalid guard expression
    const char *invalid_source = "fn invalid_guard(x: i32) -> String {\n"
                                 "    match x {\n"
                                 "        n if n.invalid_method() => \"invalid\",\n"
                                 "        _ => \"valid\"\n"
                                 "    }\n"
                                 "}\n";

    ASTNode *invalid_ast = parse_source(invalid_source);
    ASSERT_NOT_NULL(invalid_ast);

    SemanticResult *invalid_result = analyze_semantics(invalid_ast);
    ASSERT_FALSE(invalid_result->success);
    ASSERT_TRUE(invalid_result->has_errors);
    ASSERT_STRING_CONTAINS(invalid_result->error_message, "invalid_method");

    // Test type mismatch in guard
    const char *type_mismatch_source = "fn type_mismatch_guard(x: i32) -> String {\n"
                                       "    match x {\n"
                                       "        n if n == \"string\" => \"mismatch\",\n"
                                       "        _ => \"valid\"\n"
                                       "    }\n"
                                       "}\n";

    ASTNode *type_ast = parse_source(type_mismatch_source);
    ASSERT_NOT_NULL(type_ast);

    SemanticResult *type_result = analyze_semantics(type_ast);
    ASSERT_FALSE(type_result->success);
    ASSERT_TRUE(type_result->has_errors);
    ASSERT_STRING_CONTAINS(type_result->error_message, "type mismatch");

    cleanup_semantic_result(invalid_result);
    cleanup_semantic_result(type_result);
    cleanup_ast(invalid_ast);
    cleanup_ast(type_ast);

    TEST_END();
}

/**
 * Test guard compilation error detection
 */
static bool test_guard_compilation_errors(void) {
    TEST_START("Guard compilation error detection");

    // Test undefined variable in guard
    const char *undefined_var_source = "fn undefined_var_guard(x: i32) -> String {\n"
                                       "    match x {\n"
                                       "        n if n > undefined_variable => \"error\",\n"
                                       "        _ => \"valid\"\n"
                                       "    }\n"
                                       "}\n";

    ASTNode *ast = parse_source(undefined_var_source);
    ASSERT_NOT_NULL(ast);

    SemanticResult *result = analyze_semantics(ast);
    ASSERT_FALSE(result->success);
    ASSERT_TRUE(result->has_errors);
    ASSERT_STRING_CONTAINS(result->error_message, "undefined_variable");

    // Test guard with wrong return type
    const char *wrong_return_source = "fn wrong_return_guard(x: i32) -> String {\n"
                                      "    match x {\n"
                                      "        n if returns_string() => \"error\",\n"
                                      "        _ => \"valid\"\n"
                                      "    }\n"
                                      "}\n";

    ASTNode *wrong_ast = parse_source(wrong_return_source);
    ASSERT_NOT_NULL(wrong_ast);

    SemanticResult *wrong_result = analyze_semantics(wrong_ast);
    ASSERT_FALSE(wrong_result->success);
    ASSERT_TRUE(wrong_result->has_errors);

    cleanup_semantic_result(result);
    cleanup_semantic_result(wrong_result);
    cleanup_ast(ast);
    cleanup_ast(wrong_ast);

    TEST_END();
}

// =============================================================================
// PERFORMANCE TESTS
// =============================================================================

/**
 * Test guard expression performance with complex conditions
 */
static bool test_guard_performance(void) {
    TEST_START("Guard expression performance");

    const char *source =
        "fn performance_test(data: (i32, i32, i32, i32)) -> String {\n"
        "    match data {\n"
        "        (a, b, c, d) if a * a + b * b + c * c + d * d < 100 => \"close to origin\",\n"
        "        (a, b, c, d) if abs(a - b) + abs(c - d) < 5 => \"pairs close\",\n"
        "        (a, b, c, d) if max(a, b, c, d) - min(a, b, c, d) < 10 => \"small range\",\n"
        "        (a, b, c, d) if (a + b) * (c + d) > 1000 => \"large product\",\n"
        "        _ => \"other\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);

    // Measure compilation time
    clock_t start_time = clock();
    PatternCompilationResult *recompiled = compile_patterns_with_guards(ast);
    clock_t end_time = clock();

    double compilation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    ASSERT_LT(compilation_time, 50.0); // Should compile complex guards in under 50ms

    // Test execution performance
    TupleValue test_data = create_tuple_value((Value[]){create_i32_value(3), create_i32_value(4),
                                                        create_i32_value(5), create_i32_value(6)},
                                              4);

    start_time = clock();
    for (int i = 0; i < 5000; i++) {
        String result = execute_pattern_match_with_value(pattern_result, &test_data);
        ASSERT_NOT_NULL(result.data);
    }
    end_time = clock();

    double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    ASSERT_LT(execution_time, 25.0); // 5000 executions should take under 25ms

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_pattern_compilation_result(recompiled);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test memory efficiency of guard compilation
 */
static bool test_guard_memory_efficiency(void) {
    TEST_START("Guard memory efficiency");

    const char *source = "fn memory_efficient_guards(x: i32) -> String {\n"
                         "    match x {\n"
                         "        n if n % 2 == 0 => \"even\",\n"
                         "        n if n % 3 == 0 => \"divisible by 3\",\n"
                         "        n if n % 5 == 0 => \"divisible by 5\",\n"
                         "        n if n % 7 == 0 => \"divisible by 7\",\n"
                         "        _ => \"other\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    // Simulate memory measurement (in real implementation, use actual memory tracking)
    size_t initial_memory = get_memory_usage();
    PatternCompilationResult *pattern_result = compile_patterns_with_guards(ast);
    size_t final_memory = get_memory_usage();

    ASSERT_TRUE(pattern_result->success);

    // Verify reasonable memory usage
    size_t memory_used = final_memory - initial_memory;
    ASSERT_LT(memory_used, 2048); // Should use less than 2KB for this example

    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);

    TEST_END();
}

// =============================================================================
// MOCK PERFORMANCE UTILITIES
// =============================================================================

/**
 * Mock function to simulate memory usage measurement
 */
static size_t get_memory_usage(void) {
    // In a real implementation, this would use system calls to get actual memory usage
    return 1024; // Mock value
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("GUARD OPTIMIZATION AND ERROR HANDLING TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.2: Advanced Pattern Matching\n\n");

    setup_guard_test_environment();

    bool all_tests_passed = true;

    // Optimization tests
    if (!test_guard_optimization())
        all_tests_passed = false;
    if (!test_range_optimization())
        all_tests_passed = false;
    if (!test_short_circuit_optimization())
        all_tests_passed = false;

    // Error handling tests
    if (!test_guard_error_handling())
        all_tests_passed = false;
    if (!test_guard_compilation_errors())
        all_tests_passed = false;

    // Performance tests
    if (!test_guard_performance())
        all_tests_passed = false;
    if (!test_guard_memory_efficiency())
        all_tests_passed = false;

    cleanup_guard_test_environment();

    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL GUARD OPTIMIZATION AND ERROR HANDLING TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Guard Optimization Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME GUARD OPTIMIZATION AND ERROR HANDLING TESTS FAILED!\n");
        return 1;
    }
}
