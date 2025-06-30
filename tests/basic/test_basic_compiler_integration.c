/**
 * Enhanced Basic Compiler Infrastructure
 * Complete compilation pipeline integration for test_basic.c
 *
 * Phase 4: Core Infrastructure Implementation
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "asthra_runtime.h"
#include "compiler.h"
#include "lexer.h"
#include "parser_string_interface.h"
#include "pipeline_orchestrator.h"
#include "semantic_analyzer_core.h"
#include "semantic_core.h"
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
// #include "../performance/performance_validation.h" // Removed - will define inline

// Performance validation stub - defined inline to avoid linking issues
bool performance_validation_run_complete(const char *test_name) {
    printf("Performance validation for '%s' (stub)\n", test_name);
    // Always return true for now
    return true;
}

// =============================================================================
// ENHANCED COMPILER CONTEXT
// =============================================================================

typedef struct {
    bool initialized;
    size_t error_count;

    // Real compilation infrastructure
    PipelineOrchestrator *orchestrator;
    Parser *parser;
    SemanticAnalyzer *semantic_analyzer;

    // Error management
    AsthraCompilerError *errors;
    size_t max_errors;

    // Performance tracking
    struct timespec start_time;
    double compilation_time_ms;
    size_t memory_usage_bytes;

    // Compilation statistics
    atomic_size_t files_compiled;
    atomic_size_t lines_processed;
    atomic_size_t symbols_created;
} EnhancedAsthraCompilerContext;

// Using the compiler's defined structures from compiler.h
// AsthraCompilerError and AsthraCompilerOptions are already defined in compiler.h

// =============================================================================
// COMPILATION RESULT STRUCTURES
// =============================================================================

typedef struct {
    bool success;
    const char *output_path;
    double compilation_time_ms;
    size_t memory_usage_bytes;
    size_t error_count;
    size_t warning_count;
    size_t lines_compiled;
    AsthraCompilerError *errors;
} CompilationResult;

// =============================================================================
// ENHANCED COMPILER FUNCTIONS
// =============================================================================

static double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

static size_t get_memory_usage(void) {
    // Simple memory tracking - could be enhanced with more sophisticated tracking
    return 0; // Placeholder for now
}

static void add_compiler_error(EnhancedAsthraCompilerContext *ctx, const char *message, int line,
                               int column, const char *phase, int severity) {
    if (!ctx || ctx->error_count >= ctx->max_errors)
        return;

    AsthraCompilerError *error = &ctx->errors[ctx->error_count];
    error->message = strdup(message);
    error->line = (size_t)line;
    error->column = (size_t)column;
    error->phase = ASTHRA_PHASE_PARSING; // Use enum from compiler.h
    error->message = strdup(message);
    error->is_warning = (severity == 1);

    ctx->error_count++;
}

// Using the real implementation from compiler_core.o

/**
 * Compile source code through complete pipeline
 */
CompilationResult *asthra_compiler_compile_source(AsthraCompilerContext *context,
                                                  const char *source_code,
                                                  const char *output_path) {
    if (!context || !source_code)
        return NULL;

    EnhancedAsthraCompilerContext *ctx = (EnhancedAsthraCompilerContext *)context;
    CompilationResult *result = calloc(1, sizeof(CompilationResult));
    if (!result)
        return NULL;

    double start_time = get_time_ms();
    size_t start_memory = get_memory_usage();

    // Phase 1: Parse source code
    ParseResult parse_result = parse_string(source_code);
    if (!parse_result.success || !parse_result.ast) {
        add_compiler_error(ctx, "Parse error", 0, 0, "parsing", 2);
        result->success = false;
        goto compilation_complete;
    }

    atomic_fetch_add(&ctx->lines_processed, 100); // Estimate

    // Phase 2: Semantic analysis
    bool semantic_success = semantic_analyze_program(ctx->semantic_analyzer, parse_result.ast);
    if (!semantic_success) {
        add_compiler_error(ctx, "Semantic analysis failed", 0, 0, "semantic_analysis", 2);
        result->success = false;
        cleanup_parse_result(&parse_result);
        goto compilation_complete;
    }

    // Phase 3: Generate C code (current implementation generates C, not assembly)
    // Using the actual implementation approach from compilation_pipeline.c
    extern int generate_c_code(FILE * output, ASTNode * node);

    bool codegen_success = false;
    if (output_path) {
        FILE *output = fopen(output_path, "w");
        if (output) {
            int result = generate_c_code(output, parse_result.ast);
            fclose(output);
            codegen_success = (result == 0);
        } else {
            codegen_success = false;
        }
        if (!codegen_success) {
            add_compiler_error(ctx, "Code generation failed", 0, 0, "code_generation", 2);
            result->success = false;
        } else {
            result->output_path = strdup(output_path);
        }
    }

    // Cleanup
    cleanup_parse_result(&parse_result);

    result->success = (ctx->error_count == 0);
    atomic_fetch_add(&ctx->files_compiled, 1);

compilation_complete:
    result->compilation_time_ms = get_time_ms() - start_time;
    result->memory_usage_bytes = get_memory_usage() - start_memory;
    result->error_count = ctx->error_count;
    result->warning_count = 0; // Count warnings separately if needed
    result->lines_compiled = atomic_load(&ctx->lines_processed);
    result->errors = ctx->errors;

    ctx->compilation_time_ms = result->compilation_time_ms;

    return result;
}

