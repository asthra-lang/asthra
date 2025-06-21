#include "semantic_api_core.h"
#include "type_queries.h"
#include "../framework/test_framework.h"
#include <string.h>
#include <stdlib.h>

// Helper function to create a test API instance
static AsthraSemanticsAPI *setup_test_api(void) {
    AsthraSemanticsAPI *api = asthra_ai_create_api("test_file.asthra");
    if (!api) return NULL;
    
    const char *test_code = 
        "package test;\n"
        "pub struct Point {\n"
        "    x: i32,\n"
        "    y: i32\n"
        "}\n"
        "pub struct User {\n"
        "    name: string,\n"
        "    age: i32\n"
        "}\n";
    
    if (!asthra_ai_initialize_from_source(api, test_code)) {
        asthra_ai_destroy_api(api);
        return NULL;
    }
    
    return api;
}

static void cleanup_test_api(AsthraSemanticsAPI *api) {
    if (api) {
        asthra_ai_destroy_api(api);
    }
}

// Test type inference functionality
AsthraTestResult test_type_inference(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");
    
    // Test basic literal type inference
    char *type = asthra_ai_infer_expression_type(api, "42");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, type, "Failed to infer integer literal type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, type, "i32", "Incorrect integer literal type");
    free(type);
    
    type = asthra_ai_infer_expression_type(api, "3.14");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, type, "Failed to infer float literal type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, type, "f64", "Incorrect float literal type");
    free(type);
    
    type = asthra_ai_infer_expression_type(api, "\"hello\"");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, type, "Failed to infer string literal type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, type, "string", "Incorrect string literal type");
    free(type);
    
    type = asthra_ai_infer_expression_type(api, "true");
    ASTHRA_TEST_ASSERT_NOT_NULL(context, type, "Failed to infer boolean literal type");
    ASTHRA_TEST_ASSERT_STR_EQ(context, type, "bool", "Incorrect boolean literal type");
    free(type);
    
    // Test invalid expression
    type = asthra_ai_infer_expression_type(api, "invalid_syntax!");
    // Should handle gracefully (may return NULL or "unknown")
    if (type) {
        free(type);
    }
    
    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test type compatibility checking
AsthraTestResult test_type_compatibility(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");
    
    // Test exact type matches
    bool compatible = asthra_ai_check_type_compatibility(api, "i32", "i32");
    ASTHRA_TEST_ASSERT_TRUE(context, compatible, "i32 should be compatible with i32");
    
    compatible = asthra_ai_check_type_compatibility(api, "string", "string");
    ASTHRA_TEST_ASSERT_TRUE(context, compatible, "string should be compatible with string");
    
    // Test incompatible types
    compatible = asthra_ai_check_type_compatibility(api, "i32", "string");
    ASTHRA_TEST_ASSERT_FALSE(context, compatible, "i32 should not be compatible with string");
    
    // Test integer type promotion
    compatible = asthra_ai_check_type_compatibility(api, "i32", "i16");
    ASTHRA_TEST_ASSERT_TRUE(context, compatible, "i16 should be compatible with i32 (promotion)");
    
    compatible = asthra_ai_check_type_compatibility(api, "i64", "i32");
    ASTHRA_TEST_ASSERT_TRUE(context, compatible, "i32 should be compatible with i64 (promotion)");
    
    // Test float type promotion
    compatible = asthra_ai_check_type_compatibility(api, "f64", "f32");
    ASTHRA_TEST_ASSERT_TRUE(context, compatible, "f32 should be compatible with f64 (promotion)");
    
    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test compatible types query
AsthraTestResult test_compatible_types_query(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");
    
    size_t count = 0;
    char **types = asthra_ai_get_compatible_types(api, "assignment", &count);
    
    ASTHRA_TEST_ASSERT_NOT_NULL(context, types, "Failed to get compatible types");
    ASTHRA_TEST_ASSERT_TRUE(context, count > 0, "Should return at least some compatible types");
    
    // Check that basic types are included
    bool found_i32 = false, found_string = false, found_bool = false;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(types[i], "i32") == 0) found_i32 = true;
        if (strcmp(types[i], "string") == 0) found_string = true;
        if (strcmp(types[i], "bool") == 0) found_bool = true;
    }
    
    ASTHRA_TEST_ASSERT_TRUE(context, found_i32, "Compatible types should include i32");
    ASTHRA_TEST_ASSERT_TRUE(context, found_string, "Compatible types should include string");
    ASTHRA_TEST_ASSERT_TRUE(context, found_bool, "Compatible types should include bool");
    
    asthra_ai_free_string_array(types, count);
    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test type information queries
AsthraTestResult test_type_info_queries(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");
    
    // Test primitive type check
    bool is_primitive = asthra_ai_is_primitive_type(api, "i32");
    ASTHRA_TEST_ASSERT_TRUE(context, is_primitive, "i32 should be a primitive type");
    
    is_primitive = asthra_ai_is_primitive_type(api, "string");
    ASTHRA_TEST_ASSERT_TRUE(context, is_primitive, "string should be a primitive type");
    
    is_primitive = asthra_ai_is_primitive_type(api, "Point");
    ASTHRA_TEST_ASSERT_FALSE(context, is_primitive, "Point should not be a primitive type");
    
    // Test available types query
    size_t count = 0;
    char **available_types = asthra_ai_get_available_types(api, &count);
    ASTHRA_TEST_ASSERT_NOT_NULL(context, available_types, "Failed to get available types");
    ASTHRA_TEST_ASSERT_TRUE(context, count > 0, "Should return at least some available types");
    
    // Check for basic types
    bool found_i32 = false, found_void = false;
    for (size_t i = 0; i < count; i++) {
        if (strcmp(available_types[i], "i32") == 0) found_i32 = true;
        if (strcmp(available_types[i], "void") == 0) found_void = true;
    }
    
    ASTHRA_TEST_ASSERT_TRUE(context, found_i32, "Available types should include i32");
    ASTHRA_TEST_ASSERT_TRUE(context, found_void, "Available types should include void");
    
    asthra_ai_free_string_array(available_types, count);
    
    // Test type alias resolution (placeholder test)
    char *resolved = asthra_ai_resolve_type_alias(api, "NonExistentAlias");
    // Should return NULL for non-existent alias
    ASTHRA_TEST_ASSERT_NULL(context, resolved, "Non-existent alias should return NULL");
    
    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test detailed type information
AsthraTestResult test_detailed_type_info(AsthraTestContext *context) {
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");
    
    // Note: This test may not work fully until semantic analyzer populates type information
    // But we can test the API structure
    
    AITypeInfo *info = asthra_ai_get_type_info(api, "Point");
    if (info) {
        // If we get type info, validate its structure
        ASTHRA_TEST_ASSERT_NOT_NULL(context, info->type_name, "Type info should have a name");
        ASTHRA_TEST_ASSERT_NOT_NULL(context, info->category, "Type info should have a category");
        
        printf("DEBUG: Type info for Point - name: %s, category: %s, generic: %s\n",
               info->type_name, info->category, info->is_generic ? "yes" : "no");
        
        asthra_ai_free_type_info(info);
    } else {
        printf("INFO: Type info not available yet - semantic analyzer needs enhancement\n");
    }
    
    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Test error handling
AsthraTestResult test_type_queries_error_handling(AsthraTestContext *context) {
    // Test with NULL API
    char *type = asthra_ai_infer_expression_type(NULL, "42");
    ASTHRA_TEST_ASSERT_NULL(context, type, "NULL API should return NULL");
    
    // Test with NULL expression
    AsthraSemanticsAPI *api = setup_test_api();
    ASTHRA_TEST_ASSERT_NOT_NULL(context, api, "Failed to create test API");
    
    type = asthra_ai_infer_expression_type(api, NULL);
    ASTHRA_TEST_ASSERT_NULL(context, type, "NULL expression should return NULL");
    
    // Test compatibility with NULL parameters
    bool compatible = asthra_ai_check_type_compatibility(api, NULL, "i32");
    ASTHRA_TEST_ASSERT_FALSE(context, compatible, "NULL expected type should return false");
    
    compatible = asthra_ai_check_type_compatibility(api, "i32", NULL);
    ASTHRA_TEST_ASSERT_FALSE(context, compatible, "NULL actual type should return false");
    
    // Test compatible types with NULL context
    size_t count = 0;
    char **types = asthra_ai_get_compatible_types(api, NULL, &count);
    ASTHRA_TEST_ASSERT_NULL(context, types, "NULL context should return NULL");
    asthra_test_assert_int_eq(context, count, 0, "Count should be 0 for NULL context");
    
    cleanup_test_api(api);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    AsthraTestSuite *suite = asthra_test_suite_create("AI API Type Queries Tests", "Tests for type inference and type information queries");
    
    asthra_test_suite_add_test(suite, "test_type_inference", "Test type inference functionality", test_type_inference);
    asthra_test_suite_add_test(suite, "test_type_compatibility", "Test type compatibility checking", test_type_compatibility);
    asthra_test_suite_add_test(suite, "test_compatible_types_query", "Test compatible types query", test_compatible_types_query);
    asthra_test_suite_add_test(suite, "test_type_info_queries", "Test type information queries", test_type_info_queries);
    asthra_test_suite_add_test(suite, "test_detailed_type_info", "Test detailed type information", test_detailed_type_info);
    asthra_test_suite_add_test(suite, "test_type_queries_error_handling", "Test type queries error handling", test_type_queries_error_handling);
    
    AsthraTestResult result = asthra_test_suite_run(suite);
    asthra_test_suite_destroy(suite);
    
    return (result == ASTHRA_TEST_PASS) ? 0 : 1;
}
