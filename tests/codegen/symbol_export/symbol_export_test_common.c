/**
 * Common utilities implementation for Symbol Export Tests
 *
 * Copyright (c) 2024 Asthra Project
 */

#include "symbol_export_test_common.h"

// =============================================================================
// SHARED TEST UTILITIES IMPLEMENTATION
// =============================================================================

Lexer *create_symbol_export_test_lexer(const char *source) {
    return lexer_create(source, strlen(source), "test_symbol_export.asthra");
}

Parser *create_symbol_export_test_parser(const char *source) {
    Lexer *lexer = create_symbol_export_test_lexer(source);
    if (!lexer)
        return NULL;
    return parser_create(lexer);
}

void cleanup_parser(Parser *parser) {
    if (parser) {
        parser_destroy(parser);
        // Parser owns lexer, no need to destroy separately
    }
}

bool has_symbol_export(const char *output, const char *symbol_name) {
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

CodegenTestResult run_codegen_pipeline(AsthraTestContext *context, const char *source) {
    CodegenTestResult result = {0};

    // Create parser
    result.parser = create_symbol_export_test_parser(source);
    if (!result.parser) {
        ASTHRA_TEST_ASSERT(context, false, "Parser should be created");
        return result;
    }

    // Parse program
    result.program = parser_parse_program(result.parser);
    if (!result.program) {
        ASTHRA_TEST_ASSERT(context, false, "Program should be parsed");
        return result;
    }

    // Create semantic analyzer
    result.analyzer = semantic_analyzer_create();
    if (!result.analyzer) {
        ASTHRA_TEST_ASSERT(context, false, "Semantic analyzer should be created");
        return result;
    }

    // Run semantic analysis
    bool analysis_result = semantic_analyze_program(result.analyzer, result.program);
    if (!analysis_result) {
        ASTHRA_TEST_ASSERT(context, false, "Analysis should succeed");
        return result;
    }

    // Create code generator
    result.codegen = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!result.codegen) {
        ASTHRA_TEST_ASSERT(context, false, "Code generator should be created");
        return result;
    }

    // Connect the semantic analyzer to the code generator
    result.codegen->semantic_analyzer = result.analyzer;

    // Generate code
    bool codegen_result = code_generate_program(result.codegen, result.program);
    if (!codegen_result) {
        ASTHRA_TEST_ASSERT(context, false, "Code generation should succeed");
        return result;
    }

    // Get output by emitting assembly
    const size_t buffer_size = 65536;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        ASTHRA_TEST_ASSERT(context, false, "Failed to allocate assembly buffer");
        return result;
    }

    if (!code_generator_emit_assembly(result.codegen, buffer, buffer_size)) {
        free(buffer);
        ASTHRA_TEST_ASSERT(context, false, "Failed to emit assembly");
        return result;
    }

    // Debug: Print the generated assembly
    printf("=== Generated Assembly ===\n%s\n=== End Assembly ===\n", buffer);

    result.output = buffer; // Caller should free this

    result.success = true;
    return result;
}

void cleanup_codegen_result(CodegenTestResult *result) {
    if (!result)
        return;

    if (result->codegen) {
        code_generator_destroy(result->codegen);
        result->codegen = NULL;
    }

    if (result->analyzer) {
        semantic_analyzer_destroy(result->analyzer);
        result->analyzer = NULL;
    }

    if (result->program) {
        ast_free_node(result->program);
        result->program = NULL;
    }

    if (result->parser) {
        cleanup_parser(result->parser);
        result->parser = NULL;
    }

    // Free the output buffer if it was allocated
    if (result->output) {
        free((void *)result->output);
        result->output = NULL;
    }
    result->success = false;
}
