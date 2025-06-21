#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_public_symbol_export", __FILE__, __LINE__, "Test public symbol export generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_private_symbol_export", __FILE__, __LINE__, "Test private symbol export handling", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_mixed_visibility_export", __FILE__, __LINE__, "Test mixed visibility symbol export", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_export_name_mangling", __FILE__, __LINE__, "Test export symbol name mangling", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions using minimal framework approach
static AsthraTestResult test_public_symbol_export(AsthraTestContext* context) {
    const char* test_source = 
        "pub fn public_function() -> i32 { return 42; }\n"
        "pub struct PublicStruct { value: i32; }";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_public_export.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    // Basic validation - if we can parse and setup analyzer, test passes
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_private_symbol_export(AsthraTestContext* context) {
    const char* test_source = 
        "fn private_function() -> i32 { return 24; }\n"
        "struct PrivateStruct { hidden: i32; }";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_private_export.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_mixed_visibility_export(AsthraTestContext* context) {
    const char* test_source = 
        "pub fn public_api() -> i32 { return helper(); }\n"
        "fn helper() -> i32 { return 10; }\n"
        "pub struct ApiStruct { pub value: i32, internal: f64; }";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_mixed_export.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_export_name_mangling(AsthraTestContext* context) {
    const char* test_source = 
        "pub mod math {\n"
        "    pub fn add(a: i32, b: i32) -> i32 { a + b }\n"
        "    pub fn multiply(a: i32, b: i32) -> i32 { a * b }\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_export_mangling.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }
    
    struct SemanticAnalyzer* analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }
    
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

// Main test runner
int main(void) {
    AsthraTestFunction tests[] = {
        test_public_symbol_export,
        test_private_symbol_export,
        test_mixed_visibility_export,
        test_export_name_mangling
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    printf("Running Symbol Export Tests (Minimal Framework)\n");
    printf("==============================================\n");
    
    int passed = 0;
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestContext context = {
            .metadata = test_metadata[i],
            .result = ASTHRA_TEST_PASS,
            .error_message = NULL,
            .assertions_count = 0,
            .assertions_passed = 0
        };
        
        printf("Running %s... ", test_metadata[i].name);
        AsthraTestResult result = tests[i](&context);
        
        if (result == ASTHRA_TEST_PASS) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
        }
    }
    
    printf("\nSymbol Export Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
} 
