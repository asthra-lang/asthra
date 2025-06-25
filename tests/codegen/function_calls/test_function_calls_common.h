#ifndef TEST_FUNCTION_CALLS_COMMON_H
#define TEST_FUNCTION_CALLS_COMMON_H

/**
 * Function Call Code Generation Tests - Common Header
 * 
 * This header contains shared definitions and utilities for testing function call
 * generation for associated functions, instance methods, and self parameter
 * handling in the Asthra compiler.
 * 
 * Test Coverage:
 * - Associated function call generation (Point::new -> call Point_associated_new)
 * - Instance method call generation (obj.method -> call Point_instance_method)
 * - Self parameter handling and passing
 * - Method argument translation
 * 
 * Copyright (c) 2024 Asthra Project
 */

#define TEST_FRAMEWORK_MINIMAL 1

// Ensure we use real parser implementation, not stubs
#define ASTHRA_PARSER_REAL_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Test framework
#ifdef TEST_FRAMEWORK_MINIMAL
#include "../framework/test_framework_minimal.h"
#else
#include "../framework/test_context.h"
#include "../framework/test_assertions.h"
#include "../framework/test_statistics.h"
#include "../framework/test_suite.h"
#endif

// Asthra components
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include "../../../src/codegen/backend_interface.h"
#include "../../../src/compiler.h"

// =============================================================================
// COMMON UTILITIES AND HELPERS
// =============================================================================

/**
 * Create a test lexer from source code
 */
Lexer* create_test_lexer(const char* source);

/**
 * Create a test parser from source code
 */
Parser* create_test_parser(const char* source);

/**
 * Clean up parser and associated lexer
 */
void cleanup_parser(Parser* parser);

/**
 * Common test pipeline for parsing, analysis, and code generation
 */
typedef struct {
    Parser* parser;
    ASTNode* program;
    SemanticAnalyzer* analyzer;
    AsthraBackend* backend;
    AsthraCompilerContext* compiler_ctx;
    const char* output;
    bool success;
} FunctionCallTestPipeline;

/**
 * Initialize and run the complete test pipeline
 */
bool run_test_pipeline(FunctionCallTestPipeline* pipeline, const char* source, AsthraTestContext* context);

/**
 * Clean up the test pipeline
 */
void cleanup_test_pipeline(FunctionCallTestPipeline* pipeline);

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

/**
 * Test associated function call generation (Point::new)
 */
AsthraTestResult test_associated_function_calls(AsthraTestContext* context);

/**
 * Test instance method call generation (obj.method)
 */
AsthraTestResult test_instance_method_calls(AsthraTestContext* context);

/**
 * Test self parameter handling and field access
 */
AsthraTestResult test_self_parameter_handling(AsthraTestContext* context);

/**
 * Test chained method calls and complex scenarios
 */
AsthraTestResult test_chained_method_calls(AsthraTestContext* context);

/**
 * Run the complete function calls test suite
 */
AsthraTestResult run_function_calls_test_suite(void);

#endif // TEST_FUNCTION_CALLS_COMMON_H 
