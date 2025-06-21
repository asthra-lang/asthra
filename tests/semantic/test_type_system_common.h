/**
 * Asthra Programming Language
 * Type System Test Common Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common utilities and fixtures for type system tests
 */

#ifndef TYPE_SYSTEM_TEST_COMMON_H
#define TYPE_SYSTEM_TEST_COMMON_H

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "semantic_analyzer.h"
#include "parser.h"
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SHARED TEST FIXTURES
// =============================================================================

/**
 * Setup function for type checking tests
 * @param context Test context
 * @return Test result
 */
AsthraTestResult setup_type_checking_test(AsthraTestContext* context);

/**
 * Teardown function for type checking tests
 * @param context Test context
 * @return Test result
 */
AsthraTestResult teardown_type_checking_test(AsthraTestContext* context);

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Create and initialize a semantic analyzer for testing
 * @return Initialized semantic analyzer or NULL on failure
 */
SemanticAnalyzer* create_test_semantic_analyzer(void);

/**
 * Cleanup and destroy a test semantic analyzer
 * @param analyzer Analyzer to destroy
 */
void destroy_test_semantic_analyzer(SemanticAnalyzer* analyzer);

/**
 * Parse test source code and return AST
 * @param source Source code string
 * @param filename Filename for error reporting
 * @return AST node or NULL on failure
 */
ASTNode* parse_test_source_code(const char* source, const char* filename);

/**
 * Analyze AST with semantic analyzer
 * @param analyzer Semantic analyzer
 * @param ast AST to analyze
 * @return true if analysis succeeded, false otherwise
 */
bool analyze_test_ast_node(SemanticAnalyzer* analyzer, ASTNode* ast);

#endif // TYPE_SYSTEM_TEST_COMMON_H 
