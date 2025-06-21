/*
 * Annotation Tests Common - Shared Implementation
 * Common utilities for modular annotation test suite
 * 
 * Provides implementations for shared utility functions:
 * - AST navigation and search functions
 * - Annotation checking utilities
 * - Resource cleanup functions
 * - Test framework support
 */

#include "test_annotations_common.h"

// ============================================================================
// Global Test Counters
// ============================================================================

int tests_run = 0;
int tests_passed = 0;

// ============================================================================
// AST Navigation Functions
// ============================================================================

ASTNode* find_function_declaration(ASTNode* ast, const char* name) {
    if (!ast) return NULL;
    
    if (ast->type == AST_FUNCTION_DECL && 
        ast->data.function_decl.name && 
        strcmp(ast->data.function_decl.name, name) == 0) {
        return ast;
    }
    
    for (int i = 0; i < ast->child_count; i++) {
        ASTNode* result = find_function_declaration(ast->children[i], name);
        if (result) return result;
    }
    
    return NULL;
}

bool has_annotation(ASTNode* function_node, const char* annotation_name) {
    if (!function_node || function_node->type != AST_FUNCTION_DECL) {
        return false;
    }
    
    for (int i = 0; i < function_node->annotation_count; i++) {
        if (strcmp(function_node->annotations[i].name, annotation_name) == 0) {
            return true;
        }
    }
    
    return false;
}

// ============================================================================
// Resource Cleanup Functions
// ============================================================================

void cleanup_parse_result(ParseResult* result) {
    if (result && result->ast) {
        free_ast_node(result->ast);
        result->ast = NULL;
    }
}

void cleanup_semantic_result(SemanticAnalysisResult* result) {
    if (result && result->errors) {
        for (int i = 0; i < result->error_count; i++) {
            free(result->errors[i].message);
        }
        free(result->errors);
        result->errors = NULL;
    }
}

// ============================================================================
// Test Framework Support
// ============================================================================

void init_test_counters(void) {
    tests_run = 0;
    tests_passed = 0;
}

void get_test_stats(int* total_tests, int* passed_tests) {
    if (total_tests) *total_tests = tests_run;
    if (passed_tests) *passed_tests = tests_passed;
}

void print_test_results(const char* category_name) {
    printf("\n=== %s Test Results ===\n", category_name);
    printf("Tests run: %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);
    
    if (tests_run > 0) {
        printf("Success rate: %.1f%%\n", (float)tests_passed / tests_run * 100.0);
    } else {
        printf("Success rate: N/A (no tests run)\n");
    }
    
    if (tests_passed == tests_run && tests_run > 0) {
        printf("✅ All %s tests passed!\n", category_name);
    } else if (tests_run > 0) {
        printf("❌ Some %s tests failed. Please review the output above.\n", category_name);
    } else {
        printf("⚠️  No %s tests were run.\n", category_name);
    }
} 
