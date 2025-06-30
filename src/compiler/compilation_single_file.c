/**
 * Asthra Programming Language Compiler
 * Single file compilation implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../analysis/semantic_analyzer.h"
#include "../codegen/llvm_backend.h"
#include "../codegen/llvm_tools.h"
#include "../compiler.h"
#include "../parser/ast.h"
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "../platform.h"

// =============================================================================
// SINGLE FILE COMPILATION
// =============================================================================

int asthra_compile_file(AsthraCompilerContext *ctx, const char *input_file,
                        const char *output_file) {
    if (!ctx || !input_file || !output_file) {
        return -1;
    }

    printf("Compiling %s -> %s\n", input_file, output_file);
    printf("Debug: Output format in compile_file: %d\n", ctx->options.output_format);

    // Phase 1: Read source file
    printf("  Phase 1: Reading source file\n");
    FILE *source_file = fopen(input_file, "r");
    if (!source_file) {
        printf("Error: Cannot open input file '%s'\n", input_file);
        return -1;
    }

    // Get file size
    fseek(source_file, 0, SEEK_END);
    long file_size = ftell(source_file);
    fseek(source_file, 0, SEEK_SET);

    // Read entire file
    char *source_code = malloc((size_t)(file_size + 1));
    if (!source_code) {
        fclose(source_file);
        printf("Error: Memory allocation failed\n");
        return -1;
    }

    size_t bytes_read = fread(source_code, 1, (size_t)file_size, source_file);
    source_code[bytes_read] = '\0';
    fclose(source_file);

    // Phase 2: Lexical analysis
    printf("  Phase 2: Lexical analysis\n");
    Lexer *lexer = lexer_create(source_code, bytes_read, input_file);
    if (!lexer) {
        free(source_code);
        printf("Error: Failed to create lexer\n");
        return -1;
    }

    // Phase 3: Syntax analysis
    printf("  Phase 3: Syntax analysis\n");
    Parser *parser = parser_create(lexer);
    if (!parser) {
        lexer_destroy(lexer);
        free(source_code);
        printf("Error: Failed to create parser\n");
        return -1;
    }

    // Parse the program
    ASTNode *program = parser_parse_program(parser);
    if (!program || parser_had_error(parser)) {
        printf("Error: Parsing failed\n");
        if (parser_get_error_count(parser) > 0) {
            printf("Parser errors occurred during compilation\n");
        }
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }

    // Phase 4: Semantic analysis
    printf("  Phase 4: Semantic analysis\n");

    // Create semantic analyzer
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Error: Failed to create semantic analyzer\n");
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }

    // Perform semantic analysis on the program
    bool semantic_success = semantic_analyze_program(analyzer, program);
    if (!semantic_success) {
        printf("Error: Semantic analysis failed\n");
        // Print semantic errors if any
        if (analyzer->error_count > 0) {
            printf("  Semantic errors:\n");
            SemanticError *error = analyzer->errors;
            while (error) {
                printf("    Line %d, Col %d: %s\n", error->location.line, error->location.column,
                       error->message);
                error = error->next;
            }
        }
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }

    printf("  ✓ Semantic analysis completed successfully\n");

    // Phase 5: Code generation
    printf("  Phase 5: Code generation\n");

    // Using LLVM backend (only backend now)
    printf("    Using LLVM IR backend\n");

    // Determine output filename based on backend
    char *backend_output_file = NULL;
    if (ctx->options.coverage && ctx->options.output_format == ASTHRA_FORMAT_DEFAULT) {
        // For LLVM backend with coverage, always generate a temporary .ll file first
        char temp_name[256];
        snprintf(temp_name, sizeof(temp_name), "%s.tmp.ll", output_file);
        backend_output_file = strdup(temp_name);
    } else if (output_file) {
        // Generate temporary .ll file name based on output file
        char temp_name[512];
        snprintf(temp_name, sizeof(temp_name), "%s.tmp.ll", output_file);
        backend_output_file = strdup(temp_name);
    } else {
        // No output file specified, use default
        backend_output_file =
            asthra_backend_get_output_filename(0, input_file, NULL);
    }

    printf("Debug: backend_output_file = '%s', output_file = '%s'\n", backend_output_file,
           output_file);

    // Generate code using LLVM backend
    printf("    Generating LLVM IR code...\n");
    int gen_result = asthra_generate_llvm_code(ctx, program, backend_output_file);
    if (gen_result != 0) {
        printf("Error: LLVM code generation failed\n");
        printf("Troubleshooting: Check that LLVM is properly installed.\n");

        free(backend_output_file);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }

    // Code generation successful
    printf("  ✓ LLVM IR generation completed successfully\n");
    if (ctx->options.verbose) {
        printf("    Backend: LLVM\n");
    }

    // Phase 6: Post-processing with LLVM tools (if needed)
    int result = 0;

    // Check if we need to use LLVM tools for the output format
    AsthraOutputFormat final_format = ctx->options.output_format;
    if (final_format == ASTHRA_FORMAT_DEFAULT) {
        // Determine format from output file extension
        const char *ext = strrchr(output_file, '.');
        if (ext) {
            if (strcmp(ext, ".ll") == 0) {
                final_format = ASTHRA_FORMAT_LLVM_IR;
            } else if (strcmp(ext, ".bc") == 0) {
                final_format = ASTHRA_FORMAT_LLVM_BC;
            } else if (strcmp(ext, ".s") == 0) {
                final_format = ASTHRA_FORMAT_ASSEMBLY;
            } else if (strcmp(ext, ".o") == 0) {
                final_format = ASTHRA_FORMAT_OBJECT;
            } else {
                final_format = ASTHRA_FORMAT_EXECUTABLE;
            }
        } else {
            final_format = ASTHRA_FORMAT_EXECUTABLE;
        }
    }

    // LLVM backend produces .ll files by default
    // We need LLVM tools if:
    // 1. The desired format is not LLVM IR, OR
    // 2. The backend output file is different from the final output file
    bool needs_llvm_tools = false;
    if (final_format != ASTHRA_FORMAT_LLVM_IR ||
        strcmp(backend_output_file, output_file) != 0) {
        needs_llvm_tools = true;
    }

    if (needs_llvm_tools) {
        printf("  Phase 6: LLVM tools pipeline\n");

        // Check if LLVM tools are available
        if (!asthra_llvm_tools_available()) {
            printf("Error: LLVM tools (llc, opt, clang) not found in PATH\n");
            printf("Please install LLVM 18.0 or later and ensure tools are in PATH\n");
            result = -1;
        } else {
            // Run LLVM compilation pipeline
            AsthraLLVMToolResult tool_result = asthra_llvm_compile_pipeline(
                backend_output_file, output_file, final_format, &ctx->options);

            if (!tool_result.success) {
                printf("Error: LLVM tools pipeline failed\n");
                if (tool_result.stderr_output) {
                    printf("  Details: %s\n", tool_result.stderr_output);
                }
                result = -1;
            } else {
                printf("  ✓ Output generated successfully: %s\n", output_file);
                if (ctx->options.verbose && tool_result.execution_time_ms > 0) {
                    printf("    LLVM tools execution time: %.3f seconds\n",
                           tool_result.execution_time_ms / 1000.0);
                }
            }

            asthra_llvm_tool_result_free(&tool_result);
        }

        // Clean up temporary IR file if different from output
        if (strcmp(backend_output_file, output_file) != 0) {
            unlink(backend_output_file);
        }
    } else {
        printf("  Phase 6: Output ready (no post-processing needed)\n");
    }

    // Clean up
    free(backend_output_file);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source_code);

    if (result != 0) {
        return -1;
    }

    printf("Compilation completed successfully\n");
    return 0;
}