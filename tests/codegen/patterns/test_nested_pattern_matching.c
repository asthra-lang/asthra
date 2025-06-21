/**
 * Asthra Programming Language Compiler
 * Nested Pattern Matching Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test nested and complex pattern matching functionality
 */

#include "test_pattern_matching_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_nested_pattern_matching", __FILE__, __LINE__, "Test nested and complex pattern matching", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

AsthraTestResult test_nested_pattern_matching(AsthraTestContext* context) {
    PatternMatchingContext ctx;
    init_pattern_matching_context(&ctx);
    
    // Create nested variant values
    VariantValue* inner_int = create_variant_int(&ctx, 42);
    VariantValue* some_val = create_variant_option_some(&ctx, inner_int);
    VariantValue* none_val = create_variant_option_none(&ctx);
    
    VariantValue* success_str = create_variant_string(&ctx, "success");
    VariantValue* error_str = create_variant_string(&ctx, "error");
    VariantValue* ok_result = create_variant_result_ok(&ctx, success_str);
    VariantValue* error_result = create_variant_result_error(&ctx, error_str);
    
    // Create nested patterns
    Pattern* int_pattern = create_pattern_literal_int(&ctx, 42);
    Pattern* var_pattern = create_pattern_variable(&ctx, "x");
    Pattern* some_42_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_SOME, "Some", int_pattern);
    Pattern* some_var_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_SOME, "Some", var_pattern);
    Pattern* none_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_NONE, "None", NULL);
    
    // Test nested matching
    if (!match_pattern(some_42_pattern, some_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (match_pattern(some_42_pattern, none_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!match_pattern(some_var_pattern, some_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!match_pattern(none_pattern, none_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Create Result patterns
    Pattern* string_var = create_pattern_variable(&ctx, "s");
    Pattern* ok_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_RESULT_OK, "Ok", string_var);
    Pattern* error_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_RESULT_ERROR, "Error", string_var);
    
    if (!match_pattern(ok_pattern, ok_result)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (match_pattern(ok_pattern, error_result)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!match_pattern(error_pattern, error_result)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (match_pattern(error_pattern, ok_result)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test complex match expression with nested patterns
    MatchExpression* complex_match = create_match_expression(&ctx);
    if (!complex_match) {
        return ASTHRA_TEST_FAIL;
    }
    
    int action_counter = 0;
    ctx.match_context = &action_counter;
    
    add_match_case(complex_match, some_42_pattern, specific_action);
    add_match_case(complex_match, some_var_pattern, dummy_action);
    add_match_case(complex_match, none_pattern, dummy_action);
    
    // Test matching Some(42) - should hit specific action
    if (!execute_match(&ctx, complex_match, some_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (action_counter != 10 || ctx.executed_case != 0) {
        return ASTHRA_TEST_FAIL; // Should match first specific pattern
    }
    
    // Test matching None
    action_counter = 0;
    if (!execute_match(&ctx, complex_match, none_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (action_counter != 1 || ctx.executed_case != 2) {
        return ASTHRA_TEST_FAIL; // Should match None pattern
    }
    
    // Verify statistics
    if (complex_match->total_executions != 2 ||
        complex_match->successful_matches != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Nested Pattern Matching Tests ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    if (test_nested_pattern_matching(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Nested Pattern Matching: PASS\n");
        passed++;
    } else {
        printf("❌ Nested Pattern Matching: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
