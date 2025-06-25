/**
 * Asthra Programming Language
 * Type Inference Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for type inference functionality
 */

#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework.h"
#include "ast_operations.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "test_type_system_common.h"
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool test_type_check_success(const char *source, const char *test_name) {
    SemanticAnalyzer *analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode *ast = parse_test_source(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed for test: %s\n", test_name);
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
// TYPE INFERENCE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_type_inference_int, ASTHRA_TEST_SEVERITY_HIGH) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: int = 42;\n"
                         "    let y: int = -123;\n"
                         "    let z: int = 0;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "type_inference_int"),
                            "Should infer int type from integer literals");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_type_inference_float, ASTHRA_TEST_SEVERITY_HIGH) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: float = 3.14;\n"
                         "    let y: float = -2.718;\n"
                         "    let z: float = 0.0;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "type_inference_float"),
                            "Should infer float type from float literals");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_type_inference_bool, ASTHRA_TEST_SEVERITY_HIGH) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: bool = true;\n"
                         "    let y: bool = false;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "type_inference_bool"),
                            "Should infer bool type from boolean literals");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_type_inference_string, ASTHRA_TEST_SEVERITY_HIGH) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let x: string = \"hello\";\n"
                         "    let y: string = \"world\";\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "type_inference_string"),
                            "Should infer string type from string literals");

    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_type_inference_result, ASTHRA_TEST_SEVERITY_HIGH) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let result: Result<i32, string> = Result.Ok(42);\n"
                         "    return ();\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "type_inference_result"),
                            "Should infer Result<i32, string> from context in Result.Ok(42)");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_type_inference_int_metadata, test_type_inference_float_metadata,
        test_type_inference_bool_metadata, test_type_inference_string_metadata,
        test_type_inference_result_metadata};

    AsthraTestFunction test_functions[] = {test_type_inference_int, test_type_inference_float,
                                           test_type_inference_bool, test_type_inference_string,
                                           test_type_inference_result};

    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);

    AsthraTestSuiteConfig config = {.name = "Type Inference Tests",
                                    .description = "Tests for semantic analyzer type inference",
                                    .parallel_execution = false,
                                    .default_timeout_ns = 30000000000ULL,
                                    .json_output = false,
                                    .verbose_output = true};

    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}