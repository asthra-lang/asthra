/*
 * =============================================================================
 * PATTERN COMPILATION OPTIMIZATION TESTS
 * =============================================================================
 *
 * This file contains tests for pattern compilation optimization techniques
 * including basic optimization, decision trees, and compilation analysis.
 *
 * Part of Phase 3.3: Advanced Pattern Matching
 *
 * Test Categories:
 * - Basic pattern compilation optimization
 * - Decision tree optimization
 * - Compilation time benchmarks
 *
 * =============================================================================
 */

#include "test_optimization_common.h"
#include "test_pattern_common.h"

// =============================================================================
// PATTERN COMPILATION OPTIMIZATION TESTS
// =============================================================================

/**
 * Test basic pattern compilation optimization
 */
static bool test_basic_pattern_optimization(void) {
    TEST_START("Basic pattern compilation optimization");

    const char *source = "fn simple_match(x: i32) -> String {\n"
                         "    match x {\n"
                         "        1 => \"one\",\n"
                         "        2 => \"two\",\n"
                         "        3 => \"three\",\n"
                         "        4 => \"four\",\n"
                         "        5 => \"five\",\n"
                         "        _ => \"other\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    // Compile without optimization
    PatternCompilationResult *unoptimized = compile_patterns(ast, OPTIMIZATION_NONE);
    ASSERT_TRUE(unoptimized->success);

    // Compile with optimization
    PatternCompilationResult *optimized = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(optimized->success);

    // Verify optimization improvements
    ASSERT_LT(optimized->instruction_count, unoptimized->instruction_count);
    ASSERT_TRUE(optimized->uses_jump_table);
    ASSERT_FALSE(unoptimized->uses_jump_table);

    // Test performance difference
    i32 test_value = 3;

    PerformanceMeasurement unopt_perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        String result = execute_pattern_match_with_value(unoptimized, &test_value);
        ASSERT_STRING_EQ(result.data, "three");
    }
    end_performance_measurement(&unopt_perf);

    PerformanceMeasurement opt_perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        String result = execute_pattern_match_with_value(optimized, &test_value);
        ASSERT_STRING_EQ(result.data, "three");
    }
    end_performance_measurement(&opt_perf);

    double unopt_time = get_cpu_time_ms(&unopt_perf);
    double opt_time = get_cpu_time_ms(&opt_perf);

    // Optimized version should be at least 20% faster
    ASSERT_LT(opt_time, unopt_time * 0.8);

    cleanup_pattern_compilation_result(unoptimized);
    cleanup_pattern_compilation_result(optimized);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test decision tree optimization for complex patterns
 */