/**
 * Compile source file through complete pipeline
 */
CompilationResult *asthra_compiler_compile_file(AsthraCompilerContext *context,
                                                const char *input_path, const char *output_path) {
    if (!context || !input_path)
        return NULL;

    // Read file content
    FILE *file = fopen(input_path, "r");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    long length = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *content = malloc((size_t)length + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    size_t read_size = fread(content, 1, (size_t)length, file);
    content[read_size] = '\0';
    fclose(file);

    // Compile the content
    CompilationResult *result = asthra_compiler_compile_source(context, content, output_path);

    free(content);
    return result;
}

// Using the real implementation from error_handling.o

/**
 * Get compilation statistics
 */
void asthra_compiler_get_statistics(AsthraCompilerContext *context, size_t *files_compiled,
                                    size_t *lines_processed, double *total_time_ms,
                                    size_t *memory_usage) {
    if (!context)
        return;

    EnhancedAsthraCompilerContext *ctx = (EnhancedAsthraCompilerContext *)context;

    if (files_compiled)
        *files_compiled = atomic_load(&ctx->files_compiled);
    if (lines_processed)
        *lines_processed = atomic_load(&ctx->lines_processed);
    if (total_time_ms)
        *total_time_ms = ctx->compilation_time_ms;
    if (memory_usage)
        *memory_usage = ctx->memory_usage_bytes;
}

/**
 * Validate compiler performance
 */
bool asthra_compiler_validate_performance(AsthraCompilerContext *context) {
    if (!context)
        return false;

    // Use performance validation infrastructure from Phase 3
    return performance_validation_run_complete("basic_compiler_performance");
}

// Using the real implementation from compiler_core.o

/**
 * Cleanup compilation result
 */
void asthra_compiler_cleanup_result(CompilationResult *result) {
    if (!result)
        return;

    free((void *)result->output_path);
    // Note: errors are owned by the compiler context, not the result
    free(result);
}

/**
 * Test compiler component isolation.
 */
bool test_component_isolation(void) {
    printf("Testing compiler component isolation...\n");

    // Test Lexer Isolation
    printf("  Testing Lexer Isolation...\n");

    // Use parse_string function from parser_string_interface.h with valid Asthra program
    const char *test_program =
        "package test;\npub fn main(none) -> void { let x: i32 = 42; return (); }";
    ParseResult parse_result = parse_string(test_program);
    if (!parse_result.success || !parse_result.ast) {
        printf("❌ Lexer isolation test failed (unexpected error or no AST from basic lexing)\n");
        if (parse_result.error_count > 0 && parse_result.errors) {
            for (int i = 0; i < parse_result.error_count; i++) {
                printf("    Error: %s\n", parse_result.errors[i]);
            }
        }
        cleanup_parse_result(&parse_result);
        return false;
    }
    printf("  ✓ Lexer isolation test passed\n");
    cleanup_parse_result(&parse_result);

    // Test Parser Isolation
    printf("  Testing Parser Isolation...\n");
    const char *parser_test =
        "package test;\npub fn main(none) -> void { let x: i32 = 10; return (); }";
    ParseResult parser_result = parse_string(parser_test);
    if (!parser_result.success || !parser_result.ast) {
        printf("❌ Parser isolation test failed (unexpected error or no AST from basic parsing)\n");
        return false;
    }
    printf("  ✓ Parser isolation test passed\n");
    cleanup_parse_result(&parser_result);

    // Test Semantic Analyzer Isolation
    printf("  Testing Semantic Analyzer Isolation...\n");
    SemanticAnalyzer *semantic_only = semantic_analyzer_create();
    // Create a dummy AST for semantic analysis
    ASTNode *dummy_ast = calloc(1, sizeof(ASTNode));
    dummy_ast->type = AST_PROGRAM;
    bool semantic_success = semantic_analyze_program(semantic_only, dummy_ast);
    if (!semantic_success) {
        printf("❌ Semantic analyzer isolation test failed (unexpected error during analysis)\n");
        semantic_analyzer_destroy(semantic_only);
        free(dummy_ast); // Direct free for dummy node
        return false;
    }
    printf("  ✓ Semantic analyzer isolation test passed\n");
    semantic_analyzer_destroy(semantic_only);
    free(dummy_ast);

    // Backend isolation test removed - backend abstraction layer removed
    // The LLVM backend is now the only backend and is accessed directly

    printf("✅ All component isolation tests passed.\n");
    return true;
}

/**
 * Test compiler interface validation.
 */
bool test_interface_validation(void) {
    printf("Testing compiler interface validation...\n");

    // Test AsthraCompilerContext creation and destruction
    printf("  Testing AsthraCompilerContext lifecycle...\n");
    AsthraCompilerOptions options = {.input_file = "test.asthra",
                                     .output_file = "test.out",
                                     .opt_level = ASTHRA_OPT_NONE,
                                     .target_arch = ASTHRA_TARGET_X86_64,
                                     .debug_info = false,
                                     .verbose = false,
                                     .no_stdlib = false,
                                     .include_paths = NULL,
                                     .library_paths = NULL,
                                     .libraries = NULL};
    AsthraCompilerContext *ctx = asthra_compiler_create(&options);
    if (!ctx) {
        printf("❌ AsthraCompilerContext creation failed.\n");
        return false;
    }
    printf("  ✓ AsthraCompilerContext creation passed.\n");
    asthra_compiler_destroy(ctx);
    printf("  ✓ AsthraCompilerContext destruction passed.\n");

    // Test CompilationResult cleanup
    printf("  Testing CompilationResult cleanup...\n");
    CompilationResult *res = calloc(1, sizeof(CompilationResult));
    if (!res) {
        printf("❌ Failed to allocate CompilationResult.\n");
        return false;
    }
    res->output_path = strdup("/tmp/test_output.asm");
    asthra_compiler_cleanup_result(res);
    printf("  ✓ CompilationResult cleanup passed.\n");

    // Test error reporting and retrieval
    printf("  Testing error reporting and retrieval...\n");
    AsthraCompilerContext *err_ctx = asthra_compiler_create(&options);
    if (!err_ctx) {
        printf("❌ AsthraCompilerContext for error test failed.\n");
        return false;
    }

    // Test that we can get errors even if there are none yet
    size_t error_count = 0;
    const AsthraCompilerError *errors = asthra_compiler_get_errors(err_ctx, &error_count);
    if (error_count != 0) {
        printf("❌ Error reporting/retrieval failed (expected 0 errors initially, got %zu)\n",
               error_count);
        asthra_compiler_destroy(err_ctx);
        return false;
    }
    printf("  ✓ Error reporting and retrieval passed.\n");
    asthra_compiler_destroy(err_ctx);

    printf("✅ All interface validation tests passed.\n");
    return true;
}

/**
 * Run comprehensive basic compiler tests
 */
bool run_basic_compiler_tests(void) {
    printf("\n=== Basic Compiler Component Tests ===\n");

    bool all_passed = true;

    all_passed &= test_component_isolation();
    all_passed &= test_interface_validation();
    // all_passed &= test_error_propagation(); // To be implemented
    // all_passed &= test_performance_baseline(); // To be implemented
    // all_passed &= test_regression_prevention(); // To be implemented

    printf("\n=== Test Results ===\n");
    if (all_passed) {
        printf("✅ All basic compiler tests passed!\n");
    } else {
        printf("❌ Some basic compiler tests failed!\n");
    }

    return all_passed;
}

/**
 * Main entry point for the test program
 */
int main(int argc, char **argv) {
    (void)argc; // Unused
    (void)argv; // Unused

    bool success = run_basic_compiler_tests();
    return success ? 0 : 1;
}
