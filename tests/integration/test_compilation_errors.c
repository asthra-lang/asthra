/**
 * Asthra Programming Language Compiler
 * Compilation Error Handling Testing
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements error handling testing for the compilation pipeline,
 * focusing on proper error propagation and recovery across compilation phases.
 */

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "backend_interface.h"
#include "elf_writer.h"
#include "ffi_assembly_generator.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURE AND UTILITIES
// =============================================================================

/**
 * Test fixture for error handling testing
 */
typedef struct {
    Lexer *lexer;
    Parser *parser;
    SemanticAnalyzer *analyzer;
    AsthraBackend *backend;
    ELFWriter *elf_writer;
    FFIAssemblyGenerator *ffi_generator;
    ASTNode *ast;
    char *source_code;
    char *output_filename;
} ErrorHandlingTestFixture;

/**
 * Setup test fixture for error handling
 */
static ErrorHandlingTestFixture *setup_error_handling_fixture(const char *source,
                                                              const char *filename) {
    ErrorHandlingTestFixture *fixture = calloc(1, sizeof(ErrorHandlingTestFixture));
    if (!fixture)
        return NULL;

    fixture->source_code = strdup(source);
    if (!fixture->source_code) {
        free(fixture);
        return NULL;
    }

    fixture->output_filename = strdup(filename);
    if (!fixture->output_filename) {
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    fixture->lexer = lexer_create(source, strlen(source), filename);
    if (!fixture->lexer) {
        free(fixture->output_filename);
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    fixture->parser = parser_create(fixture->lexer);
    if (!fixture->parser) {
        lexer_destroy(fixture->lexer);
        free(fixture->output_filename);
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    fixture->analyzer = setup_semantic_analyzer();
    if (!fixture->analyzer) {
        parser_destroy(fixture->parser);
        lexer_destroy(fixture->lexer);
        free(fixture->output_filename);
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    fixture->backend = asthra_backend_create_by_type(ASTHRA_BACKEND_LLVM_IR);
    if (!fixture->backend) {
        destroy_semantic_analyzer(fixture->analyzer);
        parser_destroy(fixture->parser);
        lexer_destroy(fixture->lexer);
        free(fixture->output_filename);
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    fixture->ffi_generator = ffi_assembly_generator_create();
    if (!fixture->ffi_generator) {
        asthra_backend_destroy(fixture->backend);
        destroy_semantic_analyzer(fixture->analyzer);
        parser_destroy(fixture->parser);
        lexer_destroy(fixture->lexer);
        free(fixture->output_filename);
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    fixture->elf_writer = elf_writer_create(fixture->ffi_generator);
    if (!fixture->elf_writer) {
        ffi_assembly_generator_destroy(fixture->ffi_generator);
        asthra_backend_destroy(fixture->backend);
        destroy_semantic_analyzer(fixture->analyzer);
        parser_destroy(fixture->parser);
        lexer_destroy(fixture->lexer);
        free(fixture->output_filename);
        free(fixture->source_code);
        free(fixture);
        return NULL;
    }

    return fixture;
}

/**
 * Cleanup test fixture
 */
static void cleanup_error_handling_fixture(ErrorHandlingTestFixture *fixture) {
    if (!fixture)
        return;

    if (fixture->ast) {
        ast_free_node(fixture->ast);
    }
    if (fixture->elf_writer) {
        elf_writer_destroy(fixture->elf_writer);
    }
    if (fixture->ffi_generator) {
        ffi_assembly_generator_destroy(fixture->ffi_generator);
    }
    if (fixture->backend) {
        asthra_backend_destroy(fixture->backend);
    }
    if (fixture->analyzer) {
        destroy_semantic_analyzer(fixture->analyzer);
    }
    if (fixture->parser) {
        parser_destroy(fixture->parser);
    }
    if (fixture->lexer) {
        lexer_destroy(fixture->lexer);
    }
    if (fixture->output_filename) {
        remove(fixture->output_filename);
        free(fixture->output_filename);
    }
    if (fixture->source_code) {
        free(fixture->source_code);
    }
    free(fixture);
}

// =============================================================================
// ERROR PROPAGATION TESTS
// =============================================================================

/**
 * Test lexer error propagation
 */
AsthraTestResult test_lexer_error_propagation(AsthraTestContext *context) {
    // Invalid token sequence - unterminated string
    const char *source = "fn main() -> i32 {\n"
                         "    let s = \"unterminated string\n" // Missing closing quote
                         "    return 0;\n"
                         "}\n";

    ErrorHandlingTestFixture *fixture = setup_error_handling_fixture(source, "lexer_error_test.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup error handling fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Attempt to parse - should fail at lexer level
    fixture->ast = parser_parse_program(fixture->parser);
    if (!asthra_test_assert_pointer_eq(context, fixture->ast, NULL,
                                       "Expected parsing to fail due to lexer error")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that lexer has error state
    bool has_error = lexer_has_error(fixture->lexer);
    if (!asthra_test_assert_bool(context, has_error, "Lexer should have error state")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_error_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test parser error propagation
 */
AsthraTestResult test_parser_error_propagation(AsthraTestContext *context) {
    // Syntax error - missing type annotation (v1.15+ requirement)
    const char *source = "fn main() -> i32 {\n"
                         "    let x = 42;\n" // Missing type annotation
                         "    return x;\n"
                         "}\n";

    ErrorHandlingTestFixture *fixture = setup_error_handling_fixture(source, "parser_error_test.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup error handling fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Attempt to parse - should fail at parser level due to missing type annotation
    fixture->ast = parser_parse_program(fixture->parser);
    if (!asthra_test_assert_pointer_eq(context, fixture->ast, NULL,
                                       "Expected parsing to fail due to missing type annotation")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that parser has error state
    bool has_error = parser_has_error(fixture->parser);
    if (!asthra_test_assert_bool(context, has_error, "Parser should have error state")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_error_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test semantic error propagation
 */
AsthraTestResult test_semantic_error_propagation(AsthraTestContext *context) {
    // Semantic error - undefined variable
    const char *source = "fn main() -> i32 {\n"
                         "    return undefined_variable;\n" // Variable not declared
                         "}\n";

    ErrorHandlingTestFixture *fixture =
        setup_error_handling_fixture(source, "semantic_error_test.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup error handling fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Parse should succeed
    fixture->ast = parser_parse_program(fixture->parser);
    if (!asthra_test_assert_pointer(context, fixture->ast, "Parsing should succeed")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Semantic analysis should fail
    bool success = analyze_test_ast(fixture->analyzer, fixture->ast);
    if (!asthra_test_assert_bool_eq(context, success, false,
                                    "Expected semantic analysis to fail")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Check that analyzer has error state
    bool has_error = semantic_analyzer_has_error(fixture->analyzer);
    if (!asthra_test_assert_bool(context, has_error, "Semantic analyzer should have error state")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_error_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test type mismatch error
 */
AsthraTestResult test_type_mismatch_error(AsthraTestContext *context) {
    // Type error - returning wrong type
    const char *source = "fn main() -> i32 {\n"
                         "    return \"string literal\";\n" // Returning string instead of i32
                         "}\n";

    ErrorHandlingTestFixture *fixture = setup_error_handling_fixture(source, "type_error_test.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup error handling fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    fixture->ast = parser_parse_program(fixture->parser);
    if (!asthra_test_assert_pointer(context, fixture->ast, "Parsing should succeed")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Semantic analysis should fail due to type mismatch
    bool success = analyze_test_ast(fixture->analyzer, fixture->ast);
    if (!asthra_test_assert_bool_eq(context, success, false,
                                    "Expected semantic analysis to fail due to type mismatch")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_error_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test codegen error propagation
 */
AsthraTestResult test_codegen_error_propagation(AsthraTestContext *context) {
    // Valid syntax and semantics, but might fail in codegen
    const char *source = "fn main() -> i32 {\n"
                         "    return 42;\n"
                         "}\n";

    ErrorHandlingTestFixture *fixture =
        setup_error_handling_fixture(source, "codegen_error_test.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup error handling fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    fixture->ast = parser_parse_program(fixture->parser);
    if (!asthra_test_assert_pointer(context, fixture->ast, "Parsing should succeed")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    bool success = analyze_test_ast(fixture->analyzer, fixture->ast);
    if (!asthra_test_assert_bool(context, success, "Semantic analysis should succeed")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    // Force a codegen error by destroying the generator early
    asthra_backend_destroy(fixture->backend);
    fixture->backend = NULL;

    // Attempt code generation - should handle null generator gracefully
    bool codegen_success = asthra_backend_generate_program(fixture->backend, fixture->ast);
    if (!asthra_test_assert_bool_eq(context, codegen_success, false,
                                    "Expected code generation to fail with null generator")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_error_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

/**
 * Test multiple error conditions
 */
AsthraTestResult test_multiple_error_conditions(AsthraTestContext *context) {
    // Multiple errors - syntax error and semantic error
    const char *source =
        "fn main() -> i32 {\n"
        "    let x = undefined_variable\n" // Missing semicolon + undefined variable
        "    return x\n"                   // Missing semicolon
        // Missing closing brace
        "\n";

    ErrorHandlingTestFixture *fixture =
        setup_error_handling_fixture(source, "multiple_errors_test.o");
    if (!asthra_test_assert_pointer(context, fixture, "Failed to setup error handling fixture")) {
        return ASTHRA_TEST_FAIL;
    }

    // Should fail at parser level due to syntax errors
    fixture->ast = parser_parse_program(fixture->parser);
    if (!asthra_test_assert_pointer_eq(context, fixture->ast, NULL,
                                       "Expected parsing to fail due to syntax errors")) {
        cleanup_error_handling_fixture(fixture);
        return ASTHRA_TEST_FAIL;
    }

    cleanup_error_handling_fixture(fixture);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// MAIN TEST RUNNER
// =============================================================================

int main(void) {
    AsthraTestSuite suite = {
        .name = "Compilation Error Handling Tests",
        .tests = {{"Lexer Error Propagation", test_lexer_error_propagation},
                  {"Parser Error Propagation", test_parser_error_propagation},
                  {"Semantic Error Propagation", test_semantic_error_propagation},
                  {"Type Mismatch Error", test_type_mismatch_error},
                  {"Codegen Error Propagation", test_codegen_error_propagation},
                  {"Multiple Error Conditions", test_multiple_error_conditions},
                  {NULL, NULL}}};

    return asthra_test_run_suite(&suite);
}
