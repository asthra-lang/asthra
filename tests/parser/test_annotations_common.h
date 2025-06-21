/*
 * Annotation Tests Common - Shared Definitions and Utilities
 * Common header for modular annotation test suite
 * 
 * Provides shared types, macros, and utility functions for:
 * - Basic annotation parsing tests
 * - Annotation validation tests  
 * - Complex annotation scenarios
 * - Error detection tests
 * - Performance and edge cases
 */

#ifndef TEST_ANNOTATIONS_COMMON_H
#define TEST_ANNOTATIONS_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "../framework/test_framework.h"
#include "ast_types.h"
#include "parser.h"
#include "semantic_core.h"
#include "semantic_annotations.h"

// ============================================================================
// Test Framework Macros
// ============================================================================

// Global test counters (defined in test_annotations_common.c)
extern int tests_run;
extern int tests_passed;

#define ASSERT_TRUE(condition) \
    do { \
        tests_run++; \
        if (condition) { \
            tests_passed++; \
            printf("  ✅ PASS: %s\n", #condition); \
        } else { \
            printf("  ❌ FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition); \
        } \
    } while(0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_STR_EQUAL(a, b) ASSERT_TRUE(strcmp((a), (b)) == 0)

// ============================================================================
// Shared Utility Functions
// ============================================================================

/**
 * Find a function declaration by name in the AST
 * @param ast Root AST node to search
 * @param name Function name to find
 * @return Function declaration node or NULL if not found
 */
ASTNode* find_function_declaration(ASTNode* ast, const char* name);

/**
 * Check if a function has a specific annotation
 * @param function_node Function declaration AST node
 * @param annotation_name Name of annotation to check for
 * @return true if annotation is present, false otherwise
 */
bool has_annotation(ASTNode* function_node, const char* annotation_name);

/**
 * Clean up parse result resources
 * @param result Parse result to clean up
 */
void cleanup_parse_result(ParseResult* result);

/**
 * Clean up semantic analysis result resources
 * @param result Semantic analysis result to clean up
 */
void cleanup_semantic_result(SemanticAnalysisResult* result);

/**
 * Initialize test counters
 */
void init_test_counters(void);

/**
 * Get current test statistics
 * @param total_tests Output parameter for total tests run
 * @param passed_tests Output parameter for tests passed
 */
void get_test_stats(int* total_tests, int* passed_tests);

/**
 * Print test results summary
 * @param category_name Name of test category for reporting
 */
void print_test_results(const char* category_name);

// ============================================================================
// Test Function Declarations
// ============================================================================

// Basic Annotation Parsing Tests (test_annotations_basic.c)
void test_non_deterministic_annotation_parsing(void);
void test_multiple_annotations(void);
void test_annotation_with_parameters(void);

// Annotation Validation Tests (test_annotations_validation.c)
void test_non_deterministic_required_for_tier2(void);
void test_non_deterministic_sufficient_for_tier2(void);
void test_tier1_no_annotation_required(void);

// Complex Annotation Scenarios (test_annotations_complex.c)
void test_nested_function_annotations(void);
void test_mixed_tier_function_calls(void);

// Error Detection Tests (test_annotations_errors.c)
void test_invalid_annotation_syntax(void);
void test_unknown_annotation_handling(void);
void test_annotation_placement_restrictions(void);

// Performance and Edge Cases (test_annotations_performance.c)
void test_many_annotations(void);
void test_annotation_with_complex_parameters(void);

#endif // TEST_ANNOTATIONS_COMMON_H 
