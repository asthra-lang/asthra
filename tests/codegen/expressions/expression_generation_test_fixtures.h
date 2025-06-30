/**
 * Asthra Programming Language Compiler
 * Expression Generation Test Fixtures and Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Shared test fixtures and utilities for expression generation tests
 */

#ifndef ASTHRA_EXPRESSION_GENERATION_TEST_FIXTURES_H
#define ASTHRA_EXPRESSION_GENERATION_TEST_FIXTURES_H

#ifdef TEST_FRAMEWORK_MINIMAL
#include "../framework/semantic_test_utils.h"
#include "../framework/test_framework_minimal.h"
#else
#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#endif
#include "ast.h"
// Backend interface removed - using LLVM directly
#include "compiler.h"
#include "semantic_analyzer.h"
#include "../../framework/backend_stubs.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURE DEFINITIONS
// =============================================================================

/**
 * Test fixture for code generator testing
 */
typedef struct {
    void *backend; // Backend abstraction removed
    SemanticAnalyzer *analyzer;
    ASTNode *test_ast;
    char *output_buffer;
    size_t output_buffer_size;
} CodeGenTestFixture;

// =============================================================================
// FIXTURE MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Setup test fixture with a code generator
 * @return Initialized test fixture or NULL on failure
 */
static CodeGenTestFixture *setup_codegen_fixture(void) {
    CodeGenTestFixture *fixture = calloc(1, sizeof(CodeGenTestFixture));
    if (!fixture)
        return NULL;

    // Create compiler options for backend initialization
    AsthraCompilerOptions options = {.target_arch = ASTHRA_TARGET_X86_64,
                                     .opt_level = ASTHRA_OPT_NONE,
                                     .debug_info = true,
                                     .verbose = false};

    fixture->backend = asthra_backend_create(&options);
    if (!fixture->backend) {
        free(fixture);
        return NULL;
    }

    fixture->analyzer = setup_semantic_analyzer();
    if (!fixture->analyzer) {
        asthra_backend_destroy(fixture->backend);
        free(fixture);
        return NULL;
    }

    fixture->output_buffer_size = 4096;
    fixture->output_buffer = malloc(fixture->output_buffer_size);
    if (!fixture->output_buffer) {
        destroy_semantic_analyzer(fixture->analyzer);
        asthra_backend_destroy(fixture->backend);
        free(fixture);
        return NULL;
    }

    // Note: Backend doesn't directly store semantic analyzer
    // The analyzer will be passed through the compiler context during generation

    return fixture;
}

/**
 * Cleanup test fixture
 * @param fixture Test fixture to cleanup
 */
static void cleanup_codegen_fixture(CodeGenTestFixture *fixture) {
    if (!fixture)
        return;

    if (fixture->test_ast) {
        ast_free_node(fixture->test_ast);
    }
    if (fixture->output_buffer) {
        free(fixture->output_buffer);
    }
    if (fixture->analyzer) {
        destroy_semantic_analyzer(fixture->analyzer);
    }
    if (fixture->backend) {
        asthra_backend_destroy(fixture->backend);
    }
    free(fixture);
}

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

/**
 * Test arithmetic expression generation
 */
AsthraTestResult test_generate_arithmetic_expressions(AsthraTestContext *context);

/**
 * Test logical expression generation
 */
AsthraTestResult test_generate_logical_expressions(AsthraTestContext *context);

/**
 * Test comparison expression generation
 */
AsthraTestResult test_generate_comparison_expressions(AsthraTestContext *context);

/**
 * Test function call expression generation
 */
AsthraTestResult test_generate_call_expressions(AsthraTestContext *context);

/**
 * Test unary expression generation
 */
AsthraTestResult test_generate_unary_expressions(AsthraTestContext *context);

/**
 * Test bitwise expression generation
 */
AsthraTestResult test_generate_bitwise_expressions(AsthraTestContext *context);

/**
 * Test array/slice expression generation
 */
AsthraTestResult test_generate_array_slice_expressions(AsthraTestContext *context);

#endif // ASTHRA_EXPRESSION_GENERATION_TEST_FIXTURES_H
