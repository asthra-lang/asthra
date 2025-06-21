/**
 * Asthra Programming Language - Advanced Concurrency Tests Common Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Common definitions, test framework, and shared utilities for the
 * three-tier concurrency system test suite.
 */

#ifndef TEST_ADVANCED_CONCURRENCY_COMMON_H
#define TEST_ADVANCED_CONCURRENCY_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
#include "../framework/test_framework.h"
#include "ast_types.h"
#include "parser.h"
#include "parser_string_interface.h"
#include "semantic_core.h"
#include "semantic_annotations.h"
#include "code_generator_core.h"

// =============================================================================
// TEST FRAMEWORK MACROS
// =============================================================================

// Global test counters (defined in common.c)
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
#define ASSERT_STR_CONTAINS(str, substr) ASSERT_TRUE(strstr((str), (substr)) != NULL)

// =============================================================================
// SHARED DATA STRUCTURES AND TYPES
// =============================================================================

// ParseResult is now provided by parser_string_interface.h
// No need to redefine it here

/**
 * Semantic analysis result structure
 */
typedef struct {
    bool success;
    int error_count;
    struct {
        char* message;
        int line;
        int column;
    } errors[32]; // Maximum 32 errors for testing
} SemanticAnalysisResult;

// TokenType is already defined in ../../src/parser/lexer.h
// We use the official TokenType enum from the actual Asthra lexer

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Test framework functions
void cleanup_parse_result(ParseResult* result);
void cleanup_semantic_result(SemanticAnalysisResult* result);

// Parser and semantic analysis functions
// parse_string is now provided by parser_string_interface.h
SemanticAnalysisResult analyze_semantics(ASTNode* ast);
TokenType keyword_to_token_type(const char* keyword);

// AST navigation and validation helper functions
bool contains_spawn_statement(ASTNode* node);
bool contains_spawn_with_handle_statement(ASTNode* node);
bool contains_await_expression(ASTNode* node);
int count_await_expressions(ASTNode* node);
bool contains_removed_concurrency_nodes(ASTNode* node);
bool has_annotation(ASTNode* function_node, const char* annotation_name);
ASTNode* find_function_declaration(ASTNode* root, const char* function_name);

// Test category functions (implemented in respective modules)

// Tier 1 tests
void test_tier1_spawn_basic_parsing(void);
void test_tier1_spawn_with_handle_parsing(void);
void test_tier1_await_expression_parsing(void);
void test_tier1_deterministic_execution(void);

// Tier 2 tests
void test_tier2_annotation_requirement(void);
void test_tier2_annotation_sufficient(void);
void test_tier2_mixed_with_tier1(void);

// Tier 3 removal tests
void test_tier3_keywords_removed(void);
void test_tier3_syntax_errors(void);
void test_tier3_ast_nodes_removed(void);

// Semantic analysis tests
void test_semantic_tier1_validation(void);
void test_semantic_annotation_enforcement(void);

// Integration tests
void test_tier_progression_pattern(void);
void test_real_world_pattern(void);

// Statistics functions
void print_test_statistics(void);
void reset_test_counters(void);

#endif // TEST_ADVANCED_CONCURRENCY_COMMON_H
