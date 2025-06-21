/**
 * Asthra Programming Language Compiler
 * Pattern Guards Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test pattern guards and conditional matching functionality
 */

#include "test_pattern_matching_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_pattern_guards", __FILE__, __LINE__, "Test pattern guards and conditional matching", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

AsthraTestResult test_pattern_guards(AsthraTestContext* context) {
    PatternMatchingContext ctx;
    init_pattern_matching_context(&ctx);
    
    // Create test values
    VariantValue* pos_int = create_variant_int(&ctx, 9);  // Odd positive number for testing
    VariantValue* neg_int = create_variant_int(&ctx, -5);
    VariantValue* zero_int = create_variant_int(&ctx, 0);
    VariantValue* even_int = create_variant_int(&ctx, 8);
    VariantValue* large_int = create_variant_int(&ctx, 150);
    
    // Create patterns with guards
    Pattern* var_pattern = create_pattern_variable(&ctx, "x");
    Pattern* positive_guard_pattern = create_pattern_with_guard(&ctx, var_pattern, positive_guard);
    Pattern* even_guard_pattern = create_pattern_with_guard(&ctx, var_pattern, even_guard);
    Pattern* large_guard_pattern = create_pattern_with_guard(&ctx, var_pattern, large_number_guard);
    Pattern* wildcard = create_pattern_wildcard(&ctx);
    
    if (!var_pattern || !positive_guard_pattern || !even_guard_pattern || !large_guard_pattern || !wildcard) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test guard matching
    if (!match_pattern(positive_guard_pattern, pos_int)) {
        return ASTHRA_TEST_FAIL; // Positive number should match positive guard
    }
    
    if (match_pattern(positive_guard_pattern, neg_int)) {
        return ASTHRA_TEST_FAIL; // Negative number should not match positive guard
    }
    
    if (match_pattern(positive_guard_pattern, zero_int)) {
        return ASTHRA_TEST_FAIL; // Zero should not match positive guard
    }
    
    if (!match_pattern(even_guard_pattern, even_int)) {
        return ASTHRA_TEST_FAIL; // Even number should match even guard
    }
    
    if (match_pattern(even_guard_pattern, pos_int)) {
        return ASTHRA_TEST_FAIL; // Odd number should not match even guard
    }
    
    if (!match_pattern(large_guard_pattern, large_int)) {
        return ASTHRA_TEST_FAIL; // Large number should match large guard
    }
    
    if (match_pattern(large_guard_pattern, pos_int)) {
        return ASTHRA_TEST_FAIL; // Small number should not match large guard
    }
    
    // Test guard match expression with precedence
    MatchExpression* guard_match = create_match_expression(&ctx);
    if (!guard_match) {
        return ASTHRA_TEST_FAIL;
    }
    
    int action_counter = 0;
    ctx.match_context = &action_counter;
    
    add_match_case(guard_match, large_guard_pattern, specific_action);    // Large numbers (most specific)
    add_match_case(guard_match, positive_guard_pattern, dummy_action);    // Positive numbers
    add_match_case(guard_match, even_guard_pattern, dummy_action);        // Even numbers
    add_match_case(guard_match, wildcard, dummy_action);                  // Everything else
    
    // Test large positive number (should match large guard first)
    if (!execute_match(&ctx, guard_match, large_int)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (action_counter != 10 || ctx.executed_case != 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test small positive number (should match positive guard)
    action_counter = 0;
    if (!execute_match(&ctx, guard_match, pos_int)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (action_counter != 1 || ctx.executed_case != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test negative even number (should match even guard)
    VariantValue* neg_even = create_variant_int(&ctx, -4);
    action_counter = 0;
    
    if (!execute_match(&ctx, guard_match, neg_even)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (action_counter != 1 || ctx.executed_case != 2) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test negative odd number (should match wildcard)
    VariantValue* neg_odd = create_variant_int(&ctx, -3);
    action_counter = 0;
    
    if (!execute_match(&ctx, guard_match, neg_odd)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (action_counter != 1 || ctx.executed_case != 3) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify guard pattern IDs
    if (positive_guard_pattern->pattern_id == even_guard_pattern->pattern_id) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Pattern Guards Tests ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    if (test_pattern_guards(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Pattern Guards: PASS\n");
        passed++;
    } else {
        printf("❌ Pattern Guards: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
