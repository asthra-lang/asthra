/**
 * Asthra Programming Language Compiler
 * Pattern Creation and Basic Matching Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test pattern creation and basic matching functionality
 */

#include "test_pattern_matching_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_pattern_creation_and_matching", __FILE__, __LINE__, "Test pattern creation and basic matching", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

AsthraTestResult test_pattern_creation_and_matching(AsthraTestContext* context) {
    PatternMatchingContext ctx;
    init_pattern_matching_context(&ctx);
    
    // Create test values
    VariantValue* int_val = create_variant_int(&ctx, 42);
    VariantValue* string_val = create_variant_string(&ctx, "test");
    VariantValue* none_val = create_variant_option_none(&ctx);
    
    // Create patterns
    Pattern* wildcard = create_pattern_wildcard(&ctx);
    Pattern* literal_42 = create_pattern_literal_int(&ctx, 42);
    Pattern* literal_100 = create_pattern_literal_int(&ctx, 100);
    Pattern* var_x = create_pattern_variable(&ctx, "x");
    Pattern* none_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_NONE, "None", NULL);
    
    // Verify pattern creation
    if (!wildcard || !literal_42 || !literal_100 || !var_x || !none_pattern) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify pattern IDs are unique
    if (wildcard->pattern_id == literal_42->pattern_id ||
        literal_42->pattern_id == var_x->pattern_id) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test wildcard matching
    if (!match_pattern(wildcard, int_val) || 
        !match_pattern(wildcard, string_val) || 
        !match_pattern(wildcard, none_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test literal matching
    if (!match_pattern(literal_42, int_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (match_pattern(literal_100, int_val)) {
        return ASTHRA_TEST_FAIL; // Should not match
    }
    
    if (match_pattern(literal_42, string_val)) {
        return ASTHRA_TEST_FAIL; // Should not match different type
    }
    
    // Test variable matching (should always match)
    if (!match_pattern(var_x, int_val) || 
        !match_pattern(var_x, string_val) || 
        !match_pattern(var_x, none_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test constructor matching
    if (!match_pattern(none_pattern, none_val)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (match_pattern(none_pattern, int_val)) {
        return ASTHRA_TEST_FAIL; // Should not match different constructor
    }
    
    // Verify statistics
    if (ctx.total_patterns_created != 5) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Pattern Creation and Basic Matching Tests ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    if (test_pattern_creation_and_matching(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Pattern Creation and Matching: PASS\n");
        passed++;
    } else {
        printf("❌ Pattern Creation and Matching: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
