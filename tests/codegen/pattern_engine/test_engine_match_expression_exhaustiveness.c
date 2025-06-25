/**
 * Asthra Programming Language Compiler
 * Pattern Engine Tests - Match Expression Exhaustiveness
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test match expression exhaustiveness checking and coverage validation
 */

#include "test_pattern_engine_common.h"

// Test metadata
static AsthraTestMetadata test_metadata = {"test_match_expression_exhaustiveness",
                                           __FILE__,
                                           __LINE__,
                                           "Test match expression exhaustiveness checking",
                                           ASTHRA_TEST_SEVERITY_MEDIUM,
                                           5000000000ULL,
                                           false,
                                           NULL};

static AsthraTestResult test_match_expression_exhaustiveness(AsthraTestContext *context) {
    PatternEngineContext ctx;
    init_pattern_engine_context(&ctx);

    // Test exhaustiveness checking with Option type
    MatchExpression *option_match = create_match_expression(&ctx);
    if (!option_match) {
        return ASTHRA_TEST_FAIL;
    }

    Pattern *var_pattern = create_pattern_variable(&ctx, "x");
    Pattern *some_pattern =
        create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_SOME, "Some", var_pattern);
    Pattern *none_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_NONE, "None", NULL);

    add_match_case(option_match, some_pattern, dummy_action);
    add_match_case(option_match, none_pattern, dummy_action);

    // Check that we covered the expected tags
    bool found_some = false, found_none = false;
    for (int i = 0; i < option_match->covered_count; i++) {
        if (option_match->covered_tags[i] == VARIANT_TAG_OPTION_SOME)
            found_some = true;
        if (option_match->covered_tags[i] == VARIANT_TAG_OPTION_NONE)
            found_none = true;
    }

    if (!found_some || !found_none) {
        return ASTHRA_TEST_FAIL;
    }

    // Test exhaustiveness with Result type
    MatchExpression *result_match = create_match_expression(&ctx);
    if (!result_match) {
        return ASTHRA_TEST_FAIL;
    }

    Pattern *string_var = create_pattern_variable(&ctx, "s");
    Pattern *ok_pattern = create_pattern_constructor(&ctx, VARIANT_TAG_RESULT_OK, "Ok", string_var);
    Pattern *error_pattern =
        create_pattern_constructor(&ctx, VARIANT_TAG_RESULT_ERROR, "Error", string_var);

    add_match_case(result_match, ok_pattern, dummy_action);
    add_match_case(result_match, error_pattern, dummy_action);

    // Verify Result coverage
    bool found_ok = false, found_error = false;
    for (int i = 0; i < result_match->covered_count; i++) {
        if (result_match->covered_tags[i] == VARIANT_TAG_RESULT_OK)
            found_ok = true;
        if (result_match->covered_tags[i] == VARIANT_TAG_RESULT_ERROR)
            found_error = true;
    }

    if (!found_ok || !found_error) {
        return ASTHRA_TEST_FAIL;
    }

    // Test that wildcard makes things exhaustive
    MatchExpression *wildcard_match = create_match_expression(&ctx);
    Pattern *wildcard = create_pattern_wildcard(&ctx);
    add_match_case(wildcard_match, wildcard, dummy_action);

    if (!wildcard_match->is_exhaustive) {
        return ASTHRA_TEST_FAIL;
    }

    // Test non-exhaustive match (only Some, no None)
    MatchExpression *incomplete_match = create_match_expression(&ctx);
    Pattern *incomplete_some =
        create_pattern_constructor(&ctx, VARIANT_TAG_OPTION_SOME, "Some", var_pattern);
    add_match_case(incomplete_match, incomplete_some, dummy_action);

    if (incomplete_match->is_exhaustive) {
        return ASTHRA_TEST_FAIL; // Should not be exhaustive
    }

    // Test execution and statistics
    VariantValue *test_some = create_variant_option_some(&ctx, create_variant_int(&ctx, 42));
    VariantValue *test_none = create_variant_option_none(&ctx);

    int action_counter = 0;
    ctx.match_context = &action_counter;

    // Execute multiple matches
    execute_match(&ctx, option_match, test_some);
    execute_match(&ctx, option_match, test_none);
    execute_match(&ctx, wildcard_match, test_some);

    // Verify statistics
    if (option_match->total_executions != 2 || option_match->successful_matches != 2 ||
        wildcard_match->total_executions != 1 || wildcard_match->catch_all_hits != 1) {
        return ASTHRA_TEST_FAIL;
    }

    if (ctx.total_matches_executed != 3) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Pattern Engine Tests - Match Expression Exhaustiveness ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run test
    if (test_match_expression_exhaustiveness(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Match Expression Exhaustiveness: PASS\n");
        passed++;
    } else {
        printf("❌ Match Expression Exhaustiveness: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
