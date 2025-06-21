/**
 * Asthra Programming Language
 * Primitive Type Tests for Semantic Analyzer
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Tests for primitive type system validation
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
// PRIMITIVE TYPE TESTS
// =============================================================================

ASTHRA_TEST_DEFINE(test_primitive_int_type, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = 42;\n"
        "    let y: int = -123;\n"
        "    let z: int = 0;\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_success(source, "primitive_int_type"),
        "Should handle int type declarations");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_primitive_float_type, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: float = 3.14;\n"
        "    let y: float = -2.718;\n"
        "    let z: float = 0.0;\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_success(source, "primitive_float_type"),
        "Should handle float type declarations");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_primitive_bool_type, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: bool = true;\n"
        "    let y: bool = false;\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_success(source, "primitive_bool_type"),
        "Should handle bool type declarations");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_primitive_string_type, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: string = \"hello\";\n"
        "    let y: string = \"world\";\n"
        "    let z: string = \"\";\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_success(source, "primitive_string_type"),
        "Should handle string type declarations");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_primitive_type_mismatch, ASTHRA_TEST_SEVERITY_CRITICAL) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: int = \"hello\";\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_error(source, SEMANTIC_ERROR_TYPE_MISMATCH, "primitive_type_mismatch"),
        "Should detect type mismatch between int and string");
    
    return ASTHRA_TEST_PASS;
}

ASTHRA_TEST_DEFINE(test_unknown_type_error, ASTHRA_TEST_SEVERITY_HIGH) {
    const char* source = 
        "package test;\n"
        "pub fn main(none) -> void {\n"
        "    let x: NonExistentType = 42;\n"
        "}\n";
    
    ASTHRA_TEST_ASSERT_TRUE(context, 
        test_type_check_error(source, SEMANTIC_ERROR_UNDEFINED_TYPE, "unknown_type_error"),
        "Should detect unknown type usage");
    
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST MAIN
// =============================================================================

int main(void) {
    // Initialize test metadata
    AsthraTestMetadata test_metadata[] = {
        test_primitive_int_type_metadata,
        test_primitive_float_type_metadata,
        test_primitive_bool_type_metadata,
        test_primitive_string_type_metadata,
        test_primitive_type_mismatch_metadata,
        test_unknown_type_error_metadata
    };
    
    AsthraTestFunction test_functions[] = {
        test_primitive_int_type,
        test_primitive_float_type,
        test_primitive_bool_type,
        test_primitive_string_type,
        test_primitive_type_mismatch,
        test_unknown_type_error
    };
    
    size_t test_count = sizeof(test_functions) / sizeof(test_functions[0]);
    
    AsthraTestSuiteConfig config = {
        .name = "Primitive Type Tests",
        .description = "Tests for semantic analyzer primitive type checking",
        .parallel_execution = false,
        .default_timeout_ns = 30000000000ULL,
        .json_output = false,
        .verbose_output = true
    };
    
    return asthra_test_run_suite(test_functions, test_metadata, test_count, &config);
}