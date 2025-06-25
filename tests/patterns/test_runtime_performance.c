/*
 * =============================================================================
 * RUNTIME PERFORMANCE ANALYSIS AND PROFILING TESTS
 * =============================================================================
 *
 * This file contains tests for runtime performance analysis, profiling,
 * and benchmarking of pattern matching execution.
 *
 * Part of Phase 3.3: Advanced Pattern Matching
 *
 * Test Categories:
 * - Runtime performance analysis
 * - Pattern execution profiling
 * - Performance benchmarking
 *
 * =============================================================================
 */

#include "test_optimization_common.h"
#include "test_pattern_common.h"

// =============================================================================
// RUNTIME PERFORMANCE ANALYSIS TESTS
// =============================================================================

/**
 * Test runtime performance analysis and profiling
 */
static bool test_runtime_performance_analysis(void) {
    TEST_START("Runtime performance analysis");

    const char *source = "enum Operation {\n"
                         "    Add(i32, i32),\n"
                         "    Subtract(i32, i32),\n"
                         "    Multiply(i32, i32),\n"
                         "    Divide(i32, i32),\n"
                         "    Power(i32, i32)\n"
                         "}\n"
                         "\n"
                         "fn calculate(op: Operation) -> i32 {\n"
                         "    match op {\n"
                         "        Operation::Add(a, b) => a + b,\n"
                         "        Operation::Subtract(a, b) => a - b,\n"
                         "        Operation::Multiply(a, b) => a * b,\n"
                         "        Operation::Divide(a, b) if b != 0 => a / b,\n"
                         "        Operation::Divide(_, _) => 0,\n"
                         "        Operation::Power(a, b) if b >= 0 => pow(a, b),\n"
                         "        Operation::Power(_, _) => 0\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);

    // Enable performance profiling
    enable_pattern_profiling(result);

    // Test various operations
    TaggedUnionValue operations[] = {
        create_tagged_union_value(
            "Operation", "Add",
            create_tuple_value((Value[]){create_i32_value(10), create_i32_value(5)}, 2)),
        create_tagged_union_value(
            "Operation", "Subtract",
            create_tuple_value((Value[]){create_i32_value(10), create_i32_value(3)}, 2)),
        create_tagged_union_value(
            "Operation", "Multiply",
            create_tuple_value((Value[]){create_i32_value(4), create_i32_value(7)}, 2)),
        create_tagged_union_value(
            "Operation", "Divide",
            create_tuple_value((Value[]){create_i32_value(20), create_i32_value(4)}, 2)),
        create_tagged_union_value(
            "Operation", "Power",
            create_tuple_value((Value[]){create_i32_value(2), create_i32_value(3)}, 2))};

    // Run performance test
    PerformanceMeasurement perf = start_performance_measurement();
    for (int i = 0; i < 50000; i++) {
        TaggedUnionValue op = operations[i % 5];
        i32 result_val = execute_pattern_match_i32(result, &op);
        ASSERT_GT(result_val, 0); // All operations should return positive results
    }
    end_performance_measurement(&perf);

    // Analyze performance profile
    PatternPerformanceProfile *profile = get_pattern_performance_profile(result);
    ASSERT_NOT_NULL(profile);

    // Verify performance characteristics
    ASSERT_LT(profile->average_execution_time_ns, 1000); // Should average under 1 microsecond
    ASSERT_GT(profile->cache_hit_rate, 0.95);            // Should have high cache hit rate
    ASSERT_LT(profile->branch_misprediction_rate, 0.05); // Should have low branch misprediction

    // Verify pattern frequency analysis
    ASSERT_EQ(profile->pattern_hit_counts[0], 10000); // Each pattern hit 10000 times
    ASSERT_EQ(profile->pattern_hit_counts[1], 10000);
    ASSERT_EQ(profile->pattern_hit_counts[2], 10000);
    ASSERT_EQ(profile->pattern_hit_counts[3], 10000);
    ASSERT_EQ(profile->pattern_hit_counts[4], 10000);

    double total_time = get_cpu_time_ms(&perf);
    ASSERT_LT(total_time, 50.0); // 50000 operations should complete in under 50ms

    cleanup_pattern_performance_profile(profile);
    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test pattern matching throughput
 */
static bool test_pattern_matching_throughput(void) {
    TEST_START("Pattern matching throughput");

    const char *source =
        "enum MessageType {\n"
        "    Text(String),\n"
        "    Image { url: String, width: i32, height: i32 },\n"
        "    Video { url: String, duration: f64 },\n"
        "    Audio { url: String, length: f64 },\n"
        "    File { name: String, size: i64 }\n"
        "}\n"
        "\n"
        "fn process_message(msg: MessageType) -> String {\n"
        "    match msg {\n"
        "        MessageType::Text(content) => \"text: \" + content,\n"
        "        MessageType::Image { width, height, .. } => \"image: \" + width.to_string() + "
        "\"x\" + height.to_string(),\n"
        "        MessageType::Video { duration, .. } => \"video: \" + duration.to_string() + "
        "\"s\",\n"
        "        MessageType::Audio { length, .. } => \"audio: \" + length.to_string() + \"s\",\n"
        "        MessageType::File { size, .. } => \"file: \" + size.to_string() + \" bytes\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);

    // Test different message types
    TaggedUnionValue messages[] = {
        create_tagged_union_value("MessageType", "Text", create_string_value("Hello World")),
        create_tagged_union_value(
            "MessageType", "Image",
            create_struct_value((StructValue[]){{"url", create_string_value("image.jpg")},
                                                {"width", create_i32_value(1920)},
                                                {"height", create_i32_value(1080)}},
                                3)),
        create_tagged_union_value(
            "MessageType", "Video",
            create_struct_value((StructValue[]){{"url", create_string_value("video.mp4")},
                                                {"duration", create_f64_value(120.5)}},
                                2)),
        create_tagged_union_value(
            "MessageType", "Audio",
            create_struct_value((StructValue[]){{"url", create_string_value("audio.mp3")},
                                                {"length", create_f64_value(180.0)}},
                                2)),
        create_tagged_union_value(
            "MessageType", "File",
            create_struct_value((StructValue[]){{"name", create_string_value("document.pdf")},
                                                {"size", create_i64_value(1024768)}},
                                2))};

    const int iterations = 1000000; // 1 million iterations

    PerformanceMeasurement throughput_perf = start_performance_measurement();
    for (int i = 0; i < iterations; i++) {
        TaggedUnionValue msg = messages[i % 5];
        String result_str = execute_pattern_match_with_value(result, &msg);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&throughput_perf);

    double total_time_sec = get_cpu_time_ms(&throughput_perf) / 1000.0;
    double throughput = iterations / total_time_sec;

    // Should achieve at least 100K pattern matches per second
    ASSERT_GT(throughput, 100000.0);

    printf("    Pattern matching throughput: %.0f matches/second\n", throughput);

    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test latency characteristics
 */
static bool test_latency_characteristics(void) {
    TEST_START("Latency characteristics");

    const char *source = "enum Priority {\n"
                         "    Low = 1,\n"
                         "    Medium = 2,\n"
                         "    High = 3,\n"
                         "    Critical = 4\n"
                         "}\n"
                         "\n"
                         "fn get_priority_name(p: Priority) -> String {\n"
                         "    match p {\n"
                         "        Priority::Low => \"low\",\n"
                         "        Priority::Medium => \"medium\",\n"
                         "        Priority::High => \"high\",\n"
                         "        Priority::Critical => \"critical\"\n"
                         "    }\n"
                         "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);

    // Test latency for individual pattern matches
    EnumValue priorities[] = {
        create_enum_value("Priority", "Low", 1), create_enum_value("Priority", "Medium", 2),
        create_enum_value("Priority", "High", 3), create_enum_value("Priority", "Critical", 4)};

    double total_latency = 0.0;
    const int test_runs = 10000;

    for (int i = 0; i < test_runs; i++) {
        EnumValue priority = priorities[i % 4];

        clock_t start = clock();
        String result_str = execute_pattern_match_with_value(result, &priority);
        clock_t end = clock();

        ASSERT_NOT_NULL(result_str.data);

        double latency_ns = ((double)(end - start)) / CLOCKS_PER_SEC * 1000000000.0;
        total_latency += latency_ns;
    }

    double average_latency = total_latency / test_runs;

    // Average latency should be under 500 nanoseconds
    ASSERT_LT(average_latency, 500.0);

    printf("    Average pattern match latency: %.2f nanoseconds\n", average_latency);

    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);

    TEST_END();
}

/**
 * Test performance scalability
 */
static bool test_performance_scalability(void) {
    TEST_START("Performance scalability");

    // Test with different pattern complexity levels
    const char *simple_source = "enum Simple { A, B, C }\n"
                                "fn simple(s: Simple) -> i32 {\n"
                                "    match s {\n"
                                "        Simple::A => 1,\n"
                                "        Simple::B => 2,\n"
                                "        Simple::C => 3\n"
                                "    }\n"
                                "}\n";

    const char *complex_source =
        "enum Complex {\n"
        "    A(i32, String),\n"
        "    B { x: i32, y: i32, z: String },\n"
        "    C(Complex),\n"
        "    D { nested: Complex, count: i32 },\n"
        "    E(i32, i32, i32, String, f64)\n"
        "}\n"
        "\n"
        "fn complex(c: Complex) -> String {\n"
        "    match c {\n"
        "        Complex::A(x, s) => \"A: \" + x.to_string() + \" \" + s,\n"
        "        Complex::B { x, y, .. } => \"B: \" + x.to_string() + \",\" + y.to_string(),\n"
        "        Complex::C(inner) => \"C: nested\",\n"
        "        Complex::D { count, .. } => \"D: \" + count.to_string(),\n"
        "        Complex::E(a, b, c, s, f) => \"E: complex\"\n"
        "    }\n"
        "}\n";

    // Test simple patterns
    ASTNode *simple_ast = parse_source(simple_source);
    PatternCompilationResult *simple_result = compile_patterns(simple_ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(simple_result->success);

    EnumValue simple_values[] = {create_enum_value("Simple", "A", 0),
                                 create_enum_value("Simple", "B", 1),
                                 create_enum_value("Simple", "C", 2)};

    PerformanceMeasurement simple_perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        EnumValue val = simple_values[i % 3];
        i32 result_val = execute_pattern_match_i32(simple_result, &val);
        ASSERT_GT(result_val, 0);
    }
    end_performance_measurement(&simple_perf);

    // Test complex patterns
    ASTNode *complex_ast = parse_source(complex_source);
    PatternCompilationResult *complex_result = compile_patterns(complex_ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(complex_result->success);

    TaggedUnionValue complex_values[] = {
        create_tagged_union_value(
            "Complex", "A",
            create_tuple_value((Value[]){create_i32_value(42), create_string_value("test")}, 2)),
        create_tagged_union_value(
            "Complex", "B",
            create_struct_value((StructValue[]){{"x", create_i32_value(10)},
                                                {"y", create_i32_value(20)},
                                                {"z", create_string_value("data")}},
                                3))};

    PerformanceMeasurement complex_perf = start_performance_measurement();
    for (int i = 0; i < 100000; i++) {
        TaggedUnionValue val = complex_values[i % 2];
        String result_str = execute_pattern_match_with_value(complex_result, &val);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&complex_perf);

    double simple_time = get_cpu_time_ms(&simple_perf);
    double complex_time = get_cpu_time_ms(&complex_perf);

    // Complex patterns should not be more than 5x slower than simple patterns
    ASSERT_LT(complex_time, simple_time * 5.0);

    printf("    Simple patterns: %.2f ms\n", simple_time);
    printf("    Complex patterns: %.2f ms\n", complex_time);
    printf("    Complexity overhead: %.2fx\n", complex_time / simple_time);

    cleanup_pattern_compilation_result(simple_result);
    cleanup_pattern_compilation_result(complex_result);
    cleanup_ast(simple_ast);
    cleanup_ast(complex_ast);

    TEST_END();
}

/**
 * Test memory usage during execution
 */
static bool test_execution_memory_usage(void) {
    TEST_START("Execution memory usage");

    const char *source =
        "enum MemoryTest {\n"
        "    Small(i32),\n"
        "    Medium(String, i32),\n"
        "    Large(String, i32, f64, Vec<i32>),\n"
        "    Huge { data: Vec<String>, metadata: Vec<i32>, flags: Vec<bool> }\n"
        "}\n"
        "\n"
        "fn memory_test(m: MemoryTest) -> String {\n"
        "    match m {\n"
        "        MemoryTest::Small(x) => \"small: \" + x.to_string(),\n"
        "        MemoryTest::Medium(s, x) => \"medium: \" + s + \" \" + x.to_string(),\n"
        "        MemoryTest::Large(s, x, f, _) => \"large: \" + s,\n"
        "        MemoryTest::Huge { .. } => \"huge\"\n"
        "    }\n"
        "}\n";

    ASTNode *ast = parse_source(source);
    ASSERT_NOT_NULL(ast);

    PatternCompilationResult *result = compile_patterns(ast, OPTIMIZATION_FULL);
    ASSERT_TRUE(result->success);

    // Test memory usage during pattern matching
    TaggedUnionValue small_val =
        create_tagged_union_value("MemoryTest", "Small", create_i32_value(42));

    PerformanceMeasurement memory_perf = start_performance_measurement();
    for (int i = 0; i < 10000; i++) {
        String result_str = execute_pattern_match_with_value(result, &small_val);
        ASSERT_NOT_NULL(result_str.data);
    }
    end_performance_measurement(&memory_perf);

    long memory_used = get_memory_usage_kb(&memory_perf);

    // Memory usage should be minimal for simple pattern matches
    ASSERT_LT(memory_used, 100); // Should use less than 100KB

    printf("    Memory used during execution: %ld KB\n", memory_used);

    cleanup_pattern_compilation_result(result);
    cleanup_ast(ast);

    TEST_END();
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Create a tuple value for testing
 */
static TupleValue create_tuple_value(Value *values, size_t count) {
    TupleValue tuple;
    tuple.values = malloc(sizeof(Value) * count);
    tuple.count = count;
    for (size_t i = 0; i < count; i++) {
        tuple.values[i] = values[i];
    }
    return tuple;
}

/**
 * Create a tagged union value for testing
 */
static TaggedUnionValue create_tagged_union_value(const char *type_name, const char *variant,
                                                  Value value) {
    TaggedUnionValue tagged_union;
    tagged_union.type_name = strdup(type_name);
    tagged_union.variant = strdup(variant);
    tagged_union.value = value;
    return tagged_union;
}

/**
 * Create an f64 value for testing
 */
static Value create_f64_value(double value) {
    Value v;
    double *ptr = malloc(sizeof(double));
    *ptr = value;
    v.value = ptr;
    return v;
}

/**
 * Create an i64 value for testing
 */
static Value create_i64_value(int64_t value) {
    Value v;
    int64_t *ptr = malloc(sizeof(int64_t));
    *ptr = value;
    v.value = ptr;
    return v;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    printf("=============================================================================\n");
    printf("RUNTIME PERFORMANCE ANALYSIS AND PROFILING TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.3: Advanced Pattern Matching\n\n");

    setup_optimization_test_environment();

    bool all_tests_passed = true;

    // Runtime performance analysis tests
    if (!test_runtime_performance_analysis())
        all_tests_passed = false;
    if (!test_pattern_matching_throughput())
        all_tests_passed = false;
    if (!test_latency_characteristics())
        all_tests_passed = false;
    if (!test_performance_scalability())
        all_tests_passed = false;
    if (!test_execution_memory_usage())
        all_tests_passed = false;

    cleanup_optimization_test_environment();

    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL RUNTIME PERFORMANCE TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Runtime Performance Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME RUNTIME PERFORMANCE TESTS FAILED!\n");
        return 1;
    }
}
