/**
 * Asthra Programming Language Compiler
 * Option Type Variants Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test Option<T> type variants (Some/None) and unwrapping
 */

#include "test_variant_types_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_option_type_variants", __FILE__, __LINE__, "Test Option<T> type variants (Some/None)",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

static AsthraTestResult test_option_type_variants(AsthraTestContext *context) {
    VariantTypeContext ctx;
    init_variant_type_context(&ctx);

    // Create Option variants
    VariantValue *inner_int = create_variant_int(&ctx, 100);
    VariantValue *some_var = create_variant_option_some(&ctx, inner_int);
    VariantValue *none_var = create_variant_option_none(&ctx);

    if (!inner_int || !some_var || !none_var) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify Some variant
    if (some_var->tag != VARIANT_TAG_OPTION_SOME || some_var->data.option_value != inner_int ||
        some_var->size != sizeof(VariantValue *) || some_var->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify None variant
    if (none_var->tag != VARIANT_TAG_OPTION_NONE || none_var->data.option_value != NULL ||
        none_var->size != 0 || none_var->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify type classification
    if (!is_variant_option_type(some_var) || !is_variant_option_type(none_var)) {
        return ASTHRA_TEST_FAIL;
    }

    if (is_variant_option_type(inner_int) || is_variant_result_type(some_var)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test unwrapping
    VariantValue *unwrapped = unwrap_option_variant(some_var);
    if (!unwrapped || unwrapped != inner_int) {
        return ASTHRA_TEST_FAIL;
    }

    // Test unwrapping None (should fail)
    VariantValue *unwrapped_none = unwrap_option_variant(none_var);
    if (unwrapped_none != NULL) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify nested access
    if (unwrapped->tag != VARIANT_TAG_INT || unwrapped->data.int_value != 100) {
        return ASTHRA_TEST_FAIL;
    }

    // Create nested Option<Option<int>>
    VariantValue *double_nested = create_variant_option_some(&ctx, some_var);
    if (!double_nested || double_nested->tag != VARIANT_TAG_OPTION_SOME ||
        double_nested->data.option_value != some_var) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify context state
    if (ctx.variant_count != 4 || ctx.option_variants != 3 || ctx.int_variants != 1) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Option Type Variants Tests ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run test
    if (test_option_type_variants(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Option Type Variants: PASS\n");
        passed++;
    } else {
        printf("❌ Option Type Variants: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
