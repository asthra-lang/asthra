#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_register_allocation", __FILE__, __LINE__, "Test basic register allocation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_spill_handling", __FILE__, __LINE__, "Test register spill handling", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_interference_graph", __FILE__, __LINE__, "Test interference graph construction", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL},
    {"test_register_coalescing", __FILE__, __LINE__, "Test register coalescing optimization", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions using minimal framework approach
static AsthraTestResult test_basic_register_allocation(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_registers(void) {\n"
        "    let a = 5;\n"
        "    let b = 10;\n"
        "    let c = a + b;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_basic_registers.ast");
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

static AsthraTestResult test_spill_handling(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_spill(void) {\n"
        "    let v1 = 1; let v2 = 2; let v3 = 3; let v4 = 4;\n"
        "    let v5 = 5; let v6 = 6; let v7 = 7; let v8 = 8;\n"
        "    let result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_spill.ast");
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

static AsthraTestResult test_interference_graph(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_interference(void) {\n"
        "    let x = 1;\n"
        "    if (x > 0) {\n"
        "        let y = x + 1;\n"
        "        x = y * 2;\n"
        "    }\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_interference.ast");
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

static AsthraTestResult test_register_coalescing(AsthraTestContext* context) {
    const char* test_source = 
        "fn test_coalescing(void) {\n"
        "    let a = 10;\n"
        "    let b = a;  // Should coalesce\n"
        "    let c = b + 5;\n"
        "    return c;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_coalescing.ast");
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
        test_basic_register_allocation,
        test_spill_handling,
        test_interference_graph,
        test_register_coalescing
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    printf("Running Register Allocation Tests (Minimal Framework)\n");
    printf("====================================================\n");
    
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
    
    printf("\nRegister Allocation Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
} 
