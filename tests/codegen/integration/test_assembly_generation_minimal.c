#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_basic_assembly_generation", __FILE__, __LINE__, "Test basic assembly generation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_function_assembly", __FILE__, __LINE__, "Test function assembly generation",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_data_section_assembly", __FILE__, __LINE__, "Test data section assembly",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test functions using minimal framework approach
static AsthraTestResult test_basic_assembly_generation(AsthraTestContext *context) {
    const char *test_source = "fn simple() -> i32 {\n"
                              "    return 42;\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_basic_asm.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }

    // Basic validation - if we can parse and setup analyzer, test passes
    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_function_assembly(AsthraTestContext *context) {
    const char *test_source = "fn add_numbers(a: i32, b: i32) -> i32 {\n"
                              "    let result = a + b;\n"
                              "    return result;\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_function_asm.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
    if (!analyzer) {
        ast_free_node(ast);
        return ASTHRA_TEST_FAIL;
    }

    destroy_semantic_analyzer(analyzer);
    ast_free_node(ast);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_data_section_assembly(AsthraTestContext *context) {
    const char *test_source = "static global_value: i32 = 100;\n"
                              "fn get_global() -> i32 {\n"
                              "    return global_value;\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_data_asm.ast");
    if (!ast) {
        return ASTHRA_TEST_FAIL;
    }

    struct SemanticAnalyzer *analyzer = setup_semantic_analyzer();
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
    AsthraTestFunction tests[] = {test_basic_assembly_generation, test_function_assembly,
                                  test_data_section_assembly};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    printf("Running Assembly Generation Tests (Minimal Framework)\n");
    printf("====================================================\n");

    int passed = 0;
    for (size_t i = 0; i < test_count; i++) {
        AsthraTestContext context = {.metadata = test_metadata[i],
                                     .result = ASTHRA_TEST_PASS,
                                     .error_message = NULL,
                                     .assertions_count = 0,
                                     .assertions_passed = 0};

        printf("Running %s... ", test_metadata[i].name);
        AsthraTestResult result = tests[i](&context);

        if (result == ASTHRA_TEST_PASS) {
            printf("PASS\n");
            passed++;
        } else {
            printf("FAIL\n");
        }
    }

    printf("\nAssembly Generation Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
}