static bool test_decision_tree_optimization(void) {
    TEST_START("Decision tree optimization");

    const char *source = "priv struct Point { x: i32, y: i32 }\n"
                         "\n"
                         "pub fn classify_point(p: Point) -> String {\n"
                         "    match p {\n"
                         "        Point { x: 0, y: 0 } => \"origin\",\n"
                         "        Point { x: 0, y } => \"y-axis\",\n"
                         "        Point { x, y: 0 } => \"x-axis\",\n"
                         "        Point { x, y } if x > 0 && y > 0 => \"first quadrant\",\n"
                         "        Point { x, y } if x < 0 && y > 0 => \"second quadrant\",\n"
                         "        Point { x, y } if x < 0 && y < 0 => \"third quadrant\",\n"
                         "        Point { x, y } if x > 0 && y < 0 => \"fourth quadrant\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);

    // Verify decision tree optimization
    ASSERT_TRUE(result->uses_decision_tree);
    ASSERT_EQ(result->decision_tree_depth, 3);   // Should optimize to depth 3
    ASSERT_LT(result->average_comparisons, 4.0); // Should average less than 4 comparisons

    // Test various points
    StructValue origin = create_struct_value(
        (StructValue[]){{"x", create_i32_value(0)}, {"y", create_i32_value(0)}}, 2);
    String result_str = execute_pattern_match_with_value(result, &origin);
    ASSERT_STRING_EQ(result_str.data, "origin");

    StructValue first_quad = create_struct_value(
        (StructValue[]){{"x", create_i32_value(5)}, {"y", create_i32_value(3)}}, 2);
    result_str = execute_pattern_match_with_value(result, &first_quad);
    ASSERT_STRING_EQ(result_str.data, "first quadrant");

    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test compilation time performance
 */
static bool test_compilation_time_performance(void) {
    TEST_START("Compilation time performance");

    // Test with increasingly complex patterns
    const char *simple_source = "pub fn simple(x: i32) -> String {\n"
                                "    match x {\n"
                                "        1 => \"one\",\n"
                                "        2 => \"two\",\n"
                                "        _ => \"other\"\n"
                                "    }\n"
                                "}\n";

    const char *medium_source =
        "pub enum Status { A, B, C, D, E, F, G, H, I, J }\n"
        "pub fn medium(s: Status) -> String {\n"
        "    match s {\n"
        "        Status::A => \"a\", Status::B => \"b\", Status::C => \"c\",\n"
        "        Status::D => \"d\", Status::E => \"e\", Status::F => \"f\",\n"
        "        Status::G => \"g\", Status::H => \"h\", Status::I => \"i\",\n"
        "        Status::J => \"j\"\n"
        "    }\n"
        "}\n";

    const char *complex_source =
        "priv struct Point { x: i32, y: i32 }\n"
        "pub enum Shape {\n"
        "    Circle { center: Point, radius: f64 },\n"
        "    Rectangle { top_left: Point, bottom_right: Point },\n"
        "    Triangle { p1: Point, p2: Point, p3: Point }\n"
        "}\n"
        "pub fn complex(s: Shape) -> String {\n"
        "    match s {\n"
        "        Shape::Circle { center: Point { x: 0, y: 0 }, radius: r } if r > 0.0 => \"origin "
        "circle\",\n"
        "        Shape::Circle { center, radius } if radius > 10.0 => \"large circle\",\n"
        "        Shape::Circle { .. } => \"small circle\",\n"
        "        Shape::Rectangle { top_left: Point { x: x1, y: y1 }, bottom_right: Point { x: x2, "
        "y: y2 } } if (x2-x1) == (y2-y1) => \"square\",\n"
        "        Shape::Rectangle { .. } => \"rectangle\",\n"
        "        Shape::Triangle { .. } => \"triangle\"\n"
        "    }\n"
        "}\n";

    // Test simple pattern compilation time
    ASTNode *simple_ast = parse_source(simple_source);
    PerformanceMeasurement simple_perf = start_performance_measurement();
    PatternCompilationResult *simple_result = compile_patterns(simple_ast, OPTIMIZATION_FULL);
    end_performance_measurement(&simple_perf);
    ASSERT_TRUE(simple_result->success);
    double simple_time = get_cpu_time_ms(&simple_perf);

    // Test medium pattern compilation time
    ASTNode *medium_ast = parse_source(medium_source);
    PerformanceMeasurement medium_perf = start_performance_measurement();
    PatternCompilationResult *medium_result = compile_patterns(medium_ast, OPTIMIZATION_FULL);
    end_performance_measurement(&medium_perf);
    ASSERT_TRUE(medium_result->success);
    double medium_time = get_cpu_time_ms(&medium_perf);

    // Test complex pattern compilation time
    ASTNode *complex_ast = parse_source(complex_source);
    PerformanceMeasurement complex_perf = start_performance_measurement();
    PatternCompilationResult *complex_result = compile_patterns(complex_ast, OPTIMIZATION_FULL);
    end_performance_measurement(&complex_perf);
    ASSERT_TRUE(complex_result->success);
    double complex_time = get_cpu_time_ms(&complex_perf);

    // Verify compilation times are reasonable
    ASSERT_LT(simple_time, 5.0);    // Simple patterns should compile in under 5ms
    ASSERT_LT(medium_time, 20.0);   // Medium patterns should compile in under 20ms
    ASSERT_LT(complex_time, 100.0); // Complex patterns should compile in under 100ms

    // Verify compilation time scales reasonably
    ASSERT_LT(medium_time, simple_time * 10);  // Medium shouldn't be 10x slower than simple
    ASSERT_LT(complex_time, medium_time * 10); // Complex shouldn't be 10x slower than medium

    cleanup_pattern_compilation_result(simple_result);
    cleanup_pattern_compilation_result(medium_result);
    cleanup_pattern_compilation_result(complex_result);
    cleanup_ast(simple_ast);
    cleanup_ast(medium_ast);
    cleanup_ast(complex_ast);

    TEST_END();
}

/**
 * Test optimization level configuration
 */
static bool test_optimization_levels(void) {
    TEST_START("Optimization level configuration");

    const char *source = "pub enum Color { Red, Green, Blue, Yellow, Orange }\n"
                         "pub fn color_name(c: Color) -> String {\n"
                         "    match c {\n"
                         "        Color::Red => \"red\",\n"
                         "        Color::Green => \"green\",\n"
                         "        Color::Blue => \"blue\",\n"
                         "        Color::Yellow => \"yellow\",\n"
                         "        Color::Orange => \"orange\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    // Test different optimization levels
    PatternCompilationResult *none = compile_patterns(ast, OPTIMIZATION_NONE);
    PatternCompilationResult *basic = compile_patterns(ast, OPTIMIZATION_BASIC);
    PatternCompilationResult *full = compile_patterns(ast, OPTIMIZATION_FULL);

    ASSERT_TRUE(none->success);
    ASSERT_TRUE(basic->success);
    ASSERT_TRUE(full->success);

    // Verify optimization progression
    ASSERT_GE(basic->instruction_count, none->instruction_count);
    ASSERT_GE(full->instruction_count, basic->instruction_count);

    ASSERT_FALSE(none->uses_jump_table);
    ASSERT_TRUE(basic->uses_jump_table || full->uses_jump_table);
    ASSERT_TRUE(full->uses_jump_table);

    cleanup_pattern_compilation_result(none);
    cleanup_pattern_compilation_result(basic);
    cleanup_pattern_compilation_result(full);
    cleanup_ast(ast);

    TEST_END();
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("PATTERN COMPILATION OPTIMIZATION TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.3: Advanced Pattern Matching\n\n");

    setup_optimization_test_environment();

    bool all_tests_passed = true;

    // Pattern compilation optimization tests
    if (!test_basic_pattern_optimization())
        all_tests_passed = false;
    if (!test_decision_tree_optimization())
        all_tests_passed = false;
    if (!test_compilation_time_performance())
        all_tests_passed = false;
    if (!test_optimization_levels())
        all_tests_passed = false;

    cleanup_optimization_test_environment();

    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL PATTERN COMPILATION OPTIMIZATION TESTS PASSED!\n");
        printf(
            "📊 Pattern Matching Expansion Plan: Compilation Optimization Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME PATTERN COMPILATION OPTIMIZATION TESTS FAILED!\n");
        return 1;
    }
}
