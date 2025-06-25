/**
 * Symbol Export and Visibility Tests for pub, impl, and self Features
 *
 * This file contains code generation tests specifically focused on symbol export
 * and visibility handling. It validates that the code generator properly exports
 * public symbols and keeps private symbols internal.
 *
 * Test Coverage:
 * - Public symbol export (.global/.globl directives)
 * - Private symbol handling (no export)
 * - Visibility modifier processing
 * - Symbol table generation
 *
 * Copyright (c) 2024 Asthra Project
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test framework
#include "../framework/lexer_test_utils.h"
#include "../framework/parser_test_utils.h"
#include "../framework/test_assertions.h"
#include "../framework/test_context.h"
#include "../framework/test_statistics.h"
#include "../framework/test_suite.h"

// Asthra components
#include "ast.h"
#include "code_generator.h"
#include "code_generator_core.h"
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

static bool has_symbol_export(const char *output, const char *symbol_name) {
    if (!output || !symbol_name)
        return false;

    // Check for various export directive formats
    char global_directive[256];
    char globl_directive[256];
    char export_directive[256];

    snprintf(global_directive, sizeof(global_directive), ".global %s", symbol_name);
    snprintf(globl_directive, sizeof(globl_directive), ".globl %s", symbol_name);
    snprintf(export_directive, sizeof(export_directive), "export %s", symbol_name);

    return strstr(output, global_directive) != NULL || strstr(output, globl_directive) != NULL ||
           strstr(output, export_directive) != NULL;
}

// =============================================================================
// SYMBOL EXPORT TESTS
// =============================================================================

static AsthraTestResult test_public_symbol_export(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "pub struct Point { pub x: f64, priv y: f64 }\n"
                         "impl Point {\n"
                         "    pub fn new(x: f64, y: f64) -> Point {\n"
                         "        return Point { x: x, y: y };\n"
                         "    }\n"
                         "    priv fn private_helper(self) -> f64 {\n"
                         "        return self.x + self.y;\n"
                         "    }\n"
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
        // Print semantic analyzer errors if available
        if (analyzer && analyzer->error_count > 0) {
            printf("  Semantic errors: %zu\n", analyzer->error_count);
            if (analyzer->last_error && analyzer->last_error->message) {
                printf("  Last error: %s\n", analyzer->last_error->message);
            }
        }
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    CodeGenerator *codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ASTHRA_TEST_ASSERT(context, codegen != NULL, "Code generator should be created")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Connect the semantic analyzer to the code generator
    codegen->semantic_analyzer = analyzer;

    bool codegen_result = code_generate_program(codegen, program);
    if (!ASTHRA_TEST_ASSERT(context, codegen_result, "Code generation should succeed")) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Get assembly output
    const size_t buffer_size = 65536;
    char *output = malloc(buffer_size);
    if (!output) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!code_generator_emit_assembly(codegen, output, buffer_size)) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Debug: Print first 500 chars of output to see what we got
    printf("DEBUG: Assembly output (first 500 chars):\n%.500s\n", output);
    printf("DEBUG: Output buffer size used: %zu\n", strlen(output));

    // Check that public symbols are exported and private symbols are not
    bool has_public_export = has_symbol_export(output, "Point_associated_new");
    printf("DEBUG: has_public_export = %d\n", has_public_export);

    if (!ASTHRA_TEST_ASSERT(context, has_public_export,
                            "Generated code should export public functions")) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Check that private symbols are NOT exported
    bool has_private_export = has_symbol_export(output, "Point_instance_private_helper");

    if (!ASTHRA_TEST_ASSERT(context, !has_private_export,
                            "Generated code should NOT export private functions")) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    free(output);
    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_mixed_visibility_export(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "pub fn public_function(x: i32) -> i32 {\n"
                         "    return x * 2;\n"
                         "}\n"
                         "\n"
                         "priv fn private_function(y: i32) -> i32 {\n"
                         "    return y + 1;\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    let result: i32 = public_function(5);\n"
                         "    let internal: i32 = private_function(10);\n"
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
        // Print semantic analyzer errors if available
        if (analyzer && analyzer->error_count > 0) {
            printf("  Semantic errors: %zu\n", analyzer->error_count);
            if (analyzer->last_error && analyzer->last_error->message) {
                printf("  Last error: %s\n", analyzer->last_error->message);
            }
        }
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    CodeGenerator *codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ASTHRA_TEST_ASSERT(context, codegen != NULL, "Code generator should be created")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Connect the semantic analyzer to the code generator
    codegen->semantic_analyzer = analyzer;

    bool codegen_result = code_generate_program(codegen, program);
    if (!ASTHRA_TEST_ASSERT(context, codegen_result, "Code generation should succeed")) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Get assembly output
    const size_t buffer_size = 65536;
    char *output = malloc(buffer_size);
    if (!output) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!code_generator_emit_assembly(codegen, output, buffer_size)) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Check public symbols are exported
    bool has_public_export = has_symbol_export(output, "public_function");
    bool has_main_export = has_symbol_export(output, "main");

    // Check private symbols are NOT exported
    bool has_private_export = has_symbol_export(output, "private_function");

    if (!ASTHRA_TEST_ASSERT(context, has_public_export,
                            "Generated code should export public_function")) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, has_main_export,
                            "Generated code should export main function")) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_private_export,
                            "Generated code should NOT export private_function")) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    free(output);
    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

static AsthraTestResult test_private_only_symbols(AsthraTestContext *context) {
    const char *source = "package test;\n\n"
                         "priv struct InternalData {\n"
                         "    priv value: i32,\n"
                         "    priv flag: bool\n"
                         "}\n"
                         "impl InternalData {\n"
                         "    priv fn create(val: i32) -> InternalData {\n"
                         "        return InternalData { value: val, flag: true };\n"
                         "    }\n"
                         "    priv fn process(self) -> i32 {\n"
                         "        return self.value * 2;\n"
                         "    }\n"
                         "}\n"
                         "\n"
                         "pub fn main(none) -> void {\n"
                         "    // Main function required for valid program\n"
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
        // Print semantic analyzer errors if available
        if (analyzer && analyzer->error_count > 0) {
            printf("  Semantic errors: %zu\n", analyzer->error_count);
            if (analyzer->last_error && analyzer->last_error->message) {
                printf("  Last error: %s\n", analyzer->last_error->message);
            }
        }
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    CodeGenerator *codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!ASTHRA_TEST_ASSERT(context, codegen != NULL, "Code generator should be created")) {
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Connect the semantic analyzer to the code generator
    codegen->semantic_analyzer = analyzer;

    bool codegen_result = code_generate_program(codegen, program);
    if (!ASTHRA_TEST_ASSERT(context, codegen_result, "Code generation should succeed")) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Get assembly output
    const size_t buffer_size = 65536;
    char *output = malloc(buffer_size);
    if (!output) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!code_generator_emit_assembly(codegen, output, buffer_size)) {
        free(output);
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    // Check that NO symbols are exported (all private)
    bool has_create_export = has_symbol_export(output, "InternalData_associated_create");
    bool has_process_export = has_symbol_export(output, "InternalData_instance_process");

    if (!ASTHRA_TEST_ASSERT(context, !has_create_export,
                            "Generated code should NOT export private create function")) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    if (!ASTHRA_TEST_ASSERT(context, !has_process_export,
                            "Generated code should NOT export private process method")) {
        code_generator_destroy(codegen);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        cleanup_parser(parser);
        return ASTHRA_TEST_FAIL;
    }

    free(output);
    code_generator_destroy(codegen);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    cleanup_parser(parser);
    return ASTHRA_TEST_PASS;
}

// =============================================================================
// TEST SUITE EXECUTION
// =============================================================================

static AsthraTestResult run_symbol_export_test_suite(void) {
    printf("Running symbol export and visibility test suite...\n");

    AsthraTestStatistics *global_stats = asthra_test_statistics_create();
    AsthraTestResult overall_result = ASTHRA_TEST_PASS;

    AsthraTestMetadata base_metadata = {.name = "Symbol Export Tests",
                                        .file = __FILE__,
                                        .line = __LINE__,
                                        .function = __func__,
                                        .severity = ASTHRA_TEST_SEVERITY_HIGH,
                                        .timeout_ns = 10000000000ULL, // 10 seconds
                                        .skip = false,
                                        .skip_reason = NULL};

    struct {
        const char *name;
        AsthraTestResult (*test_func)(AsthraTestContext *);
    } tests[] = {
        {"Public symbol export", test_public_symbol_export},
        {"Mixed visibility export", test_mixed_visibility_export},
        {"Private only symbols", test_private_only_symbols},
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

    printf("\n=== Symbol Export Test Summary ===\n");
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
    printf("Asthra Symbol Export and Visibility Tests\n");
    printf("==========================================\n\n");

    AsthraTestResult result = run_symbol_export_test_suite();

    printf("\n");
    if (result == ASTHRA_TEST_PASS) {
        printf("✅ All symbol export tests passed!\n");
        return 0;
    } else {
        printf("❌ Some symbol export tests failed!\n");
        return 1;
    }
}
