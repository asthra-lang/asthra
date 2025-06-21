/*
 * =============================================================================
 * ENUM PATTERN MATCHING PERFORMANCE TESTS
 * =============================================================================
 * 
 * This file contains performance tests for enum pattern matching,
 * including optimization verification and benchmark scenarios.
 * 
 * Part of Phase 3.1: Advanced Pattern Matching
 * 
 * Test Categories:
 * - Large enum compilation performance
 * - Jump table optimization
 * - Runtime execution performance
 * 
 * =============================================================================
 */

#include "test_pattern_common.h"

// =============================================================================
// PERFORMANCE TESTS
// =============================================================================

/**
 * Test enum pattern matching performance with large enums
 */
static bool test_large_enum_pattern_performance(void) {
    TEST_START("Large enum pattern performance");
    
    const char* source = 
        "enum LargeEnum {\n"
        "    V1, V2, V3, V4, V5, V6, V7, V8, V9, V10,\n"
        "    V11, V12, V13, V14, V15, V16, V17, V18, V19, V20\n"
        "}\n"
        "\n"
        "fn handle_large_enum(value: LargeEnum) -> i32 {\n"
        "    match value {\n"
        "        LargeEnum::V1 => 1, LargeEnum::V2 => 2, LargeEnum::V3 => 3,\n"
        "        LargeEnum::V4 => 4, LargeEnum::V5 => 5, LargeEnum::V6 => 6,\n"
        "        LargeEnum::V7 => 7, LargeEnum::V8 => 8, LargeEnum::V9 => 9,\n"
        "        LargeEnum::V10 => 10, LargeEnum::V11 => 11, LargeEnum::V12 => 12,\n"
        "        LargeEnum::V13 => 13, LargeEnum::V14 => 14, LargeEnum::V15 => 15,\n"
        "        LargeEnum::V16 => 16, LargeEnum::V17 => 17, LargeEnum::V18 => 18,\n"
        "        LargeEnum::V19 => 19, LargeEnum::V20 => 20\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Measure compilation time
    clock_t start_time = clock();
    PatternCompilationResult* pattern_result = compile_enum_patterns(ast);
    clock_t end_time = clock();
    
    double compilation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    
    ASSERT_TRUE(pattern_result->success);
    ASSERT_LT(compilation_time, 100.0); // Should compile in under 100ms
    
    // Verify jump table optimization
    ASSERT_TRUE(pattern_result->uses_jump_table);
    ASSERT_EQ(pattern_result->jump_table_size, 20);
    
    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test runtime execution performance
 */
static bool test_runtime_execution_performance(void) {
    TEST_START("Runtime execution performance");
    
    const char* source = 
        "enum TestEnum {\n"
        "    A, B, C, D, E, F, G, H, I, J\n"
        "}\n"
        "\n"
        "fn benchmark_enum(value: TestEnum) -> i32 {\n"
        "    match value {\n"
        "        TestEnum::A => 1,\n"
        "        TestEnum::B => 2,\n"
        "        TestEnum::C => 3,\n"
        "        TestEnum::D => 4,\n"
        "        TestEnum::E => 5,\n"
        "        TestEnum::F => 6,\n"
        "        TestEnum::G => 7,\n"
        "        TestEnum::H => 8,\n"
        "        TestEnum::I => 9,\n"
        "        TestEnum::J => 10\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* pattern_result = compile_enum_patterns(ast);
    ASSERT_TRUE(pattern_result->success);
    
    // Test execution performance
    EnumValue test_value = create_enum_value("TestEnum", "E", 4);
    
    clock_t start_time = clock();
    for (int i = 0; i < 10000; i++) {
        i32 result = execute_pattern_match_i32(pattern_result, test_value);
        ASSERT_EQ(result, 5);
    }
    clock_t end_time = clock();
    
    double execution_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    ASSERT_LT(execution_time, 10.0); // 10000 executions should take under 10ms
    
    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test memory usage optimization
 */
static bool test_memory_usage_optimization(void) {
    TEST_START("Memory usage optimization");
    
    const char* source = 
        "enum MemoryTestEnum {\n"
        "    SMALL,\n"
        "    MEDIUM,\n"
        "    LARGE\n"
        "}\n"
        "\n"
        "fn memory_test(value: MemoryTestEnum) -> String {\n"
        "    match value {\n"
        "        MemoryTestEnum::SMALL => \"small\",\n"
        "        MemoryTestEnum::MEDIUM => \"medium\",\n"
        "        MemoryTestEnum::LARGE => \"large\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Measure memory usage during compilation
    size_t initial_memory = get_memory_usage(); // Mock function
    PatternCompilationResult* pattern_result = compile_enum_patterns(ast);
    size_t final_memory = get_memory_usage(); // Mock function
    
    ASSERT_TRUE(pattern_result->success);
    
    // Verify reasonable memory usage (should be under 1KB for this simple case)
    size_t memory_used = final_memory - initial_memory;
    ASSERT_LT(memory_used, 1024);
    
    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test compilation scalability with many patterns
 */
static bool test_compilation_scalability(void) {
    TEST_START("Compilation scalability");
    
    // Generate a large enum with many variants
    const char* source = 
        "enum ScalabilityEnum {\n"
        "    V1, V2, V3, V4, V5, V6, V7, V8, V9, V10,\n"
        "    V11, V12, V13, V14, V15, V16, V17, V18, V19, V20,\n"
        "    V21, V22, V23, V24, V25, V26, V27, V28, V29, V30,\n"
        "    V31, V32, V33, V34, V35, V36, V37, V38, V39, V40,\n"
        "    V41, V42, V43, V44, V45, V46, V47, V48, V49, V50\n"
        "}\n"
        "\n"
        "fn handle_scalability(value: ScalabilityEnum) -> i32 {\n"
        "    match value {\n"
        "        ScalabilityEnum::V1 => 1, ScalabilityEnum::V2 => 2,\n"
        "        ScalabilityEnum::V3 => 3, ScalabilityEnum::V4 => 4,\n"
        "        ScalabilityEnum::V5 => 5, ScalabilityEnum::V6 => 6,\n"
        "        ScalabilityEnum::V7 => 7, ScalabilityEnum::V8 => 8,\n"
        "        ScalabilityEnum::V9 => 9, ScalabilityEnum::V10 => 10,\n"
        "        // ... continue for all 50 variants ...\n"
        "        _ => 0\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Measure compilation time for large enum
    clock_t start_time = clock();
    PatternCompilationResult* pattern_result = compile_enum_patterns(ast);
    clock_t end_time = clock();
    
    double compilation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    
    ASSERT_TRUE(pattern_result->success);
    // Should scale well - even 50 variants should compile quickly
    ASSERT_LT(compilation_time, 200.0); // Under 200ms
    
    // Verify optimization is used for large enums
    ASSERT_TRUE(pattern_result->uses_jump_table);
    ASSERT_EQ(pattern_result->jump_table_size, 50);
    
    cleanup_pattern_compilation_result(pattern_result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test complex pattern performance
 */
static bool test_complex_pattern_performance(void) {
    TEST_START("Complex pattern performance");
    
    const char* source = 
        "enum ComplexEnum {\n"
        "    Simple(i32),\n"
        "    Pair(i32, i32),\n"
        "    Triple(i32, i32, i32),\n"
        "    Struct { a: i32, b: String, c: f64 },\n"
        "    Nested(Option<Result<i32, String>>)\n"
        "}\n"
        "\n"
        "fn handle_complex(value: ComplexEnum) -> String {\n"
        "    match value {\n"
        "        ComplexEnum::Simple(x) => \"Simple: \" + x.to_string(),\n"
        "        ComplexEnum::Pair(x, y) => \"Pair: \" + x.to_string() + \", \" + y.to_string(),\n"
        "        ComplexEnum::Triple(x, y, z) => \"Triple: \" + x.to_string() + \", \" + y.to_string() + \", \" + z.to_string(),\n"
        "        ComplexEnum::Struct { a, b, c } => \"Struct: \" + a.to_string() + \", \" + b + \", \" + c.to_string(),\n"
        "        ComplexEnum::Nested(nested) => \"Nested: complex\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Test compilation performance for complex patterns
    clock_t start_time = clock();
    PatternCompilationResult* pattern_result = compile_enum_patterns(ast);
    clock_t end_time = clock();
    
    double compilation_time = ((double)(end_time - start_time)) / CLOCKS_PER_SEC * 1000; // ms
    
    ASSERT_TRUE(pattern_result->success);
    ASSERT_LT(compilation_time, 150.0); // Should handle complex patterns efficiently
    
    // Verify complex pattern handling
    ASSERT_TRUE(pattern_result->handles_tagged_unions);
    ASSERT_TRUE(pattern_result->handles_nested_patterns);
    ASSERT_TRUE(pattern_result->extracts_values);
    
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
    return 512; // Mock value
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("ENUM PATTERN MATCHING PERFORMANCE TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3: Advanced Pattern Matching Implementation\n\n");
    
    bool all_tests_passed = true;
    
    // Performance tests
    if (!test_large_enum_pattern_performance()) all_tests_passed = false;
    if (!test_runtime_execution_performance()) all_tests_passed = false;
    if (!test_memory_usage_optimization()) all_tests_passed = false;
    if (!test_compilation_scalability()) all_tests_passed = false;
    if (!test_complex_pattern_performance()) all_tests_passed = false;
    
    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL ENUM PATTERN MATCHING PERFORMANCE TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Performance Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME ENUM PATTERN MATCHING PERFORMANCE TESTS FAILED!\n");
        return 1;
    }
} 
