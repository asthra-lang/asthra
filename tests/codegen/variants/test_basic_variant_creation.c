/**
 * Asthra Programming Language Compiler
 * Basic Variant Creation Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test basic variant type creation and primitive types
 */

#include "test_variant_types_common.h"

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_variant_creation", __FILE__, __LINE__, "Test basic variant type creation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL}
};

static AsthraTestResult test_basic_variant_creation(AsthraTestContext* context) {
    VariantTypeContext ctx;
    init_variant_type_context(&ctx);
    
    // Create basic variant types
    VariantValue* int_var = create_variant_int(&ctx, 42);
    VariantValue* float_var = create_variant_float(&ctx, 3.14159);
    VariantValue* string_var = create_variant_string(&ctx, "hello world");
    
    // Verify basic variant creation
    if (!int_var || !float_var || !string_var) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify integer variant
    if (int_var->tag != VARIANT_TAG_INT || 
        int_var->data.int_value != 42 ||
        int_var->size != sizeof(int) ||
        int_var->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify float variant
    if (float_var->tag != VARIANT_TAG_FLOAT || 
        float_var->data.float_value != 3.14159 ||
        float_var->size != sizeof(double) ||
        float_var->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify string variant
    if (string_var->tag != VARIANT_TAG_STRING || 
        strcmp(string_var->data.string_value, "hello world") != 0 ||
        string_var->size != strlen("hello world") ||
        string_var->variant_id == 0) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify unique IDs
    if (int_var->variant_id == float_var->variant_id ||
        float_var->variant_id == string_var->variant_id ||
        int_var->variant_id == string_var->variant_id) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify type classification
    if (!is_variant_primitive_type(int_var) ||
        !is_variant_primitive_type(float_var) ||
        !is_variant_primitive_type(string_var)) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (is_variant_option_type(int_var) || 
        is_variant_result_type(float_var)) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify context state
    if (ctx.variant_count != 3 ||
        ctx.int_variants != 1 ||
        ctx.float_variants != 1 ||
        ctx.string_variants != 1) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Verify memory tracking
    size_t expected_memory = sizeof(int) + sizeof(double) + strlen("hello world");
    if (ctx.total_memory_used != expected_memory) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    printf("=== Basic Variant Creation Tests ===\n");
    
    AsthraTestContext context = {0};
    int passed = 0, total = 0;
    
    // Run test
    if (test_basic_variant_creation(&context) == ASTHRA_TEST_PASS) {
        printf("✅ Basic Variant Creation: PASS\n");
        passed++;
    } else {
        printf("❌ Basic Variant Creation: FAIL\n");
    }
    total++;
    
    printf("\n=== Results ===\n");
    printf("Passed: %d/%d (%.1f%%)\n", passed, total, (passed * 100.0) / total);
    
    return (passed == total) ? 0 : 1;
} 
