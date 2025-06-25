/*
 * Generic Structs Phase 5 Core Implementation - Fixed Version
 * Provides compilation pipeline functions for generic struct integration tests
 */

#include "ast_operations.h"
#include "parser_string_interface.h"
#include "test_generic_structs_phase5_common.h"

// Global test counters
size_t tests_run = 0;
size_t tests_passed = 0;

// ============================================================================
// COMPILATION PIPELINE IMPLEMENTATION
// ============================================================================

/**
 * Perform complete compilation pipeline from source to C code
 */
CompilationResult *compile_source(const char *source) {
    CompilationResult *result = calloc(1, sizeof(CompilationResult));
    if (!result)
        return NULL;

    // Step 1: Parse the source using the string parser interface
    ParseResult parse_result = parse_string(source);
    if (!parse_result.success) {
        result->error_message =
            strdup(parse_result.errors && parse_result.error_count > 0 ? parse_result.errors[0]
                                                                       : "Failed to parse source");
        return result;
    }
    result->ast = parse_result.ast;

    // Step 2: Semantic analysis
    result->analyzer = semantic_analyzer_create();
    if (!result->analyzer) {
        result->error_message = strdup("Failed to create semantic analyzer");
        return result;
    }

    bool semantic_success = semantic_analyze_program(result->analyzer, result->ast);
    if (!semantic_success) {
        result->error_message = strdup("Semantic analysis failed");
        return result;
    }

    // Step 3: Code generation
    result->generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!result->generator) {
        result->error_message = strdup("Failed to create code generator");
        return result;
    }

    bool codegen_success = code_generate_program(result->generator, result->ast);
    if (!codegen_success) {
        result->error_message = strdup("Code generation failed");
        return result;
    }

    // Step 4: Generate C code for generic instantiations
    char *c_output_buffer = malloc(8192);
    if (c_output_buffer) {
        bool c_gen_success =
            code_generate_all_generic_instantiations(result->generator, c_output_buffer, 8192);
        if (c_gen_success) {
            result->c_code_output = c_output_buffer;
        } else {
            free(c_output_buffer);
            result->error_message = strdup("C code generation failed");
        }
    }

    result->success = true;
    return result;
}

/**
 * Clean up compilation result and free all resources
 */
void cleanup_compilation_result(CompilationResult *result) {
    if (!result)
        return;

    if (result->ast) {
        ast_free_node(result->ast);
    }
    if (result->analyzer) {
        semantic_analyzer_destroy(result->analyzer);
    }
    if (result->generator) {
        code_generator_destroy(result->generator);
    }
    free(result->c_code_output);
    free(result->error_message);
    free(result);
}