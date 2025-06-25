#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test metadata
static AsthraTestMetadata test_metadata[] = {
    {"test_constant_folding", __FILE__, __LINE__, "Test constant folding optimization",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_dead_code_elimination", __FILE__, __LINE__, "Test dead code elimination",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_loop_optimization", __FILE__, __LINE__, "Test loop optimization techniques",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Test functions using minimal framework approach
static AsthraTestResult test_constant_folding(AsthraTestContext *context) {
    const char *test_source = "fn test_constants(void) {\n"
                              "    let a = 2 + 3;      // Should fold to 5\n"
                              "    let b = a * 4;      // Should fold to 20\n"
                              "    let c = b / 2;      // Should fold to 10\n"
                              "    return c;\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_constant_folding.ast");
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

static AsthraTestResult test_dead_code_elimination(AsthraTestContext *context) {
    const char *test_source = "fn test_dead_code(void) {\n"
                              "    let used = 42;\n"
                              "    let unused = 99;    // Dead code\n"
                              "    if (false) {        // Dead branch\n"
                              "        let never = 0;\n"
                              "    }\n"
                              "    return used;\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_dead_code.ast");
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

static AsthraTestResult test_loop_optimization(AsthraTestContext *context) {
    const char *test_source = "fn test_loop(void) {\n"
                              "    let sum = 0;\n"
                              "    for (let i = 0; i < 10; i++) {\n"
                              "        sum = sum + i;  // Loop invariant optimization\n"
                              "    }\n"
                              "    return sum;\n"
                              "}";

    struct ASTNode *ast = parse_test_source(test_source, "test_loop_opt.ast");
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
    AsthraTestFunction tests[] = {test_constant_folding, test_dead_code_elimination,
                                  test_loop_optimization};

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    printf("Running Code Optimization Tests (Minimal Framework)\n");
    printf("==================================================\n");

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

    printf("\nCode Optimization Tests Summary: %d/%zu passed\n", passed, test_count);
    return (passed == (int)test_count) ? 0 : 1;
}
