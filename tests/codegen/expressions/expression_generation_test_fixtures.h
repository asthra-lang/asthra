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
#include "../framework/test_framework_minimal.h"
#include "../framework/semantic_test_utils.h"
#else
#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#endif
#include "code_generator.h"
#include "code_generator_core.h"
#include "code_generator_types.h"
#include "code_generator_instructions.h"
#include "ast.h"
#include "semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

// =============================================================================
// TEST FIXTURE DEFINITIONS
// =============================================================================

/**
 * Test fixture for code generator testing
 */
typedef struct {
    CodeGenerator* generator;
    SemanticAnalyzer* analyzer;
    ASTNode* test_ast;
    char* output_buffer;
    size_t output_buffer_size;
} CodeGenTestFixture;

// =============================================================================
// FIXTURE MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Setup test fixture with a code generator
 * @return Initialized test fixture or NULL on failure
 */
static CodeGenTestFixture* setup_codegen_fixture(void) {
    CodeGenTestFixture* fixture = calloc(1, sizeof(CodeGenTestFixture));
    if (!fixture) return NULL;
    
    fixture->generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!fixture->generator) {
        free(fixture);
        return NULL;
    }
    
    fixture->analyzer = setup_semantic_analyzer();
    if (!fixture->analyzer) {
        code_generator_destroy(fixture->generator);
        free(fixture);
        return NULL;
    }
    
    fixture->output_buffer_size = 4096;
    fixture->output_buffer = malloc(fixture->output_buffer_size);
    if (!fixture->output_buffer) {
        destroy_semantic_analyzer(fixture->analyzer);
        code_generator_destroy(fixture->generator);
        free(fixture);
        return NULL;
    }
    
    // Connect the semantic analyzer to the code generator
    fixture->generator->semantic_analyzer = fixture->analyzer;
    
    return fixture;
}

/**
 * Cleanup test fixture
 * @param fixture Test fixture to cleanup
 */
static void cleanup_codegen_fixture(CodeGenTestFixture* fixture) {
    if (!fixture) return;
    
    if (fixture->test_ast) {
        ast_free_node(fixture->test_ast);
    }
    if (fixture->output_buffer) {
        free(fixture->output_buffer);
    }
    if (fixture->analyzer) {
        destroy_semantic_analyzer(fixture->analyzer);
    }
    if (fixture->generator) {
        code_generator_destroy(fixture->generator);
    }
    free(fixture);
}

// =============================================================================
// TEST FUNCTION DECLARATIONS
// =============================================================================

/**
 * Test arithmetic expression generation
 */
AsthraTestResult test_generate_arithmetic_expressions(AsthraTestContext* context);

/**
 * Test logical expression generation
 */
AsthraTestResult test_generate_logical_expressions(AsthraTestContext* context);

/**
 * Test comparison expression generation
 */
AsthraTestResult test_generate_comparison_expressions(AsthraTestContext* context);

/**
 * Test function call expression generation
 */
AsthraTestResult test_generate_call_expressions(AsthraTestContext* context);

/**
 * Test unary expression generation
 */
AsthraTestResult test_generate_unary_expressions(AsthraTestContext* context);

/**
 * Test bitwise expression generation
 */
AsthraTestResult test_generate_bitwise_expressions(AsthraTestContext* context);

/**
 * Test array/slice expression generation
 */
AsthraTestResult test_generate_array_slice_expressions(AsthraTestContext* context);

#endif // ASTHRA_EXPRESSION_GENERATION_TEST_FIXTURES_H 
