/**
 * Asthra Programming Language
 * Type Compatibility Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for type compatibility and assignment validation
 */

#include "test_type_system_common.h"
#include "semantic_analyzer.h"
#include "semantic_errors.h"
#include "semantic_types.h"
#include "parser.h"
#include "ast_operations.h"
#include "../framework/test_framework.h"
#include "../framework/semantic_test_utils.h"
#include <string.h>

// =============================================================================
// TEST HELPERS
// =============================================================================

static bool test_type_check_success(const char* source, const char* test_name) {
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode* ast = parse_test_source(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast(analyzer, ast);
    if (!success) {
        printf("Semantic analysis failed for test: %s\n", test_name);
        SemanticError* error = analyzer->errors;
        while (error) {
            printf("  Error: %s at line %d, column %d\n", 
                   error->message, error->location.line, error->location.column);
            error = error->next;
        }
    }

    ast_free_node(ast);
    destroy_test_semantic_analyzer(analyzer);
    return success;
}

static bool test_type_check_error(const char* source, SemanticErrorCode expected_error, const char* test_name) {
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode* ast = parse_test_source(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool analysis_result = analyze_test_ast(analyzer, ast);
    bool has_expected_error = false;
    
    SemanticError* error = analyzer->errors;
    while (error) {
        if (error->code == expected_error) {
            has_expected_error = true;
            break;
        }
        error = error->next;
    }

    if (analysis_result && !has_expected_error) {
        printf("Expected error %d but analysis succeeded for test: %s\n", expected_error, test_name);
    } else if (!has_expected_error) {
        printf("Expected error %d but got different errors for test: %s\n", expected_error, test_name);
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
// TYPE COMPATIBILITY TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_assignment_compatibility, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: int = 10;\n"
        "    x = 20;\n"
        "    let mut y: string = \"hello\";\n"
        "    y = \"world\";\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_success(source, "assignment_compatibility"),
        "Should allow assignment of compatible types");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_assignment_incompatibility, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let mut x: int = 10;\n"
        "    x = \"string\";\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_error(source, SEMANTIC_ERROR_INCOMPATIBLE_TYPES, "assignment_incompatibility"),
        "Should detect assignment of incompatible types");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_binary_operator_type_checking, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let a: int = 5 + 3;\n"
        "    let b: float = 10.5 - 3.2;\n"
        "    let c: bool = true && false;\n"
        "    let d: bool = 5 > 3;\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_success(source, "binary_operator_type_checking"),
        "Should type check binary operators correctly");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_binary_operator_type_mismatch, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = 5 + \"string\";\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_error(source, SEMANTIC_ERROR_MISMATCHED_TYPES, "binary_operator_type_mismatch"),
        "Should detect type mismatch in binary operations");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_assignment_compatibility_metadata,
        test_assignment_incompatibility_metadata,
        test_binary_operator_type_checking_metadata,
        test_binary_operator_type_mismatch_metadata
    };
    
    AsthraTestFunction test_functions[] = {
        test_assignment_compatibility,
        test_assignment_incompatibility,
        test_binary_operator_type_checking,
        test_binary_operator_type_mismatch
    };
    
    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);
    
    AsthraTestSuiteConfig config = {
        .name = "Type Compatibility Tests",
        .description = "Tests for semantic analyzer type compatibility checking",
        .parallel_execution = false,
        .default_timeout_ns = 30000000000ULL,
        .json_output = false,
        .verbose_output = true
    };
    
    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}