/*
 * =============================================================================
 * JUMP TABLE OPTIMIZATION TESTS
 * =============================================================================
 * 
 * This file contains tests for jump table optimization techniques
 * including dense and sparse jump tables for enum pattern matching.
 * 
 * Part of Phase 3.3: Advanced Pattern Matching
 * 
 * Test Categories:
 * - Dense enum jump table optimization
 * - Sparse enum jump table optimization
 * - Jump table performance benchmarks
 * 
 * =============================================================================
 */

#include "test_pattern_common.h"
#include "test_optimization_common.h"

// =============================================================================
// JUMP TABLE OPTIMIZATION TESTS
// =============================================================================

/**
 * Test jump table generation for enum patterns
 */
static bool test_enum_jump_table_optimization(void) {
    TEST_START("Enum jump table optimization");
    
    const char* source = 
        "enum Color {\n"
        "    RED = 0,\n"
        "    GREEN = 1,\n"
        "    BLUE = 2,\n"
        "    YELLOW = 3,\n"
        "    ORANGE = 4,\n"
        "    PURPLE = 5,\n"
        "    PINK = 6,\n"
        "    BROWN = 7,\n"
        "    BLACK = 8,\n"
        "    WHITE = 9\n"
        "}\n"
        "\n"
        "fn color_to_hex(color: Color) -> String {\n"
        "    match color {\n"
        "        Color::RED => \"#FF0000\",\n"
        "        Color::GREEN => \"#00FF00\",\n"
        "        Color::BLUE => \"#0000FF\",\n"
        "        Color::YELLOW => \"#FFFF00\",\n"
        "        Color::ORANGE => \"#FFA500\",\n"
        "        Color::PURPLE => \"#800080\",\n"
        "        Color::PINK => \"#FFC0CB\",\n"
        "        Color::BROWN => \"#A52A2A\",\n"
        "        Color::BLACK => \"#000000\",\n"
        "        Color::WHITE => \"#FFFFFF\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // Verify jump table optimization
    ASSERT_TRUE(result->uses_jump_table);
    ASSERT_EQ(result->jump_table_size, 10);
    ASSERT_EQ(result->jump_table_density, 1.0); // Perfect density for consecutive enum values
    
    // Test performance with jump table
    EnumValue test_colors[] = {
        create_enum_value("Color", "RED", 0),
        create_enum_value("Color", "GREEN", 1),
        create_enum_value("Color", "BLUE", 2),
        create_enum_value("Color", "YELLOW", 3),
        create_enum_value("Color", "ORANGE", 4)
    };
    
    PerformanceMeasurement perf = start_performance_measurement();
    for (int i = 0; i < 50000; i++) {
        EnumValue color = test_colors[i % 5];
        String result_str = execute_pattern_match_with_value(result, &color);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&perf);
    
    double execution_time = get_cpu_time_ms(&perf);
    ASSERT_LT(execution_time, 20.0); // Should execute 50000 matches in under 20ms
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test sparse jump table optimization
 */
static bool test_sparse_jump_table_optimization(void) {
    TEST_START("Sparse jump table optimization");
    
    const char* source = 
        "enum HttpStatus {\n"
        "    OK = 200,\n"
        "    NOT_FOUND = 404,\n"
        "    SERVER_ERROR = 500,\n"
        "    BAD_REQUEST = 400,\n"
        "    UNAUTHORIZED = 401,\n"
        "    FORBIDDEN = 403\n"
        "}\n"
        "\n"
        "fn status_message(status: HttpStatus) -> String {\n"
        "    match status {\n"
        "        HttpStatus::OK => \"Success\",\n"
        "        HttpStatus::BAD_REQUEST => \"Bad Request\",\n"
        "        HttpStatus::UNAUTHORIZED => \"Unauthorized\",\n"
        "        HttpStatus::FORBIDDEN => \"Forbidden\",\n"
        "        HttpStatus::NOT_FOUND => \"Not Found\",\n"
        "        HttpStatus::SERVER_ERROR => \"Internal Server Error\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // For sparse enums, should use hash table or binary search instead of jump table
    ASSERT_FALSE(result->uses_jump_table); // Too sparse for jump table
    ASSERT_TRUE(result->uses_hash_table || result->uses_binary_search);
    
    if (result->uses_hash_table) {
        ASSERT_GT(result->hash_table_load_factor, 0.5);
        ASSERT_LT(result->hash_table_load_factor, 0.8);
    }
    
    // Test performance
    EnumValue test_status = create_enum_value("HttpStatus", "NOT_FOUND", 404);
    String result_str = execute_pattern_match_with_value(result, &test_status);
    ASSERT_STRING_EQ(result_str.data, "Not Found");
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test jump table density analysis
 */
static bool test_jump_table_density_analysis(void) {
    TEST_START("Jump table density analysis");
    
    // Test dense enum (should use jump table)
    const char* dense_source = 
        "enum DenseEnum { A = 1, B = 2, C = 3, D = 4, E = 5 }\n"
        "fn handle_dense(e: DenseEnum) -> i32 {\n"
        "    match e {\n"
        "        DenseEnum::A => 1,\n"
        "        DenseEnum::B => 2,\n"
        "        DenseEnum::C => 3,\n"
        "        DenseEnum::D => 4,\n"
        "        DenseEnum::E => 5\n"
        "    }\n"
        "}\n";
    
    ASTNode* dense_ast = parse_source(dense_source);
    PatternCompilationResult* dense_result = compile_patterns(dense_ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(dense_result->success);
    ASSERT_TRUE(dense_result->uses_jump_table);
    ASSERT_GT(dense_result->jump_table_density, 0.8); // High density
    
    // Test sparse enum (should not use jump table)
    const char* sparse_source = 
        "enum SparseEnum { X = 1, Y = 100, Z = 1000 }\n"
        "fn handle_sparse(e: SparseEnum) -> i32 {\n"
        "    match e {\n"
        "        SparseEnum::X => 1,\n"
        "        SparseEnum::Y => 100,\n"
        "        SparseEnum::Z => 1000\n"
        "    }\n"
        "}\n";
    
    ASTNode* sparse_ast = parse_source(sparse_source);
    PatternCompilationResult* sparse_result = compile_patterns(sparse_ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(sparse_result->success);
    ASSERT_FALSE(sparse_result->uses_jump_table); // Too sparse
    ASSERT_TRUE(sparse_result->uses_hash_table || sparse_result->uses_binary_search);
    
    cleanup_pattern_compilation_result(dense_result);
    cleanup_pattern_compilation_result(sparse_result);
    cleanup_ast(dense_ast);
    cleanup_ast(sparse_ast);
    
    TEST_END();
}

/**
 * Test jump table vs linear search performance
 */
static bool test_jump_table_vs_linear_performance(void) {
    TEST_START("Jump table vs linear search performance");
    
    const char* source = 
        "enum Operation {\n"
        "    ADD = 0, SUB = 1, MUL = 2, DIV = 3, MOD = 4,\n"
        "    AND = 5, OR = 6, XOR = 7, SHL = 8, SHR = 9\n"
        "}\n"
        "\n"
        "fn operation_name(op: Operation) -> String {\n"
        "    match op {\n"
        "        Operation::ADD => \"addition\",\n"
        "        Operation::SUB => \"subtraction\",\n"
        "        Operation::MUL => \"multiplication\",\n"
        "        Operation::DIV => \"division\",\n"
        "        Operation::MOD => \"modulo\",\n"
        "        Operation::AND => \"bitwise and\",\n"
        "        Operation::OR => \"bitwise or\",\n"
        "        Operation::XOR => \"bitwise xor\",\n"
        "        Operation::SHL => \"shift left\",\n"
        "        Operation::SHR => \"shift right\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    // Compile with jump table optimization
    PatternCompilationResult* jump_table_result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(jump_table_result->success);
    ASSERT_TRUE(jump_table_result->uses_jump_table);
    
    // Compile without jump table optimization (simulate linear search)
    PatternCompilationResult* linear_result = compile_patterns(ast, OPTIMIZATION_NONE);
    ASSERT_TRUE(linear_result->success);
    ASSERT_FALSE(linear_result->uses_jump_table);
    
    // Test operations
    EnumValue test_ops[] = {
        create_enum_value("Operation", "ADD", 0),
        create_enum_value("Operation", "MUL", 2),
        create_enum_value("Operation", "AND", 5),
        create_enum_value("Operation", "SHR", 9)
    };
    
    // Benchmark jump table performance
    PerformanceMeasurement jump_perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        EnumValue op = test_ops[i % 4];
        String result_str = execute_pattern_match_with_value(jump_table_result, &op);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&jump_perf);
    
    // Benchmark linear search performance
    PerformanceMeasurement linear_perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        EnumValue op = test_ops[i % 4];
        String result_str = execute_pattern_match_with_value(linear_result, &op);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&linear_perf);
    
    double jump_time = get_cpu_time_ms(&jump_perf);
    double linear_time = get_cpu_time_ms(&linear_perf);
    
    // Jump table should be significantly faster for dense enums
    ASSERT_LT(jump_time, linear_time * 0.7); // At least 30% faster
    
    cleanup_pattern_compilation_result(jump_table_result);
    cleanup_pattern_compilation_result(linear_result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test jump table boundary conditions
 */
static bool test_jump_table_boundary_conditions(void) {
    TEST_START("Jump table boundary conditions");
    
    const char* source = 
        "enum BoundaryEnum {\n"
        "    MIN = 0,\n"
        "    MID = 5,\n"
        "    MAX = 10\n"
        "}\n"
        "\n"
        "fn boundary_test(e: BoundaryEnum) -> String {\n"
        "    match e {\n"
        "        BoundaryEnum::MIN => \"minimum\",\n"
        "        BoundaryEnum::MID => \"middle\",\n"
        "        BoundaryEnum::MAX => \"maximum\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    PatternCompilationResult* result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);
    
    // Test boundary values
    EnumValue min_val = create_enum_value("BoundaryEnum", "MIN", 0);
    String result_str = execute_pattern_match_with_value(result, &min_val);
    ASSERT_STRING_EQ(result_str.data, "minimum");
    
    EnumValue mid_val = create_enum_value("BoundaryEnum", "MID", 5);
    result_str = execute_pattern_match_with_value(result, &mid_val);
    ASSERT_STRING_EQ(result_str.data, "middle");
    
    EnumValue max_val = create_enum_value("BoundaryEnum", "MAX", 10);
    result_str = execute_pattern_match_with_value(result, &max_val);
    ASSERT_STRING_EQ(result_str.data, "maximum");
    
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("JUMP TABLE OPTIMIZATION TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.3: Advanced Pattern Matching\n\n");
    
    setup_optimization_test_environment();
    
    bool all_tests_passed = true;
    
    // Jump table optimization tests
    if (!test_enum_jump_table_optimization()) all_tests_passed = false;
    if (!test_sparse_jump_table_optimization()) all_tests_passed = false;
    if (!test_jump_table_density_analysis()) all_tests_passed = false;
    if (!test_jump_table_vs_linear_performance()) all_tests_passed = false;
    if (!test_jump_table_boundary_conditions()) all_tests_passed = false;
    
    cleanup_optimization_test_environment();
    
    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL JUMP TABLE OPTIMIZATION TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Jump Table Optimization Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME JUMP TABLE OPTIMIZATION TESTS FAILED!\n");
        return 1;
    }
} 
