/**
 * Asthra Programming Language
 * Advanced Type Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for advanced types (sized integers, enums, generics)
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

static bool test_type_check_error(const char *source, SemanticErrorCode expected_error,
                                  const char *test_name) {
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

    bool analysis_result = analyze_test_ast(analyzer, ast);
    bool has_expected_error = false;

    SemanticError *error = analyzer->errors;
    while (error) {
        if (error->code == expected_error) {
            has_expected_error = true;
            break;
        }
        error = error->next;
    }

    if (analysis_result && !has_expected_error) {
        printf("Expected error %d but analysis succeeded for test: %s\n", expected_error,
               test_name);
    } else if (!has_expected_error) {
        printf("Expected error %d but got different errors for test: %s\n", expected_error,
               test_name);
        error = analyzer->errors;
        while (error) {
            printf("  Got error %d: %s\n", error->code, error->message);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return has_expected_error;
}

// =============================================================================
// SIZED INTEGER TYPE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_sized_integer_types, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char *source = "package test;\n"
                         "pub fn main(none) -> void {\n"
                         "    let a: i8 = 127;\n"
                         "    let b: i16 = 32767;\n"
                         "    let c: i32 = 2147483647;\n"
                         "    let d: i64 = 100;\n"
                         "    let e: u8 = 255;\n"
                         "    let f: u16 = 65535;\n"
                         "    let g: u32 = 100;\n"
                         "    let h: u64 = 100;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "sized_integer_types"),
                            "Should handle all sized integer types");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// ENUM TYPE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_enum_type_checking, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char *source = "package test;\n"
                         "pub enum Color {\n"
                         "    Red,\n"
                         "    Green,\n"
                         "    Blue\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let c: Color = Color.Red;\n"
                         "}\n";

    ASTHRA_TEST_ASSERT_TRUE(context, test_type_check_success(source, "enum_type_checking"),
                            "Should handle enum type declarations and usage");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// GENERIC TYPE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_generic_struct_type, ASTHRA_TEST_SEVERITY_MEDIUM) {
    const char *source =
        "package test;\n"
        "pub struct Container<T> {\n"
        "    pub value: T\n"
        "}\n"
        "\n"
        "pub fn main(none) -> void {\n"
        "    let int_container: Container<int> = Container { value: 42 };\n"
        "    let str_container: Container<string> = Container { value: \"hello\" };\n"
        "}\n";

    // Generics are not yet implemented - expect this test to fail gracefully
    // Testing that we get appropriate "invalid type" errors rather than crashes
    // Note: Parser now correctly recognizes generic structs but semantic analyzer
    // reports that type arguments are required (SEMANTIC_ERROR_INVALID_TYPE = 4)
    ASTHRA_TEST_ASSERT_TRUE(
        context, test_type_check_error(source, SEMANTIC_ERROR_INVALID_TYPE, "generic_struct_type"),
        "Should report invalid type error for generic struct requiring type arguments");

    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {test_sized_integer_types_metadata,
                                          test_enum_type_checking_metadata,
                                          test_generic_struct_type_metadata};

    AsthraTestFunction test_functions[] = {test_sized_integer_types, test_enum_type_checking,
                                           test_generic_struct_type};

    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);

    AsthraTestSuiteConfig config = {.name = "Advanced Type Tests",
                                    .description =
                                        "Tests for semantic analyzer advanced type checking",
                                    .parallel_execution = false,
                                    .default_timeout_ns = 30000000000ULL,
                                    .json_output = false,
                                    .verbose_output = true};

    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}