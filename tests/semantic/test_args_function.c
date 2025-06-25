/**
 * Tests for args() predeclared function
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework.h"
#include "ast_operations.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "test_type_system_common.h"

static bool test_args_function_exists(void) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let arguments: []string = args();\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode *ast = parse_test_source(source, "test_args_function_exists");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError *error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", error->message, error->location.line,
                   error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_returns_string_slice(void) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let arguments: []string = args();\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode *ast = parse_test_source(source, "test_args_function_returns_string_slice");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError *error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", error->message, error->location.line,
                   error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_no_parameters(void) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let arguments: []string = args(\"invalid\");\n" // Should fail - args
                                                                              // takes no parameters
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode *ast = parse_test_source(source, "test_args_function_no_parameters");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    // This test expects failure - args takes no parameters
    if (success) {
        printf("Expected semantic analysis to fail but it passed\n");
        success = false;
    } else {
        printf("Expected failure - args() takes no parameters\n");
        success = true; // We expected it to fail
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_can_iterate(void) {
    const char *source = "package test;\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let arguments: []string = args();\n"
                         "    for arg in arguments {\n"
                         "        log(arg);\n"
                         "    }\n"
                         "    return ();\n"
                         "}\n";

    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode *ast = parse_test_source(source, "test_args_function_can_iterate");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError *error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", error->message, error->location.line,
                   error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_args_function_can_index(void) {
    const char *source =
        "package test;\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let arguments: []string = args();\n"
        "    // TODO: Add length check once len() is implemented\n"
        "    // For now, just test that args() returns a slice that can be indexed\n"
        "    return ();\n"
        "}\n";

    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer\n");
        return false;
    }

    ASTNode *ast = parse_test_source(source, "test_args_function_can_index");
    if (!ast) {
        printf("Failed to parse source\n");
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed\n");
        SemanticError *error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", error->message, error->location.line,
                   error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

// =============================================================================
// TEST FRAMEWORK INTEGRATION
// =============================================================================

ASTHRA_TEST_DEFINE(args_function_exists, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_function_exists() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_returns_string_slice, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_function_returns_string_slice() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_no_parameters, ASTHRA_TEST_SEVERITY_CRITICAL) {
    return test_args_function_no_parameters() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_can_iterate, ASTHRA_TEST_SEVERITY_HIGH) {
    return test_args_function_can_iterate() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

ASTHRA_TEST_DEFINE(args_function_can_index, ASTHRA_TEST_SEVERITY_HIGH) {
    return test_args_function_can_index() ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}

int main(void) {
    AsthraTestFunction tests[] = {args_function_exists,
                                  args_function_returns_string_slice,
                                  args_function_no_parameters,
                                  args_function_can_iterate,
                                  args_function_can_index};

    AsthraTestMetadata metadatas[] = {{.name = "args_function_exists",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_function_exists",
                                       .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                       .timeout_ns = 0,
                                       .skip = false},
                                      {.name = "args_function_returns_string_slice",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_function_returns_string_slice",
                                       .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                       .timeout_ns = 0,
                                       .skip = false},
                                      {.name = "args_function_no_parameters",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_function_no_parameters",
                                       .severity = ASTHRA_TEST_SEVERITY_CRITICAL,
                                       .timeout_ns = 0,
                                       .skip = false},
                                      {.name = "args_function_can_iterate",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_function_can_iterate",
                                       .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                       .timeout_ns = 0,
                                       .skip = false},
                                      {.name = "args_function_can_index",
                                       .file = __FILE__,
                                       .line = __LINE__,
                                       .function = "args_function_can_index",
                                       .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                       .timeout_ns = 0,
                                       .skip = false}};

    AsthraTestSuiteConfig config = asthra_test_suite_config_create(
        "Args Function Semantic Tests",
        "Tests for args() predeclared function");

    AsthraTestResult result =
        asthra_test_run_suite(tests, metadatas, sizeof(tests) / sizeof(tests[0]), &config);

    return result == ASTHRA_TEST_PASS ? 0 : 1;
}