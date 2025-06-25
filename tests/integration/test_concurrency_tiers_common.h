/*
 * Concurrency Tiers Integration Tests - Common Definitions
 *
 * Shared header for the modular concurrency tiers test suite.
 * Contains common includes, types, macros, and helper function declarations.
 *
 * Phase 8: Testing and Validation
 * Focus: Tier interactions, progression patterns, and real-world scenarios
 */

#ifndef CONCURRENCY_TIERS_COMMON_H
#define CONCURRENCY_TIERS_COMMON_H

#include "../../runtime/asthra_concurrency_bridge_modular.h"
#include "../../runtime/stdlib_concurrency_support.h"
#include "../framework/test_framework.h"
#include "ast_types.h"
#include "code_generator_core.h"
#include "parser.h"
#include "parser_string_interface.h"
#include "semantic_core.h"
#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

// Test framework macros and global counters
extern size_t tests_run;
extern size_t tests_passed;

#define ASSERT_TRUE(condition)                                                                     \
    do {                                                                                           \
        tests_run++;                                                                               \
        if (condition) {                                                                           \
            tests_passed++;                                                                        \
            printf("  ✅ PASS: %s\n", #condition);                                                 \
        } else {                                                                                   \
            printf("  ❌ FAIL: %s:%d - %s\n", __FILE__, __LINE__, #condition);                     \
        }                                                                                          \
    } while (0)

#define ASSERT_FALSE(condition) ASSERT_TRUE(!(condition))
#define ASSERT_NULL(ptr) ASSERT_TRUE((ptr) == NULL)
#define ASSERT_NOT_NULL(ptr) ASSERT_TRUE((ptr) != NULL)
#define ASSERT_EQUAL(a, b) ASSERT_TRUE((a) == (b))
#define ASSERT_STR_CONTAINS(str, substring) ASSERT_TRUE(strstr((str), (substring)) != NULL)

// ParseResult is now provided by parser_string_interface.h

typedef struct {
    bool success;
    int error_count;
    struct {
        char *message;
    } *errors;
} SemanticAnalysisResult;

// Helper function declarations
ASTNode *find_function_declaration(ASTNode *ast, const char *name);
bool has_annotation(ASTNode *function_node, const char *annotation_name);
bool contains_spawn_statement(ASTNode *node);
bool contains_await_expression(ASTNode *node);
// cleanup_parse_result is now provided by parser_string_interface.h
void cleanup_semantic_result(SemanticAnalysisResult *result);

// Function declarations for semantic analysis and AST management
// parse_string is now provided by parser_string_interface.h
SemanticAnalysisResult analyze_semantics(ASTNode *ast);
void free_ast_node(ASTNode *node);

// Test function declarations for each module
void test_simple_to_advanced_progression(void);
void test_mixed_tier_usage(void);
void test_web_server_scenario(void);
void test_data_processing_pipeline(void);
void test_cpu_intensive_workload(void);
void test_io_intensive_workload(void);
void test_go_to_asthra_migration(void);
void test_annotation_missing_detection(void);

// Common test initialization and cleanup
int initialize_concurrency_runtime(void);
void cleanup_concurrency_runtime(void);

#endif // CONCURRENCY_TIERS_COMMON_H
