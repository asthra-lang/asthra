/*
 * =============================================================================
 * MEMORY USAGE AND CACHE EFFICIENCY OPTIMIZATION TESTS
 * =============================================================================
 * 
 * This file contains tests for memory usage optimization and cache efficiency
 * in pattern matching compilation and execution.
 * 
 * Part of Phase 3.3: Advanced Pattern Matching
 * 
 * Test Categories:
 * - Memory usage optimization in compilation
 * - Cache efficiency optimization
 * - Memory layout optimization
 * 
 * =============================================================================
 */

#include "test_pattern_common.h"
#include "test_optimization_common.h"

// =============================================================================
// MEMORY USAGE OPTIMIZATION TESTS
// =============================================================================

/**
 * Test memory usage optimization in pattern compilation
 */
static bool test_memory_usage_optimization(void) {
    TEST_START("Memory usage optimization");
    
    const char* source = 
        "enum LargeEnum {\n"
        "    V1, V2, V3, V4, V5, V6, V7, V8, V9, V10,\n"
        "    V11, V12, V13, V14, V15, V16, V17, V18, V19, V20,\n"
        "    V21, V22, V23, V24, V25, V26, V27, V28, V29, V30,\n"
        "    V31, V32, V33, V34, V35, V36, V37, V38, V39, V40,\n"
        "    V41, V42, V43, V44, V45, V46, V47, V48, V49, V50\n"
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
        "        LargeEnum::V19 => 19, LargeEnum::V20 => 20, LargeEnum::V21 => 21,\n"
        "        LargeEnum::V22 => 22, LargeEnum::V23 => 23, LargeEnum::V24 => 24,\n"
        "        LargeEnum::V25 => 25, LargeEnum::V26 => 26, LargeEnum::V27 => 27,\n"
        "        LargeEnum::V28 => 28, LargeEnum::V29 => 29, LargeEnum::V30 => 30,\n"
        "        LargeEnum::V31 => 31, LargeEnum::V32 => 32, LargeEnum::V33 => 33,\n"
        "        LargeEnum::V34 => 34, LargeEnum::V35 => 35, LargeEnum::V36 => 36,\n"
        "        LargeEnum::V37 => 37, LargeEnum::V38 => 38, LargeEnum::V39 => 39,\n"
        "        LargeEnum::V40 => 40, LargeEnum::V41 => 41, LargeEnum::V42 => 42,\n"
        "        LargeEnum::V43 => 43, LargeEnum::V44 => 44, LargeEnum::V45 => 45,\n"
        "        LargeEnum::V46 => 46, LargeEnum::V47 => 47, LargeEnum::V48 => 48,\n"
        "        LargeEnum::V49 => 49, LargeEnum::V50 => 50\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Measure memory usage during compilation
    PerformanceMeasurement compilation_perf = start_performance_measurement();
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    end_performance_measurement(&compilation_perf);
    
    ASSERT_TRUE(result->success);
    
    // Verify memory optimization
    long memory_used = get_memory_usage_kb(&compilation_perf);
    ASSERT_LT(memory_used, 1024); // Should use less than 1MB for compilation
    
    // Verify runtime memory efficiency
    ASSERT_LT(result->runtime_memory_usage, 512); // Runtime structures should be under 512 bytes
    ASSERT_TRUE(result->uses_compact_representation);
    
    // Test that the optimized version still works correctly
    EnumValue test_value = create_enum_value("LargeEnum", "V25", 24);
    i32 result_val = execute_pattern_match_i32(result, &test_value);
    ASSERT_EQ(result_val, 25);
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test memory layout optimization for structs
 */
static bool test_memory_layout_optimization(void) {
    TEST_START("Memory layout optimization");
    
    const char* source = 
        "struct OptimizedStruct {\n"
        "    a: i8,\n"
        "    b: i64,\n"
        "    c: i8,\n"
        "    d: i32,\n"
        "    e: i8\n"
        "}\n"
        "\n"
        "fn process_struct(s: OptimizedStruct) -> String {\n"
        "    match s {\n"
        "        OptimizedStruct { a: 0, .. } => \"a is zero\",\n"
        "        OptimizedStruct { b: 0, .. } => \"b is zero\",\n"
        "        OptimizedStruct { c: 0, .. } => \"c is zero\",\n"
        "        OptimizedStruct { d: 0, .. } => \"d is zero\",\n"
        "        OptimizedStruct { e: 0, .. } => \"e is zero\",\n"
        "        _ => \"no zeros\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // Verify memory layout optimization
    ASSERT_TRUE(result->optimizes_field_access_order);
    ASSERT_TRUE(result->minimizes_memory_jumps);
    ASSERT_TRUE(result->uses_compact_representation);
    
    // Test with various struct configurations
    StructValue test_struct = create_struct_value((StructValue[]){
        {"a", create_i8_value(0)},
        {"b", create_i64_value(100)},
        {"c", create_i8_value(5)},
        {"d", create_i32_value(10)},
        {"e", create_i8_value(15)}
    }, 5);
    
    String result_str = execute_pattern_match_with_value(result, &test_struct);
    ASSERT_STRING_EQ(result_str.data, "a is zero");
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test memory allocation optimization
 */
static bool test_memory_allocation_optimization(void) {
    TEST_START("Memory allocation optimization");
    
    const char* source = 
        "enum NestedEnum {\n"
        "    Simple(i32),\n"
        "    Complex { data: Vec<String>, count: i32 },\n"
        "    Nested(NestedEnum)\n"
        "}\n"
        "\n"
        "fn handle_nested(e: NestedEnum) -> String {\n"
        "    match e {\n"
        "        NestedEnum::Simple(x) => \"simple: \" + x.to_string(),\n"
        "        NestedEnum::Complex { count, .. } => \"complex: \" + count.to_string(),\n"
        "        NestedEnum::Nested(inner) => \"nested\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Test memory allocation patterns
    PerformanceMeasurement alloc_perf = start_performance_measurement();
    
    for (int i = 0; i < 1000; i++) {
        PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
        ASSERT_TRUE(result->success);
        cleanup_pattern_compilation_result(result);
    }
    
    end_performance_measurement(&alloc_perf);
    
    long memory_per_compilation = get_memory_usage_kb(&alloc_perf) / 1000;
    ASSERT_LT(memory_per_compilation, 10); // Should use less than 10KB per compilation
    
    cleanup_ast(ast);
    
    TEST_END();
}

// =============================================================================
// CACHE EFFICIENCY TESTS
// =============================================================================

/**
 * Test cache efficiency of pattern matching
 */
static bool test_cache_efficiency(void) {
    TEST_START("Cache efficiency optimization");
    
    const char* source = 
        "struct Data {\n"
        "    id: i32,\n"
        "    category: i32,\n"
        "    priority: i32,\n"
        "    status: i32\n"
        "}\n"
        "\n"
        "fn process_data(data: Data) -> String {\n"
        "    match data {\n"
        "        Data { category: 1, priority: p, .. } if p > 5 => \"high priority category 1\",\n"
        "        Data { category: 1, .. } => \"category 1\",\n"
        "        Data { category: 2, status: 1, .. } => \"active category 2\",\n"
        "        Data { category: 2, .. } => \"category 2\",\n"
        "        Data { category: 3, priority: p, .. } if p < 3 => \"low priority category 3\",\n"
        "        Data { category: 3, .. } => \"category 3\",\n"
        "        _ => \"other\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // Verify cache-friendly optimizations
    ASSERT_TRUE(result->optimizes_field_access_order);
    ASSERT_TRUE(result->minimizes_memory_jumps);
    ASSERT_LT(result->cache_miss_estimate, 0.1); // Should have low cache miss rate
    
    // Test with many data items to stress cache
    StructValue test_data[] = {
        create_struct_value((StructValue[]){
            {"id", create_i32_value(1)},
            {"category", create_i32_value(1)},
            {"priority", create_i32_value(8)},
            {"status", create_i32_value(1)}
        }, 4),
        create_struct_value((StructValue[]){
            {"id", create_i32_value(2)},
            {"category", create_i32_value(2)},
            {"priority", create_i32_value(3)},
            {"status", create_i32_value(1)}
        }, 4),
        create_struct_value((StructValue[]){
            {"id", create_i32_value(3)},
            {"category", create_i32_value(3)},
            {"priority", create_i32_value(1)},
            {"status", create_i32_value(0)}
        }, 4)
    };
    
    PerformanceMeasurement perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        StructValue data = test_data[i % 3];
        String result_str = execute_pattern_match_with_value(result, &data);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&perf);
    
    double execution_time = get_cpu_time_ms(&perf);
    ASSERT_LT(execution_time, 30.0); // Should be cache-efficient
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test cache line optimization
 */
static bool test_cache_line_optimization(void) {
    TEST_START("Cache line optimization");
    
    const char* source = 
        "struct CacheLineStruct {\n"
        "    field1: i32,\n"
        "    field2: i32,\n"
        "    field3: i32,\n"
        "    field4: i32,\n"
        "    field5: i32,\n"
        "    field6: i32,\n"
        "    field7: i32,\n"
        "    field8: i32\n"
        "}\n"
        "\n"
        "fn cache_test(s: CacheLineStruct) -> i32 {\n"
        "    match s {\n"
        "        CacheLineStruct { field1: 1, field2: 2, .. } => 1,\n"
        "        CacheLineStruct { field3: 3, field4: 4, .. } => 2,\n"
        "        CacheLineStruct { field5: 5, field6: 6, .. } => 3,\n"
        "        CacheLineStruct { field7: 7, field8: 8, .. } => 4,\n"
        "        _ => 0\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // Verify cache line optimization
    ASSERT_TRUE(result->optimizes_field_access_order);
    ASSERT_LT(result->cache_miss_estimate, 0.15);
    
    // Test sequential field access pattern
    StructValue cache_struct = create_struct_value((StructValue[]){
        {"field1", create_i32_value(1)},
        {"field2", create_i32_value(2)},
        {"field3", create_i32_value(0)},
        {"field4", create_i32_value(0)},
        {"field5", create_i32_value(0)},
        {"field6", create_i32_value(0)},
        {"field7", create_i32_value(0)},
        {"field8", create_i32_value(0)}
    }, 8);
    
    i32 result_val = execute_pattern_match_i32(result, &cache_struct);
    ASSERT_EQ(result_val, 1);
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test memory prefetching optimization
 */
static bool test_memory_prefetching_optimization(void) {
    TEST_START("Memory prefetching optimization");
    
    const char* source = 
        "struct LargeData {\n"
        "    metadata: i32,\n"
        "    payload: [i32; 16],\n"
        "    checksum: i32\n"
        "}\n"
        "\n"
        "fn process_large_data(data: LargeData) -> String {\n"
        "    match data {\n"
        "        LargeData { metadata: 0, .. } => \"no metadata\",\n"
        "        LargeData { checksum: 0, .. } => \"no checksum\",\n"
        "        LargeData { payload, .. } if payload[0] > 100 => \"large payload\",\n"
        "        _ => \"regular data\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // Verify memory access optimization
    ASSERT_TRUE(result->minimizes_memory_jumps);
    ASSERT_TRUE(result->optimizes_field_access_order);
    
    // Create test data that would benefit from prefetching
    ArrayValue payload_array = create_array_value(16);
    for (int i = 0; i < 16; i++) {
        payload_array.elements[i] = create_i32_value(i * 10);
    }
    
    StructValue large_data = create_struct_value((StructValue[]){
        {"metadata", create_i32_value(1)},
        {"payload", &payload_array},
        {"checksum", create_i32_value(0)}
    }, 3);
    
    String result_str = execute_pattern_match_with_value(result, &large_data);
    ASSERT_STRING_EQ(result_str.data, "no checksum");
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Create an i8 value for testing
 */
static Value create_i8_value(int8_t value) {
    Value v;
    int8_t* ptr = malloc(sizeof(int8_t));
    *ptr = value;
    v.value = ptr;
    return v;
}

/**
 * Create an array value for testing
 */
static ArrayValue create_array_value(size_t size) {
    ArrayValue arr;
    arr.elements = malloc(sizeof(Value) * size);
    arr.size = size;
    return arr;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("MEMORY USAGE AND CACHE EFFICIENCY OPTIMIZATION TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.3: Advanced Pattern Matching\n\n");
    
    setup_optimization_test_environment();
    
    bool all_tests_passed = true;
    
    // Memory usage optimization tests
    if (!test_memory_usage_optimization()) all_tests_passed = false;
    if (!test_memory_layout_optimization()) all_tests_passed = false;
    if (!test_memory_allocation_optimization()) all_tests_passed = false;
    
    // Cache efficiency tests
    if (!test_cache_efficiency()) all_tests_passed = false;
    if (!test_cache_line_optimization()) all_tests_passed = false;
    if (!test_memory_prefetching_optimization()) all_tests_passed = false;
    
    cleanup_optimization_test_environment();
    
    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL MEMORY AND CACHE OPTIMIZATION TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Memory/Cache Optimization Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME MEMORY AND CACHE OPTIMIZATION TESTS FAILED!\n");
        return 1;
    }
} 
