/*
 * =============================================================================
 * BASIC GUARD EXPRESSION TESTS
 * =============================================================================
 * 
 * This file contains tests for fundamental guard expression functionality,
 * including simple numeric and string guards.
 * 
 * Part of Phase 3.2: Advanced Pattern Matching
 * 
 * Test Categories:
 * - Simple numeric guard expressions
 * - String guard expressions
 * - Basic guard compilation
 * 
 * =============================================================================
 */

#include "test_pattern_common.h"
#include "test_guard_common.h"

// =============================================================================
// BASIC GUARD EXPRESSION TESTS
// =============================================================================

/**
 * Test simple numeric guard expressions
 */
static bool test_simple_numeric_guards(void) {
    TEST_START("Simple numeric guard expressions");
    
    const char* source = 
        "fn classify_number(x: i32) -> String {\n"
        "    match x {\n"
        "        n if n < 0 => \"negative\",\n"
        "        n if n == 0 => \"zero\",\n"
        "        n if n < 10 => \"single digit\",\n"
        "        n if n < 100 => \"double digit\",\n"
        "        _ => \"large number\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    SemanticResult* result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);
    
    PatternCompilationResult* pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 4);
    
    // Test negative number
    i32 test_value = -5;
    String result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "negative");
    
    // Test zero
    test_value = 0;
    result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "zero");
    
    // Test single digit
    test_value = 7;
    result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "single digit");
    
    // Test double digit
    test_value = 42;
    result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "double digit");
    
    // Test large number
    test_value = 1000;
    result_str = execute_pattern_match_with_value(pattern_result, &test_value);
    ASSERT_STRING_EQ(result_str.data, "large number");
    
    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test string guard expressions
 */
static bool test_string_guards(void) {
    TEST_START("String guard expressions");
    
    const char* source = 
        "fn analyze_string(s: String) -> String {\n"
        "    match s {\n"
        "        text if text.length() == 0 => \"empty\",\n"
        "        text if text.length() == 1 => \"single character\",\n"
        "        text if text.starts_with(\"Hello\") => \"greeting\",\n"
        "        text if text.contains(\"@\") => \"email-like\",\n"
        "        text if text.length() > 100 => \"very long\",\n"
        "        _ => \"regular text\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    SemanticResult* result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);
    
    PatternCompilationResult* pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 5);
    
    // Test empty string
    String test_str = create_string("");
    String result_str = execute_pattern_match_with_value(pattern_result, &test_str);
    ASSERT_STRING_EQ(result_str.data, "empty");
    
    // Test greeting
    test_str = create_string("Hello world");
    result_str = execute_pattern_match_with_value(pattern_result, &test_str);
    ASSERT_STRING_EQ(result_str.data, "greeting");
    
    // Test email-like
    test_str = create_string("user@example.com");
    result_str = execute_pattern_match_with_value(pattern_result, &test_str);
    ASSERT_STRING_EQ(result_str.data, "email-like");
    
    cleanup_pattern_compilation_result(pattern_result);
    cleanup_semantic_result(result);
    cleanup_ast(ast);
    
    TEST_END();
}

/**
 * Test basic guard compilation features
 */
static bool test_basic_guard_compilation(void) {
    TEST_START("Basic guard compilation");
    
    const char* source = 
        "fn simple_guards(x: i32, y: i32) -> String {\n"
        "    match (x, y) {\n"
        "        (a, b) if a > b => \"first greater\",\n"
        "        (a, b) if a == b => \"equal\",\n"
        "        (a, b) if a < b => \"second greater\"\n"
        "    }\n"
        "}\n";
    
    ASTNode* ast = parse_source(source);
    ASSERT_NOT_NULL(ast);
    
    SemanticResult* result = analyze_semantics(ast);
    ASSERT_TRUE(result->success);
    
    PatternCompilationResult* pattern_result = compile_patterns_with_guards(ast);
    ASSERT_TRUE(pattern_result->success);
    ASSERT_EQ(pattern_result->guard_count, 3);
    
    // Verify basic guard compilation features
    // ASSERT_TRUE(pattern_result->has_guards);  // Field doesn't exist
    // ASSERT_TRUE(pattern_result->compiles_guard_expressions);  // Field doesn't exist
    
    // Test first greater
    Value test_tuple = create_tuple_value((Value[]){
        create_i32_value(10),
        create_i32_value(5)
    }, 2);
    String result_str = execute_pattern_match_with_value(pattern_result, test_tuple);
    ASSERT_STRING_EQ(result_str.data, "first greater");
    
    // Test equal
    test_tuple = create_tuple_value((Value[]){
        create_i32_value(7),
        create_i32_value(7)
    }, 2);
    result_str = execute_pattern_match_with_value(pattern_result, test_tuple);
    ASSERT_STRING_EQ(result_str.data, "equal");
    
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
    printf("BASIC GUARD EXPRESSION TESTS\n");
    printf("=============================================================================\n");
    printf("Part of Pattern Matching and Enum Testing Expansion Plan\n");
    printf("Phase 3.2: Advanced Pattern Matching\n\n");
    
    setup_guard_test_environment();
    
    bool all_tests_passed = true;
    
    // Basic guard expression tests
    if (!test_simple_numeric_guards()) all_tests_passed = false;
    if (!test_string_guards()) all_tests_passed = false;
    if (!test_basic_guard_compilation()) all_tests_passed = false;
    
    cleanup_guard_test_environment();
    
    printf("\n=============================================================================\n");
    if (all_tests_passed) {
        printf("✅ ALL BASIC GUARD EXPRESSION TESTS PASSED!\n");
        printf("📊 Pattern Matching Expansion Plan: Basic Guard Features Demonstrated\n");
        return 0;
    } else {
        printf("❌ SOME BASIC GUARD EXPRESSION TESTS FAILED!\n");
        return 1;
    }
} 
