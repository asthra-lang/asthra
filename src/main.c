/**
 * Asthra Programming Language Compiler
 * Main entry point - Simplified after modular split
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "platform_utils.h"
#include "compiler.h"
#include "version.h"
#include "../runtime/asthra_runtime.h"

// C17 compile-time validation of main.c assumptions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits");
ASTHRA_STATIC_ASSERT(sizeof(char*) >= sizeof(void*), "char* must be at least as large as void*");

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main(int argc, char *argv[]) {
    // Initialize runtime
    asthra_runtime_init(NULL);
    
    int result = 1; // Default to error
    char* normalized_input = NULL;
    char* output_filename = NULL;
    AsthraCompilerContext *ctx = NULL;
    
    // Initialize CLI options
    CliOptions cli_options = cli_options_init();
    
    // Parse command line arguments
    int parse_result = cli_parse_arguments(argc, argv, &cli_options);
    if (parse_result != 0) {
        result = cli_options.exit_code;
        goto cleanup_and_exit;
    }
    
    // Handle special cases (help, version)
    if (cli_options.show_help) {
        cli_print_usage(argv[0]);
        result = 0;
        goto cleanup_and_exit;
    }
    
    if (cli_options.show_version) {
        cli_print_version();
        result = 0;
        goto cleanup_and_exit;
    }
    
    // Normalize the input file path for the platform
    normalized_input = asthra_strdup(cli_options.compiler_options.input_file);
    if (normalized_input) {
        asthra_normalize_path(normalized_input);
        cli_options.compiler_options.input_file = normalized_input;
    }

    // Generate output filename if not specified
    output_filename = platform_build_output_filename(
        cli_options.compiler_options.input_file, 
        cli_options.compiler_options.output_file
    );
    if (!output_filename) {
        fprintf(stderr, "Error: Failed to generate output filename\n");
        goto cleanup_and_exit;
    }
    cli_options.compiler_options.output_file = output_filename;

    // Validate options
    if (!asthra_compiler_validate_options(&cli_options.compiler_options)) {
        fprintf(stderr, "Error: Invalid compiler options\n");
        goto cleanup_and_exit;
    }

    // Check if input file exists
    if (!asthra_file_exists(cli_options.compiler_options.input_file)) {
        fprintf(stderr, "Error: Input file '%s' does not exist\n", 
                cli_options.compiler_options.input_file);
        goto cleanup_and_exit;
    }

    if (cli_options.compiler_options.verbose) {
        printf("Asthra Compiler %s\n", ASTHRA_VERSION_STRING);
        printf("Platform: %s\n", asthra_get_platform_info());
        printf("Input file: %s\n", cli_options.compiler_options.input_file);
        printf("Output file: %s\n", cli_options.compiler_options.output_file);
        printf("Target: %s\n", asthra_get_target_triple(cli_options.compiler_options.target_arch));
        printf("Optimization: %s\n", asthra_get_optimization_level_string(cli_options.compiler_options.opt_level));
    }

    // Create compiler context
    ctx = asthra_compiler_create(&cli_options.compiler_options);
    if (!ctx) {
        fprintf(stderr, "Error: Failed to create compiler context\n");
        goto cleanup_and_exit;
    }
    
    // Debug: Print output format
    if (cli_options.compiler_options.verbose) {
        printf("Output format: %d\n", cli_options.compiler_options.output_format);
    }

    if (cli_options.test_mode) {
        // Run in test mode
        printf("Running in test mode...\n");
        // TODO: Implement test mode functionality
        result = 0;
    } else {
        // Compile the file
        result = asthra_compile_file(ctx, 
                                   cli_options.compiler_options.input_file, 
                                   cli_options.compiler_options.output_file);
        
        if (result != 0) {
            // Print compilation errors
            size_t error_count;
            const AsthraCompilerError *errors = asthra_compiler_get_errors(ctx, &error_count);
            
            for (size_t i = 0; i < error_count; i++) {
                const AsthraCompilerError *error = &errors[i];
                fprintf(stderr, "%s:%zu:%zu: %s: %s\n",
                       error->file ? error->file : "<unknown>",
                       error->line,
                       error->column,
                       error->is_warning ? "warning" : "error",
                       error->message);
            }
        } else if (cli_options.compiler_options.verbose) {
            printf("Compilation successful!\n");
        }
    }

cleanup_and_exit:
    // Cleanup compiler context
    if (ctx) {
        asthra_compiler_destroy(ctx);
    }
    
    // Cleanup CLI options
    cli_options_cleanup(&cli_options);
    
    // Free allocated strings
    if (normalized_input) {
        free(normalized_input);
    }
    if (output_filename) {
        free(output_filename);
    }

    return result;
} 
