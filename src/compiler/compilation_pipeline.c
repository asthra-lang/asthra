/**
 * Asthra Programming Language Compiler
 * Compilation pipeline - file compilation and multi-file handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../platform.h"
#include "../compiler.h"
#include "../codegen/backend_interface.h"

// Platform-specific includes for system() return value handling
#if ASTHRA_PLATFORM_UNIX
#include <sys/wait.h>
#endif
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "../analysis/semantic_analyzer.h"
#include "../parser/ast.h"

// Forward declarations
extern int generate_c_code(FILE *output, ASTNode *node);
static int asthra_compile_file_to_c(AsthraCompilerContext *ctx, const char *input_file, const char *output_c_file);

// =============================================================================
// COMPILATION FUNCTIONS
// =============================================================================

int asthra_compile_file(AsthraCompilerContext *ctx, const char *input_file, const char *output_file) {
    if (!ctx || !input_file || !output_file) {
        return -1;
    }

    printf("Compiling %s -> %s\n", input_file, output_file);

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
                printf("    Line %d, Col %d: %s\n", error->location.line, error->location.column, error->message);
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
    
    // Validate backend type and provide helpful error messages
    const char *backend_name = asthra_get_backend_type_string(ctx->options.backend_type);
    printf("    Using %s backend\n", backend_name);
    
    // Create backend based on compiler options
    AsthraBackend *backend = asthra_backend_create(&ctx->options);
    if (!backend) {
        printf("Error: Failed to create %s backend\n", backend_name);
        printf("Suggestion: Try using a different backend with --emit-asm or --emit-llvm\n");
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }
    
    // Initialize backend with detailed error reporting
    int init_result = asthra_backend_initialize(backend, &ctx->options);
    if (init_result != 0) {
        const char *error_msg = asthra_backend_get_last_error(backend);
        printf("Error: Failed to initialize %s backend: %s\n", backend_name, error_msg);
        
        // Provide specific suggestions based on backend type
        switch (ctx->options.backend_type) {
            case ASTHRA_BACKEND_LLVM_IR:
                printf("Suggestion: LLVM backend requires LLVM to be installed and linked. Try --emit-asm or the default C backend.\n");
                break;
            case ASTHRA_BACKEND_ASSEMBLY:
                printf("Suggestion: Assembly backend may not support your target architecture. Try the default C backend.\n");
                break;
            case ASTHRA_BACKEND_C:
                printf("Suggestion: C backend should always work. Check system resources and permissions.\n");
                break;
            default:
                printf("Suggestion: Try using the default C backend without additional flags.\n");
                break;
        }
        
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }
    
    // Determine output filename based on backend
    char *backend_output_file = NULL;
    if (ctx->options.backend_type == ASTHRA_BACKEND_C) {
        // For C backend, generate temporary file for compilation
        backend_output_file = strdup("temp_asthra_output.c");
    } else {
        // For other backends, use the final output file directly
        backend_output_file = asthra_backend_get_output_filename(ctx->options.backend_type, 
                                                                 input_file, 
                                                                 output_file);
    }
    
    // Generate code using backend with progress reporting
    printf("    Generating code with %s backend...\n", backend_name);
    int gen_result = asthra_backend_generate(backend, ctx, program, backend_output_file);
    if (gen_result != 0) {
        const char *error_msg = asthra_backend_get_last_error(backend);
        printf("Error: Code generation failed: %s\n", error_msg);
        
        // Provide backend-specific troubleshooting
        switch (ctx->options.backend_type) {
            case ASTHRA_BACKEND_LLVM_IR:
                printf("Troubleshooting: LLVM IR generation failed. Check that LLVM is properly installed.\n");
                printf("  Try: --emit-asm or remove backend flags to use default C backend\n");
                break;
            case ASTHRA_BACKEND_ASSEMBLY:
                printf("Troubleshooting: Assembly generation failed. This may be due to unsupported features.\n");
                printf("  Try: Remove --emit-asm flag to use default C backend\n");
                break;
            case ASTHRA_BACKEND_C:
                printf("Troubleshooting: C code generation failed. This may indicate an AST issue.\n");
                printf("  Check: Input file syntax and semantic correctness\n");
                break;
        }
        
        free(backend_output_file);
        asthra_backend_destroy(backend);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }
    
    // Report backend statistics and success
    size_t lines, functions;
    double time;
    asthra_backend_get_stats(backend, &lines, &functions, &time);
    printf("  ✓ Code generation completed successfully\n");
    if (ctx->options.verbose) {
        printf("    Statistics: %zu lines, %zu functions, %.3f seconds\n", 
               lines, functions, time);
        printf("    Backend: %s version %s\n", 
               asthra_backend_get_name(backend), asthra_backend_get_version(backend));
    }

    // Phase 6: Linking (only for C backend)
    int result = 0;
    if (ctx->options.backend_type == ASTHRA_BACKEND_C) {
        printf("  Phase 6: Linking\n");
        
        // Compile the generated C file
        char compile_cmd[512];
        // TODO: Link with runtime library once path resolution is implemented
        // For now, just compile without runtime linking (will fail at runtime if args() is called)
        snprintf(compile_cmd, sizeof(compile_cmd), "cc -o %s %s", output_file, backend_output_file);
        result = system(compile_cmd);
        
        // Clean up temporary C file
        remove(backend_output_file);
    } else {
        // For non-C backends, no linking phase needed
        printf("  Phase 6: Linking (skipped for %s backend)\n", 
               asthra_backend_get_name(backend));
    }
    
    // Clean up
    free(backend_output_file);
    asthra_backend_destroy(backend);
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source_code);
    
    if (result == -1) {
        printf("Error: Failed to execute compile command\n");
        return -1;
    }
    
#if ASTHRA_PLATFORM_UNIX
    if (!WIFEXITED(result) || WEXITSTATUS(result) != 0) {
        printf("Error: Failed to compile generated C code\n");
        return -1;
    }
#else
    // On Windows, system() returns the exit code directly
    if (result != 0) {
        printf("Error: Failed to compile generated C code (exit code: %d)\n", result);
        return -1;
    }
#endif

    printf("Compilation completed successfully\n");
    return 0;
}

int asthra_compile_files(AsthraCompilerContext *ctx, const char **input_files, size_t file_count, const char *output_file) {
    if (!ctx || !input_files || file_count == 0 || !output_file) {
        return -1;
    }

    printf("Compiling %zu files -> %s\n", file_count, output_file);

    // For now, implement multi-file compilation by compiling each file to C
    // and then linking them together
    
    // Phase 1: Compile each file to C
    printf("  Phase 1: Compiling source files to C\n");
    
    char **temp_c_files = malloc(file_count * sizeof(char*));
    if (!temp_c_files) {
        printf("Error: Memory allocation failed\n");
        return -1;
    }
    
    for (size_t i = 0; i < file_count; i++) {
        temp_c_files[i] = malloc(256);
        if (!temp_c_files[i]) {
            for (size_t j = 0; j < i; j++) {
                free(temp_c_files[j]);
            }
            free(temp_c_files);
            printf("Error: Memory allocation failed\n");
            return -1;
        }
        
        snprintf(temp_c_files[i], 256, "temp_asthra_%zu.c", i);
        
        printf("    Compiling %s to %s\n", input_files[i], temp_c_files[i]);
        
        // Use the single file compilation logic
        if (asthra_compile_file_to_c(ctx, input_files[i], temp_c_files[i]) != 0) {
            printf("Error: Failed to compile %s\n", input_files[i]);
            
            // Clean up
            for (size_t j = 0; j <= i; j++) {
                remove(temp_c_files[j]);
                free(temp_c_files[j]);
            }
            for (size_t j = i + 1; j < file_count; j++) {
                free(temp_c_files[j]);
            }
            free(temp_c_files);
            return -1;
        }
    }
    
    // Phase 2: Link all C files together
    printf("  Phase 2: Linking object files\n");
    
    // Build the compile command
    char compile_cmd[4096];
    int offset = snprintf(compile_cmd, sizeof(compile_cmd), "cc -o %s", output_file);
    
    for (size_t i = 0; i < file_count; i++) {
        offset += snprintf(compile_cmd + offset, sizeof(compile_cmd) - offset, " %s", temp_c_files[i]);
    }
    
    // Execute the linking command
    int result = system(compile_cmd);
    
    // Clean up temporary files
    for (size_t i = 0; i < file_count; i++) {
        remove(temp_c_files[i]);
        free(temp_c_files[i]);
    }
    free(temp_c_files);
    
    if (result == -1) {
        printf("Error: Failed to execute linking command\n");
        return -1;
    }
    
#if ASTHRA_PLATFORM_UNIX
    if (!WIFEXITED(result) || WEXITSTATUS(result) != 0) {
        printf("Error: Linking failed\n");
        return -1;
    }
#else
    // On Windows, system() returns the exit code directly
    if (result != 0) {
        printf("Error: Linking failed (exit code: %d)\n", result);
        return -1;
    }
#endif
    
    printf("Multi-file compilation completed successfully\n");
    return 0;
}

// Helper function to compile a single file to C
static int asthra_compile_file_to_c(AsthraCompilerContext *ctx, const char *input_file, const char *output_c_file) {
    // Read source file
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

    // Create lexer
    Lexer *lexer = lexer_create(source_code, bytes_read, input_file);
    if (!lexer) {
        free(source_code);
        printf("Error: Failed to create lexer\n");
        return -1;
    }

    // Create parser
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

    // Semantic analysis
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (!analyzer) {
        printf("Error: Failed to create semantic analyzer\n");
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }
    
    bool semantic_success = semantic_analyze_program(analyzer, program);
    if (!semantic_success) {
        printf("Error: Semantic analysis failed\n");
        if (analyzer->error_count > 0) {
            printf("  Semantic errors:\n");
            SemanticError *error = analyzer->errors;
            while (error) {
                printf("    Line %d, Col %d: %s\n", error->location.line, error->location.column, error->message);
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

    // Generate C code
    FILE *output_file = fopen(output_c_file, "w");
    if (!output_file) {
        printf("Error: Failed to create output file '%s'\n", output_c_file);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }
    
    // Write C header (only for the first file to avoid duplicates)
    static bool headers_written = false;
    if (!headers_written) {
        fprintf(output_file, "#include <stdio.h>\n");
        fprintf(output_file, "#include <stdlib.h>\n");
        fprintf(output_file, "#include <string.h>\n");
        fprintf(output_file, "#include <stddef.h>\n");
        fprintf(output_file, "#include <stdbool.h>\n");
        fprintf(output_file, "#include <stdint.h>\n");
        fprintf(output_file, "\n");
        fprintf(output_file, "// Asthra runtime types and functions for slice support\n");
        fprintf(output_file, "typedef enum {\n");
        fprintf(output_file, "    ASTHRA_OWNERSHIP_GC,\n");
        fprintf(output_file, "    ASTHRA_OWNERSHIP_C,\n");
        fprintf(output_file, "    ASTHRA_OWNERSHIP_PINNED\n");
        fprintf(output_file, "} AsthraOwnershipHint;\n");
        fprintf(output_file, "\n");
        fprintf(output_file, "typedef struct {\n");
        fprintf(output_file, "    void * restrict ptr;\n");
        fprintf(output_file, "    size_t len;\n");
        fprintf(output_file, "    size_t cap;\n");
        fprintf(output_file, "    size_t element_size;\n");
        fprintf(output_file, "    AsthraOwnershipHint ownership;\n");
        fprintf(output_file, "    bool is_mutable;\n");
        fprintf(output_file, "    uint32_t type_id;\n");
        fprintf(output_file, "} AsthraSliceHeader;\n");
        fprintf(output_file, "\n");
        fprintf(output_file, "// Slice operation functions\n");
        fprintf(output_file, "static inline size_t asthra_slice_get_len(AsthraSliceHeader slice) { return slice.len; }\n");
        fprintf(output_file, "static inline void* asthra_slice_get_element(AsthraSliceHeader slice, size_t index) {\n");
        fprintf(output_file, "    if (index >= slice.len) return NULL;\n");
        fprintf(output_file, "    return (char*)slice.ptr + index * slice.element_size;\n");
        fprintf(output_file, "}\n");
        fprintf(output_file, "static inline AsthraSliceHeader asthra_slice_subslice(AsthraSliceHeader slice, size_t start, size_t end) {\n");
        fprintf(output_file, "    if (start > slice.len) start = slice.len;\n");
        fprintf(output_file, "    if (end > slice.len) end = slice.len;\n");
        fprintf(output_file, "    if (start > end) end = start;\n");
        fprintf(output_file, "    return (AsthraSliceHeader){.ptr = (char*)slice.ptr + start * slice.element_size,\n");
        fprintf(output_file, "                               .len = end - start, .cap = end - start,\n");
        fprintf(output_file, "                               .element_size = slice.element_size,\n");
        fprintf(output_file, "                               .ownership = slice.ownership,\n");
        fprintf(output_file, "                               .is_mutable = slice.is_mutable,\n");
        fprintf(output_file, "                               .type_id = slice.type_id};\n");
        fprintf(output_file, "}\n\n");
        headers_written = true;
    }
    
    // Generate code from AST
    if (generate_c_code(output_file, program) != 0) {
        printf("Error: Code generation failed\n");
        fclose(output_file);
        remove(output_c_file);
        semantic_analyzer_destroy(analyzer);
        ast_free_node(program);
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source_code);
        return -1;
    }
    
    fclose(output_file);
    
    // Clean up
    semantic_analyzer_destroy(analyzer);
    ast_free_node(program);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source_code);
    
    return 0;
}

// =============================================================================
// COMPILATION PHASE STUBS
// =============================================================================

// TODO: These compilation phase stubs will be implemented when the modular
// pipeline orchestrator is fully integrated. For now, the main compilation
// logic is handled directly in asthra_compile_file(). 
