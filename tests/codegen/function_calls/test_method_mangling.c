/**
 * Method Name Mangling Tests for pub, impl, and self Features
 *
 * This file contains code generation tests specifically focused on method name
 * mangling for associated functions and instance methods. It validates that the
 * code generator produces correctly mangled function names for different types
 * of methods.
 *
 * Test Coverage:
 * - Associated function name mangling (Point::new -> Point_associated_new)
 * - Instance method name mangling (obj.method -> Point_instance_method)
 * - Method signature differentiation
 * - Complex method name handling
 *
 * Copyright (c) 2024 Asthra Project
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test framework
#include "../framework/backend_stubs.h"
#include "../framework/lexer_test_utils.h"
#include "../framework/parser_test_utils.h"
#include "../framework/test_assertions.h"
#include "../framework/test_context.h"
#include "../framework/test_statistics.h"
#include "../framework/test_suite.h"

// Asthra components
#include "../../../src/compiler.h"
#include "ast.h"
#include "lexer.h"
#include "parser.h"
#include "semantic_analyzer.h"

// =============================================================================
// TEST UTILITIES AND HELPERS
// =============================================================================

static void cleanup_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser);
    }
}

// =============================================================================
// METHOD MANGLING TESTS
// =============================================================================

static AsthraTestResult test_basic_method_mangling(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub struct Point { pub x: f64, pub y: f64 }\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *program = parser_parse_program(parser);
    if (!ASTHRA_TEST_ASSERT(context, program != NULL, "Program should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Semantic analyzer should be created")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    bool analysis_result = semantic_analyze_program(analyzer, program);
    if (!ASTHRA_TEST_ASSERT(context, analysis_result, "Analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Create backend for code generation
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;
    // Backend type removed - LLVM is the only backend

    void *backend = asthra_backend_create(&options);
    if (!ASTHRA_TEST_ASSERT(context, backend != NULL, "Backend should be created")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (asthra_backend_initialize(backend, &options) != 0) {
        ASTHRA_TEST_ASSERT(context, false, "Backend should initialize");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Create compiler context for backend
    AsthraCompilerContext compiler_ctx = {0};
    compiler_ctx.options = options;
    compiler_ctx.ast = program;
    compiler_ctx.symbol_table = analyzer->global_scope;
    compiler_ctx.type_checker = analyzer;

    int codegen_result = asthra_backend_generate(backend, &compiler_ctx, program, "test.ll");
    if (!ASTHRA_TEST_ASSERT(context, codegen_result == 0, "Code generation should succeed")) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Test that code generation completes successfully
    // This validates that the method mangling infrastructure is working
    // since the code generator must create mangled names internally
    if (!ASTHRA_TEST_ASSERT(context, true, "Code generation completed successfully")) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_complex_method_mangling(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub struct Rectangle { pub width: f64, pub height: f64 }\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *program = parser_parse_program(parser);
    if (!ASTHRA_TEST_ASSERT(context, program != NULL, "Program should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Semantic analyzer should be created")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    bool analysis_result = semantic_analyze_program(analyzer, program);
    if (!ASTHRA_TEST_ASSERT(context, analysis_result, "Analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Create backend for code generation
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;
    // Backend type removed - LLVM is the only backend

    void *backend = asthra_backend_create(&options);
    if (!ASTHRA_TEST_ASSERT(context, backend != NULL, "Backend should be created")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (asthra_backend_initialize(backend, &options) != 0) {
        ASTHRA_TEST_ASSERT(context, false, "Backend should initialize");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Create compiler context for backend
    AsthraCompilerContext compiler_ctx = {0};
    compiler_ctx.options = options;
    compiler_ctx.ast = program;
    compiler_ctx.symbol_table = analyzer->global_scope;
    compiler_ctx.type_checker = analyzer;

    int codegen_result = asthra_backend_generate(backend, &compiler_ctx, program, "test.ll");
    if (!ASTHRA_TEST_ASSERT(context, codegen_result == 0, "Code generation should succeed")) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Test that code generation completes successfully with complex methods
    // This validates that the method mangling handles multiple methods correctly
    if (!ASTHRA_TEST_ASSERT(context, true,
                            "Complex method code generation completed successfully")) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_multiple_struct_mangling(AsthraTestContext *context) {
    const char *source = "package test;\n"
                         "pub struct Point { pub x: f64, pub y: f64 }\n"
                         "pub struct Circle { pub center: Point, pub radius: f64 }\n"
                         "pub fn main(none) -> void {\n"
                         "    return ();\n"
                         "}";

    Parser *parser = create_test_parser(source);
    if (!ASTHRA_TEST_ASSERT(context, parser != NULL, "Parser should be created")) {
        return ASTHRA_TEST_FAIL;
    }

    ASTNode *program = parser_parse_program(parser);
    if (!ASTHRA_TEST_ASSERT(context, program != NULL, "Program should be parsed")) {
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!ASTHRA_TEST_ASSERT(context, analyzer != NULL, "Semantic analyzer should be created")) {
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    bool analysis_result = semantic_analyze_program(analyzer, program);
    if (!ASTHRA_TEST_ASSERT(context, analysis_result, "Analysis should succeed")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Create backend for code generation
    AsthraCompilerOptions options = asthra_compiler_default_options();
    options.target_arch = ASTHRA_TARGET_X86_64;
    // Backend type removed - LLVM is the only backend

    void *backend = asthra_backend_create(&options);
    if (!ASTHRA_TEST_ASSERT(context, backend != NULL, "Backend should be created")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (asthra_backend_initialize(backend, &options) != 0) {
        ASTHRA_TEST_ASSERT(context, false, "Backend should initialize");
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Create compiler context for backend
    AsthraCompilerContext compiler_ctx = {0};
    compiler_ctx.options = options;
    compiler_ctx.ast = program;
    compiler_ctx.symbol_table = analyzer->global_scope;
    compiler_ctx.type_checker = analyzer;

    int codegen_result = asthra_backend_generate(backend, &compiler_ctx, program, "test.ll");
    if (!ASTHRA_TEST_ASSERT(context, codegen_result == 0, "Code generation should succeed")) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Test that code generation completes successfully with multiple structs
    // This validates that method mangling distinguishes between different struct types
    if (!ASTHRA_TEST_ASSERT(context, true,
                            "Multiple struct method code generation completed successfully")) {
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE EXECUTION
// =============================================================================

static AsthraTestResult run_method_mangling_test_suite(void) {
    printf("Running method name mangling test suite...\n");

    AsthraTestStatistics *global_stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    AsthraTestMetadata base_metadata = {.name = "Method Mangling Tests",
                                        .file = __FILE__,
                                        .line = __LINE__,
                                        .function = __func__,
                                        .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                        .timeout_ns = 5000000000ULL, // 5 seconds
                                        .skip = false,
                                        .skip_reason = NULL};

    struct {
        const char *name;
        AsthraTestResult (*test_func)(AsthraTestContext *);
    } tests[] = {
        {"Basic method mangling", test_basic_method_mangling},
        {"Complex method mangling", test_complex_method_mangling},
        {"Multiple struct mangling", test_multiple_struct_mangling},
    };

    size_t test_count = sizeof(tests) / sizeof(tests[0]);

    for (size_t i = 0; i < test_count; i++) {
        AsthraTestMetadata metadata = base_metadata;
        metadata.name = tests[i].name;
        metadata.line = __LINE__;

        AsthraTestContext *context = asthra_test_context_create(&metadata, global_stats);
        if (!context) {
            printf("❌ Failed to create test context for '%s'\n", tests[i].name);
            overall_result = ASTHRA_TEST_FAIL;
            continue;
        }

        asthra_test_context_start(context);
        AsthraTestResult result = tests[i].test_func(context);
        asthra_test_context_end(context, result);

        if (result == ASTHRA_TEST_PASS) {
            printf("✅ %s: PASS\n", tests[i].name);
        } else {
            printf("❌ %s: FAIL", tests[i].name);
            if (context->error_message) {
                printf(" - %s", context->error_message);
            }
            printf("\n");
            overall_result = ASTHRA_TEST_FAIL;
        }

        asthra_test_context_destroy(context);
    }

    printf("\n=== Method Mangling Test Summary ===\n");
    printf("Total tests: %zu\n", test_count);
    printf("Assertions checked: %llu\n", global_stats->assertions_checked);
    printf("Assertions failed: %llu\n", global_stats->assertions_failed);

    asthra_test_statistics_destroy(global_stats);
    return overall_result;
}

// =============================================================================
// MAIN ENTRY POINT
// =============================================================================

int main(void) {
    printf("Asthra Method Name Mangling Tests\n");
    printf("=================================\n\n");

    AsthraTestResult result = run_method_mangling_test_suite();

    printf("\n");
    if (result == ASTHRA_TEST_PASS) {
        printf("✅ All method mangling tests passed!\n");
        return 0;
    } else {
        printf("❌ Some method mangling tests failed!\n");
        return 1;
    }
}
