/**
 * Asthra Programming Language Compiler
 * Nested Variant Structures Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test nested variant type structures and complex combinations
 */

#include "test_variant_types_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_nested_variant_structures", __FILE__, __LINE__, "Test nested variant type structures",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

static AsthraTestResult test_nested_variant_structures(AsthraTestContext *context) {
    VariantTypeContext ctx;
    init_variant_type_context(&ctx);

    // Create deeply nested structure: Result<Option<int>, Option<String>>
    VariantValue *inner_int = create_variant_int(&ctx, 123);
    VariantValue *some_int = create_variant_option_some(&ctx, inner_int);

    VariantValue *error_string = create_variant_string(&ctx, "nested error");
    VariantValue *some_error = create_variant_option_some(&ctx, error_string);

    VariantValue *success_result = create_variant_result_ok(&ctx, some_int);
    VariantValue *error_result = create_variant_result_error(&ctx, some_error);

    if (!success_result || !error_result) {
        return ASTHRA_TEST_FAIL;
    }

    // Test nested unwrapping for success case
    VariantValue *result_content = unwrap_result_ok(success_result);
    if (!result_content || !is_variant_option_type(result_content)) {
        return ASTHRA_TEST_FAIL;
    }

    VariantValue *option_content = unwrap_option_variant(result_content);
    if (!option_content || option_content->tag != VARIANT_TAG_INT ||
        option_content->data.int_value != 123) {
        return ASTHRA_TEST_FAIL;
    }

    // Test nested unwrapping for error case
    VariantValue *error_content = unwrap_result_error(error_result);
    if (!error_content || !is_variant_option_type(error_content)) {
        return ASTHRA_TEST_FAIL;
    }

    VariantValue *error_option_content = unwrap_option_variant(error_content);
    if (!error_option_content || error_option_content->tag != VARIANT_TAG_STRING ||
        strcmp(error_option_content->data.string_value, "nested error") != 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Create Option<Result<int, String>>
    VariantValue *simple_int = create_variant_int(&ctx, 456);
    VariantValue *simple_ok = create_variant_result_ok(&ctx, simple_int);
    VariantValue *option_result = create_variant_option_some(&ctx, simple_ok);

    // Create another Result<int, None> for the third expected result
    VariantValue *third_int = create_variant_int(&ctx, 789);
    VariantValue *third_ok = create_variant_result_ok(&ctx, third_int);

    // Test reverse nesting
    VariantValue *option_unwrapped = unwrap_option_variant(option_result);
    if (!option_unwrapped || !is_variant_result_type(option_unwrapped)) {
        return ASTHRA_TEST_FAIL;
    }

    VariantValue *result_unwrapped = unwrap_result_ok(option_unwrapped);
    if (!result_unwrapped || result_unwrapped->tag != VARIANT_TAG_INT ||
        result_unwrapped->data.int_value != 456) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify tag counting
    if (count_variants_by_tag(&ctx, VARIANT_TAG_INT) != 3 ||
        count_variants_by_tag(&ctx, VARIANT_TAG_STRING) != 1 ||
        count_variants_by_tag(&ctx, VARIANT_TAG_OPTION_SOME) != 3 ||
        count_variants_by_tag(&ctx, VARIANT_TAG_RESULT_OK) != 3 ||
        count_variants_by_tag(&ctx, VARIANT_TAG_RESULT_ERROR) != 1) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify all variants have unique IDs
    for (int i = 0; i < ctx.variant_count; i++) {
        for (int j = i + 1; j < ctx.variant_count; j++) {
            if (ctx.variants[i].variant_id == ctx.variants[j].variant_id) {
                return ASTHRA_TEST_FAIL;
            }
        }
    }

    // Verify context statistics
    if (ctx.variant_count != 11) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Nested Variant Structures Tests ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run test
    if (test_nested_variant_structures(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Nested Variant Structures: PASS\n");
        passed++;
    } else {
        printf("❌ Nested Variant Structures: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
