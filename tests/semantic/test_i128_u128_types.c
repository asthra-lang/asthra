/**
 * Asthra Programming Language
 * 128-bit Integer Types Tests
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Test i128 and u128 type recognition and semantic analysis
 */

#include "../framework/test_framework.h"
#include "../framework/semantic_test_utils.h"
#include "semantic_core.h"

// =============================================================================
// 128-BIT INTEGER TYPE TESTS
// =============================================================================

/**
 * Test i128 type recognition
 */
AsthraTestResult test_i128_type_recognition(AsthraTestContext *context) {
    SemanticAnalyzer *analyzer = semantic_analyzer_create("test_package");
    if (!asthra_test_assert_not_null(context, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test i128 type recognition
    TypeDescriptor *i128_type = semantic_get_builtin_type(analyzer, "i128");
    if (!asthra_test_assert_not_null(context, i128_type, "Failed to get i128 builtin type")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_string_eq(context, i128_type->name, "i128",
                                     "i128 type name should be 'i128'")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_type->category, (int)TYPE_PRIMITIVE,
                                  "i128 should be a primitive type")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_type->data.primitive.primitive_kind, (int)PRIMITIVE_I128,
                                  "i128 should have PRIMITIVE_I128 kind")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_type->size, 16,
                                  "i128 should be 16 bytes (128 bits)")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_type->alignment, 16,
                                  "i128 should have 16-byte alignment")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    semantic_analyzer_destroy(analyzer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test u128 type recognition
 */
AsthraTestResult test_u128_type_recognition(AsthraTestContext *context) {
    SemanticAnalyzer *analyzer = semantic_analyzer_create("test_package");
    if (!asthra_test_assert_not_null(context, analyzer, "Failed to create semantic analyzer")) {
        return ASTHRA_TEST_FAIL;
    }
    
    // Test u128 type recognition
    TypeDescriptor *u128_type = semantic_get_builtin_type(analyzer, "u128");
    if (!asthra_test_assert_not_null(context, u128_type, "Failed to get u128 builtin type")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_string_eq(context, u128_type->name, "u128",
                                     "u128 type name should be 'u128'")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_type->category, (int)TYPE_PRIMITIVE,
                                  "u128 should be a primitive type")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_type->data.primitive.primitive_kind, (int)PRIMITIVE_U128,
                                  "u128 should have PRIMITIVE_U128 kind")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_type->size, 16,
                                  "u128 should be 16 bytes (128 bits)")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_type->alignment, 16,
                                  "u128 should have 16-byte alignment")) {
        semantic_analyzer_destroy(analyzer);
        return ASTHRA_TEST_FAIL;
    }
    
    semantic_analyzer_destroy(analyzer);
    return ASTHRA_TEST_PASS;
}

/**
 * Test 128-bit type builtin definitions
 */
AsthraTestResult test_128bit_builtin_definitions(AsthraTestContext *context) {
    // Test the builtin type definitions directly
    TypeDescriptor *i128_type = &g_builtin_types[PRIMITIVE_I128];
    if (!asthra_test_assert_not_null(context, i128_type, "i128 builtin type should exist")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_type->size, 16,
                                  "i128 builtin should be 16 bytes")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_type->alignment, 16,
                                  "i128 builtin should have 16-byte alignment")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_string_eq(context, i128_type->name, "i128",
                                     "i128 builtin name should be 'i128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    TypeDescriptor *u128_type = &g_builtin_types[PRIMITIVE_U128];
    if (!asthra_test_assert_not_null(context, u128_type, "u128 builtin type should exist")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_type->size, 16,
                                  "u128 builtin should be 16 bytes")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_type->alignment, 16,
                                  "u128 builtin should have 16-byte alignment")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_string_eq(context, u128_type->name, "u128",
                                     "u128 builtin name should be 'u128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

/**
 * Test 128-bit TypeInfo mappings
 */
AsthraTestResult test_128bit_type_info_mapping(AsthraTestContext *context) {
    // Test TypeInfo mappings
    TypeInfo *i128_info = TYPE_INFO_I128;
    if (!asthra_test_assert_not_null(context, i128_info, "TYPE_INFO_I128 should exist")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_string_eq(context, i128_info->name, "i128",
                                     "TYPE_INFO_I128 name should be 'i128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)i128_info->size, 16,
                                  "TYPE_INFO_I128 should be 16 bytes")) {
        return ASTHRA_TEST_FAIL;
    }
    
    TypeInfo *u128_info = TYPE_INFO_U128;
    if (!asthra_test_assert_not_null(context, u128_info, "TYPE_INFO_U128 should exist")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_string_eq(context, u128_info->name, "u128",
                                     "TYPE_INFO_U128 name should be 'u128'")) {
        return ASTHRA_TEST_FAIL;
    }
    
    if (!asthra_test_assert_int_eq(context, (int)u128_info->size, 16,
                                  "TYPE_INFO_U128 should be 16 bytes")) {
        return ASTHRA_TEST_FAIL;
    }
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

/**
 * Main test function
 */
int main(void) {
    // Initialize test statistics
    AsthraTestStatistics *stats = asthra_test_statistics_create();
    if (!stats) {
        fprintf(stderr, "Failed to create test statistics\n");
        return 1;
    }

    // Define test cases
    const struct {
        AsthraTestFunction test_func;
        const char* test_name;
    } test_cases[] = {
        {test_i128_type_recognition, "i128 type recognition"},
        {test_u128_type_recognition, "u128 type recognition"},
        {test_128bit_builtin_definitions, "128-bit builtin definitions"},
        {test_128bit_type_info_mapping, "128-bit TypeInfo mapping"},
        {NULL, NULL}
    };

    printf("=== 128-bit Integer Types Tests ===\n\n");

    // Run tests
    for (int i = 0; test_cases[i].test_func != NULL; i++) {
        AsthraTestMetadata metadata = {
            .name = test_cases[i].test_name,
            .file = __FILE__,
            .line = __LINE__,
            .function = test_cases[i].test_name,
            .severity = ASTHRA_TEST_SEVERITY_MEDIUM,
            .timeout_ns = 30000000000ULL,
            .skip = false,
            .skip_reason = NULL
        };

        AsthraTestResult result = asthra_test_run_single(test_cases[i].test_func, &metadata, stats);
        printf("[%s] %s\n", 
               result == ASTHRA_TEST_PASS ? "PASS" : "FAIL", 
               test_cases[i].test_name);
    }

    // Print results
    printf("\nTest Results: %llu/%llu passed\n", 
           (unsigned long long)stats->tests_passed, (unsigned long long)stats->tests_run);

    bool success = (stats->tests_failed == 0);
    asthra_test_statistics_destroy(stats);
    return success ? 0 : 1;
}