/**
 * Expression Validation Tests - Helper Functions
 * 
 * Shared helper functions for expression validation testing.
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_expression_validation_common.h"

// =============================================================================
// SHARED TEST HELPER FUNCTIONS
// =============================================================================

bool test_expression_success(const char* source, const char* test_name) {
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode* ast = parse_test_source_code(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool success = analyze_test_ast_node(analyzer, ast);
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

bool test_expression_error(const char* source, SemanticErrorCode expected_error, const char* test_name) {
    SemanticAnalyzer* analyzer = create_test_semantic_analyzer();
    if (!analyzer) {
        printf("Failed to create semantic analyzer for test: %s\n", test_name);
        return false;
    }

    ASTNode* ast = parse_test_source_code(source, test_name);
    if (!ast) {
        printf("Failed to parse source for test: %s\n", test_name);
        destroy_test_semantic_analyzer(analyzer);
        return false;
    }

    bool analysis_result = analyze_test_ast_node(analyzer, ast);
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