/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Fixture Management
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Fixture management functions for statement generation tests
 */

#include "expression_oriented_test_utils.h"
#include "statement_generation_test_fixtures.h"

// =============================================================================
// FIXTURE MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Setup test fixture with a code generator
 */
CodeGenTestFixture *setup_codegen_fixture(void) {
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

    // Note: Backend doesn't directly store semantic analyzer
    // The analyzer will be passed through the compiler context during generation

    fixture->output_buffer_size = 4096;
    fixture->output_buffer = malloc(fixture->output_buffer_size);
    if (!fixture->output_buffer) {
        destroy_semantic_analyzer(fixture->analyzer);
        asthra_backend_destroy(fixture->backend);
        free(fixture);
        return NULL;
    }

    return fixture;
}

/**
 * Cleanup test fixture and free all resources
 */
void cleanup_codegen_fixture(CodeGenTestFixture *fixture) {
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