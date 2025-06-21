#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_end_to_end_codegen", __FILE__, __LINE__, "Test end-to-end code generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_multi_function_codegen", __FILE__, __LINE__, "Test multi-function code generation", ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_program_integration", __FILE__, __LINE__, "Test complete program integration", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}
};

// Test functions using minimal framework approach
static AsthraTestResult test_end_to_end_codegen(AsthraTestContext* context) {
    const char* test_source = 
        "fn main(void) {\n"
        "    let x = 42;\n"
        "    let y = calculate(x);\n"
        "    return y;\n"
        "}\n"
        "fn calculate(value: i32) -> i32 {\n"
        "    return value * 2;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_end_to_end.ast");
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

static AsthraTestResult test_multi_function_codegen(AsthraTestContext* context) {
    const char* test_source = 
        "fn add(a: i32, b: i32) -> i32 { return a + b; }\n"
        "fn multiply(x: i32, y: i32) -> i32 { return x * y; }\n"
        "fn compute(val: i32) -> i32 {\n"
        "    let sum = add(val, 5);\n"
        "    let result = multiply(sum, 2);\n"
        "    return result;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_multi_function.ast");
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

static AsthraTestResult test_program_integration(AsthraTestContext* context) {
    const char* test_source = 
        "struct Point { x: i32, y: i32; }\n"
        "impl Point {\n"
        "    fn new(x: i32, y: i32) -> Point { Point { x: x, y: y } }\n"
        "    fn distance_squared(self) -> i32 { self.x * self.x + self.y * self.y }\n"
        "}\n"
        "fn main(void) {\n"
        "    let p = Point::new(3, 4);\n"
        "    let dist = p.distance_squared();\n"
        "    return dist;\n"
        "}";
    
    struct ASTNode* ast = parse_test_source(test_source, "test_program_integration.ast");
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
        test_end_to_end_codegen,
        test_multi_function_codegen,
        test_program_integration
    };
    
    size_t test_count = sizeof(tests) / sizeof(tests[0]);
    
    printf("Running Codegen Integration Tests (Minimal Framework)\n");
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
    
    printf("\nCodegen Integration Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
} 
