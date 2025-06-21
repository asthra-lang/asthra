/**
 * Asthra Programming Language Compiler
 * Struct Generation Test Fixtures and Utilities
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Shared test fixtures and utilities for struct generation tests
 */

#ifndef ASTHRA_STRUCT_GENERATION_TEST_FIXTURES_H
#define ASTHRA_STRUCT_GENERATION_TEST_FIXTURES_H

#include "../framework/test_framework.h"
#include "../framework/compiler_test_utils.h"
#include "code_generator.h"
#include "code_generator_core.h"
#include "code_generator_types.h"
#include "code_generator_instructions.h"
#include "ast.h"
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
 * Test struct field access generation
 */
AsthraTestResult test_generate_struct_access(AsthraTestContext* context);

/**
 * Test struct instantiation generation
 */
AsthraTestResult test_generate_struct_instantiation(AsthraTestContext* context);

/**
 * Test struct method generation
 */
AsthraTestResult test_generate_struct_methods(AsthraTestContext* context);

/**
 * Test struct field assignment generation
 */
AsthraTestResult test_generate_struct_assignment(AsthraTestContext* context);

/**
 * Test struct copy and move semantics
 */
AsthraTestResult test_generate_struct_copy_move(AsthraTestContext* context);

/**
 * Test struct with complex types
 */
AsthraTestResult test_generate_struct_complex_types(AsthraTestContext* context);

#endif // ASTHRA_STRUCT_GENERATION_TEST_FIXTURES_H 
