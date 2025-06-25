/**
 * Asthra Programming Language Compiler
 * Result Type Variants Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test Result<T,E> type variants (Ok/Error) and unwrapping
 */

#include "test_variant_types_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_result_type_variants", __FILE__, __LINE__, "Test Result<T,E> type variants (Ok/Error)",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}};

static AsthraTestResult test_result_type_variants(AsthraTestContext *context) {
    VariantTypeContext ctx;
    init_variant_type_context(&ctx);

    // Create Result variants
    VariantValue *success_value = create_variant_string(&ctx, "operation succeeded");
    VariantValue *error_message = create_variant_string(&ctx, "operation failed");
    VariantValue *ok_result = create_variant_result_ok(&ctx, success_value);
    VariantValue *error_result = create_variant_result_error(&ctx, error_message);

    if (!success_value || !error_message || !ok_result || !error_result) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify Ok variant
    if (ok_result->tag != VARIANT_TAG_RESULT_OK ||
        ok_result->data.result.ok_value != success_value ||
        ok_result->data.result.error_value != NULL || ok_result->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify Error variant
    if (error_result->tag != VARIANT_TAG_RESULT_ERROR ||
        error_result->data.result.ok_value != NULL ||
        error_result->data.result.error_value != error_message || error_result->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify type classification
    if (!is_variant_result_type(ok_result) || !is_variant_result_type(error_result)) {
        return ASTHRA_TEST_FAIL;
    }

    if (is_variant_result_type(success_value) || is_variant_option_type(ok_result)) {
        return ASTHRA_TEST_FAIL;
    }

    // Test unwrapping Ok
    VariantValue *unwrapped_ok = unwrap_result_ok(ok_result);
    if (!unwrapped_ok || unwrapped_ok != success_value) {
        return ASTHRA_TEST_FAIL;
    }

    // Test unwrapping Error
    VariantValue *unwrapped_error = unwrap_result_error(error_result);
    if (!unwrapped_error || unwrapped_error != error_message) {
        return ASTHRA_TEST_FAIL;
    }

    // Test cross unwrapping (should fail)
    if (unwrap_result_ok(error_result) != NULL || unwrap_result_error(ok_result) != NULL) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify nested access
    if (unwrapped_ok->tag != VARIANT_TAG_STRING ||
        strcmp(unwrapped_ok->data.string_value, "operation succeeded") != 0) {
        return ASTHRA_TEST_FAIL;
    }

    if (unwrapped_error->tag != VARIANT_TAG_STRING ||
        strcmp(unwrapped_error->data.string_value, "operation failed") != 0) {
        return ASTHRA_TEST_FAIL;
    }

    // Create Result<Option<int>, String>
    VariantValue *inner_int = create_variant_int(&ctx, 42);
    VariantValue *some_int = create_variant_option_some(&ctx, inner_int);
    VariantValue *complex_ok = create_variant_result_ok(&ctx, some_int);

    if (!complex_ok || complex_ok->tag != VARIANT_TAG_RESULT_OK ||
        complex_ok->data.result.ok_value != some_int) {
        return ASTHRA_TEST_FAIL;
    }

    // Verify context state
    if (ctx.variant_count != 7 || ctx.result_variants != 3 || ctx.string_variants != 2 ||
        ctx.option_variants != 1 || ctx.int_variants != 1) {
        return ASTHRA_TEST_FAIL;
    }

    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Result Type Variants Tests ===\n");

    AsthraTestContext context = {0};
    int passed = 0, total = 0;

    // Run test
    if (test_result_type_variants(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Result Type Variants: PASS\n");
        passed++;
    } else {
        printf("❌ Result Type Variants: FAIL\n");
    }
    total++;

    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);

    return (passed == total) ? 0 : 1;
}
