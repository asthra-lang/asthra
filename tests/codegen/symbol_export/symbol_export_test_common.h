/**
 * Common utilities and includes for Symbol Export Tests
 * 
 * This header provides shared functionality for testing symbol export
 * and visibility handling in the Asthra code generator.
 * 
 * Copyright (c) 2024 Asthra Project
 */

#ifndef SYMBOL_EXPORT_TEST_COMMON_H
#define SYMBOL_EXPORT_TEST_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test framework
#include "../framework/test_context.h"
#include "../framework/test_assertions.h"
#include "../framework/test_statistics.h"
#include "../framework/test_suite.h"

// Asthra components
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include "backend_interface.h"
#include "parser_core.h"
// =============================================================================
// SHARED TEST UTILITIES
// =============================================================================

/**
 * Create a test lexer for the given source code
 */
Lexer* create_symbol_export_test_lexer(const char* source);

/**
 * Create a test parser for the given source code
 */
Parser* create_symbol_export_test_parser(const char* source);

/**
 * Clean up parser and associated lexer
 */
void cleanup_parser(Parser* parser);

/**
 * Check if output contains export directive for a symbol
 */
bool has_symbol_export(const char* output, const char* symbol_name);

/**
 * Run the complete parsing and code generation pipeline
 */
typedef struct {
    bool success;
    const char* output;
    Parser* parser;
    ASTNode* program;
    SemanticAnalyzer* analyzer;
    AsthraBackend* backend;
} CodegenTestResult;

CodegenTestResult run_codegen_pipeline(AsthraTestContext* context, const char* source);
void cleanup_codegen_result(CodegenTestResult* result);

// =============================================================================
// TEST SUITE RUNNER DECLARATIONS
// =============================================================================

/**
 * Run public symbol export tests
 */
AsthraTestResult run_public_symbol_export_tests(void);

/**
 * Run private symbol export tests
 */
AsthraTestResult run_private_symbol_export_tests(void);

/**
 * Run mixed visibility export tests
 */
AsthraTestResult run_mixed_visibility_export_tests(void);

#endif // SYMBOL_EXPORT_TEST_COMMON_H 
