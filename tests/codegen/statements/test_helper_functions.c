/**
 * Asthra Programming Language Compiler
 * Statement Generation Test Helper Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Helper functions for code generation and verification
 */

#include "expression_oriented_test_utils.h"
#include "statement_generation_test_fixtures.h"

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

/**
 * Generate code for a statement and verify basic success
 */
bool generate_and_verify_statement(AsthraTestContext *context, CodeGenTestFixture *fixture,
                                   ASTNode *ast, const char *test_name) {
    if (!ast) {
        return false;
    }

    // Create a minimal compiler context for the backend
    AsthraCompilerOptions opts = asthra_compiler_default_options();
    // Backend type removed - LLVM is the only backend
    AsthraCompilerContext ctx = {
        .options = opts, .error_count = 0, .errors = NULL, .error_capacity = 0};

    // For statement generation, we need to generate the whole AST
    // The backend expects a complete program AST
    bool result = (asthra_backend_generate(fixture->backend, &ctx, ast, NULL) == 0);
    if (!result) {
        if (context) {
            context->result = ASTHRA_TEST_FAIL;
            printf("ERROR: Failed to generate code for %s\n", test_name);
        }
        return false;
    }

    return true;
}

/**
 * Parse test source and verify parsing succeeded
 */
ASTNode *parse_and_verify_source(AsthraTestContext *context, const char *source,
                                 const char *filename, const char *test_name) {
    ASTNode *ast = parse_test_source(source, filename);
    if (!ast) {
        if (context) {
            context->result = ASTHRA_TEST_FAIL;
            printf("ERROR: Failed to parse source for %s\n", test_name);
        }
        return NULL;
    }

    return ast;
}

/**
 * Common test pattern: parse source, generate code, cleanup
 */
AsthraTestResult test_statement_generation_pattern(AsthraTestContext *context,
                                                   CodeGenTestFixture *fixture, const char *source,
                                                   const char *test_name) {
    // Use the fragment parser for statement fragments
    ASTNode *program = parse_statement_fragment(source, test_name);
    if (!program) {
        if (context) {
            context->result = ASTHRA_TEST_FAIL;
        }
        return ASTHRA_TEST_FAIL;
    }

    // First, run semantic analysis on the program
    if (fixture->analyzer) {
        bool semantic_success = semantic_analyze_program(fixture->analyzer, program);
        if (!semantic_success) {
            // Print any semantic errors
            if (fixture->analyzer->error_count > 0) {
                printf("Semantic errors:\n");
                for (size_t i = 0; i < fixture->analyzer->error_count && i < 5; i++) {
                    printf("  - %s\n", fixture->analyzer->errors[i].message);
                }
            }
            ast_free_node(program);
            if (context) {
                context->result = ASTHRA_TEST_FAIL;
                printf("ERROR: Semantic analysis failed for %s\n", test_name);
            }
            return ASTHRA_TEST_FAIL;
        }

        // Note: Semantic analyzer is passed through compiler context, not stored in backend
    }

    // Create a compiler context with semantic analyzer
    AsthraCompilerOptions opts = asthra_compiler_default_options();
    // Backend type removed - LLVM is the only backend
    AsthraCompilerContext ctx = {
        .options = opts, .error_count = 0, .errors = NULL, .error_capacity = 0
        // Note: Add semantic analyzer to context if needed by backend
    };

    // For now, generate code for the entire program
    // In a more sophisticated implementation, we would extract just the statement
    bool success = (asthra_backend_generate(fixture->backend, &ctx, program, NULL) == 0);

    if (!success && context) {
        context->result = ASTHRA_TEST_FAIL;
        printf("ERROR: Failed to generate code for %s\n", test_name);
    }

    ast_free_node(program);

    return success ? ASTHRA_TEST_PASS : ASTHRA_TEST_FAIL;
}