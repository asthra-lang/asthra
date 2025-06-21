/**
 * Asthra Programming Language
 * Semantic Analyzer Test Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test utility functions for semantic analysis testing
 */

#ifndef ASTHRA_SEMANTIC_TEST_UTILS_H
#define ASTHRA_SEMANTIC_TEST_UTILS_H

#include "semantic_analyzer.h"
#include "semantic_enums.h"
#include "type_info.h"
#include "ast.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SEMANTIC ANALYZER TEST UTILITIES
// =============================================================================

/**
 * Setup a semantic analyzer for testing
 * @return A new semantic analyzer for testing
 */
SemanticAnalyzer* setup_semantic_analyzer(void);

/**
 * Destroy a test semantic analyzer
 * @param analyzer The analyzer to destroy
 */
void destroy_semantic_analyzer(SemanticAnalyzer* analyzer);

/**
 * Analyze an AST and return semantic information
 * @param analyzer The semantic analyzer
 * @param ast The AST to analyze
 * @return true if analysis succeeded, false otherwise
 */
bool analyze_test_ast(SemanticAnalyzer* analyzer, ASTNode* ast);

// =============================================================================
// ENHANCED SEMANTIC VALIDATION UTILITIES
// =============================================================================

/**
 * Validate enum semantic analysis with comprehensive checks
 * @param analyzer The semantic analyzer
 * @param enum_ast The enum declaration AST
 * @param expected_name Expected enum name
 * @return true if validation passed, false otherwise
 */
bool validate_enum_semantic_analysis(SemanticAnalyzer* analyzer, ASTNode* enum_ast, const char* expected_name);

/**
 * Validate type inference for expressions
 * @param analyzer The semantic analyzer
 * @param expr_ast The expression AST
 * @param expected_type_name Expected type name
 * @return true if type inference is correct, false otherwise
 */
bool validate_type_inference(SemanticAnalyzer* analyzer, ASTNode* expr_ast, const char* expected_type_name);

/**
 * Validate pattern matching exhaustiveness
 * @param analyzer The semantic analyzer
 * @param match_ast The match statement AST
 * @param should_be_exhaustive Expected exhaustiveness state
 * @return true if exhaustiveness check matches expectation, false otherwise
 */
bool semantic_test_validate_pattern_exhaustiveness(SemanticAnalyzer* analyzer, ASTNode* match_ast, bool should_be_exhaustive);

/**
 * Validate symbol resolution and visibility
 * @param analyzer The semantic analyzer
 * @param symbol_name Symbol name to check
 * @param should_be_visible Expected visibility
 * @return true if visibility matches expectation, false otherwise
 */
bool validate_symbol_visibility(SemanticAnalyzer* analyzer, const char* symbol_name, bool should_be_visible);

/**
 * Validate error reporting and collection
 * @param analyzer The semantic analyzer
 * @param expected_error_count Expected number of errors
 * @param expected_error_types Array of expected error types (can be NULL)
 * @return true if error reporting matches expectation, false otherwise
 */
bool validate_error_reporting(SemanticAnalyzer* analyzer, size_t expected_error_count, int* expected_error_types);

/**
 * Validate generic type parameter constraints
 * @param analyzer The semantic analyzer
 * @param type_params The type parameters AST node list
 * @param expected_constraint_count Expected number of constraints
 * @return true if constraints are valid, false otherwise
 */
bool validate_generic_constraints(SemanticAnalyzer* analyzer, ASTNodeList* type_params, size_t expected_constraint_count);

/**
 * Validate FFI semantic compatibility
 * @param analyzer The semantic analyzer
 * @param decl_ast The FFI declaration AST
 * @param expected_abi Expected ABI type
 * @return true if FFI semantics are valid, false otherwise
 */
bool validate_ffi_semantics(SemanticAnalyzer* analyzer, ASTNode* decl_ast, const char* expected_abi);

/**
 * Validate memory safety analysis
 * @param analyzer The semantic analyzer
 * @param expr_ast The expression AST to check
 * @param expected_safety_level Expected safety level
 * @return true if memory safety analysis is correct, false otherwise
 */
bool validate_memory_safety(SemanticAnalyzer* analyzer, ASTNode* expr_ast, int expected_safety_level);

/**
 * Get semantic analysis statistics for validation
 * @param analyzer The semantic analyzer
 * @return Statistics structure with analysis metrics
 */
typedef struct {
    size_t symbols_analyzed;
    size_t types_created;
    size_t errors_reported;
    size_t warnings_reported;
    size_t enum_variants_processed;
    size_t pattern_matches_validated;
    bool memory_tracking_enabled;
} SemanticAnalysisStats;

SemanticAnalysisStats get_semantic_analysis_stats(SemanticAnalyzer* analyzer);

/**
 * Reset semantic analyzer state for fresh testing
 * @param analyzer The semantic analyzer
 */
void reset_semantic_analyzer(SemanticAnalyzer* analyzer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_TEST_UTILS_H
