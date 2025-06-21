/*
 * Core Module for Generic Structs Phase 5 Integration Tests
 * Compilation pipeline functions and shared utilities
 * 
 * Part of test_generic_structs_phase5.c split (580 lines -> 6 focused modules)
 * Provides complete compilation pipeline from source to C code generation
 */

#include "test_generic_structs_phase5_common.h"

// ============================================================================
// GLOBAL TEST STATISTICS
// ============================================================================

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
    if (!result) return NULL;
    
    // Step 1: Parse the source using the string parser interface
    ParseResult parse_result = parse_string(source);
    if (!parse_result.success) {
        result->error_message = strdup(parse_result.error ? parse_result.error : "Failed to parse source");
        return result;
    }
    result->ast = parse_result.ast;
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        result->error_message = strdup("Failed to create parser");
        return result;
    }
    
    result->ast = parse_program(parser);
    if (!result->ast) {
        parser_destroy(parser);
        lexer_destroy(lexer);
        result->error_message = strdup("Failed to parse source");
        return result;
    }
    
    // Step 2: Semantic analysis
    result->analyzer = semantic_analyzer_create();
    if (!result->analyzer) {
        free_ast_node(result->ast);
        result->ast = NULL;
        parser_destroy(parser);
        lexer_destroy(lexer);
        result->error_message = strdup("Failed to create semantic analyzer");
        return result;
    }
    
    bool semantic_success = semantic_analyze_program(result->analyzer, result->ast);
    if (!semantic_success) {
        result->error_message = strdup("Semantic analysis failed");
        // Don't cleanup yet - caller might want to examine partial results
        parser_destroy(parser);
        lexer_destroy(lexer);
        return result;
    }
    
    // Step 3: Code generation
    result->generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!result->generator) {
        result->error_message = strdup("Failed to create code generator");
        parser_destroy(parser);
        lexer_destroy(lexer);
        return result;
    }
    
    bool codegen_success = code_generate_program(result->generator, result->ast);
    if (!codegen_success) {
        result->error_message = strdup("Code generation failed");
        parser_destroy(parser);
        lexer_destroy(lexer);
        return result;
    }
    
    // Step 4: Generate C code for generic instantiations
    char *c_output_buffer = malloc(8192);
    if (c_output_buffer) {
        bool c_gen_success = code_generate_all_generic_instantiations(result->generator, 
                                                                     c_output_buffer, 8192);
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
    if (!result) return;
    
    if (result->ast) {
        free_ast_node(result->ast);
    }
    if (result->analyzer) {
        semantic_analyzer_destroy(result->analyzer);
    }
    if (result->generator) {
        code_generator_destroy(result->generator);
    }
    if (result->c_code_output) {
        free(result->c_code_output);
    }
    if (result->error_message) {
        free(result->error_message);
    }
    free(result);
} 
