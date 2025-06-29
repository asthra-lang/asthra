/**
 * Asthra Programming Language LLVM Pipeline
 * Implementation of integrated compilation pipeline
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_pipeline.h"
#include "llvm_compilation.h"
#include "llvm_linking.h"
#include "llvm_optimization.h"
#include "llvm_process_execution.h"
#include "llvm_tool_detection.h"
#include "llvm_utilities.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

// =============================================================================
// PIPELINE INTEGRATION
// =============================================================================

AsthraLLVMToolResult asthra_llvm_compile_pipeline(const char *ir_file, const char *output_file,
                                                  AsthraOutputFormat format,
                                                  const AsthraCompilerOptions *options) {
    AsthraLLVMToolResult result = {0};

    // If output is LLVM IR and no optimization, just copy the file
    if (format == ASTHRA_FORMAT_LLVM_IR && options->opt_level == ASTHRA_OPT_NONE) {
        // Simple file copy
        FILE *src = fopen(ir_file, "r");
        FILE *dst = fopen(output_file, "w");
        if (src && dst) {
            char buffer[4096];
            size_t n;
            while ((n = fread(buffer, 1, sizeof(buffer), src)) > 0) {
                fwrite(buffer, 1, n, dst);
            }
            fclose(src);
            fclose(dst);
            result.success = true;
            return result;
        }
        result.stderr_output = strdup("Failed to copy IR file");
        return result;
    }

    // Temporary file for optimization output
    char *optimized_file = NULL;
    const char *compile_input = ir_file;

    // Run optimization if requested
    if (options->opt_level > ASTHRA_OPT_NONE) {
        optimized_file = asthra_llvm_get_output_filename(ir_file, ASTHRA_FORMAT_LLVM_BC);
        AsthraLLVMToolOptions opt_options = {.output_file = optimized_file,
                                             .opt_level = options->opt_level,
                                             .debug_info = options->debug_info,
                                             .verbose = options->verbose,
                                             .coverage = options->coverage,
                                             .emit_llvm_after_opt =
                                                 (format == ASTHRA_FORMAT_LLVM_IR)};

        result = asthra_llvm_optimize(ir_file, &opt_options);
        if (!result.success) {
            free(optimized_file);
            return result;
        }

        compile_input = optimized_file;
    }

    // If final output is LLVM IR, we're done
    if (format == ASTHRA_FORMAT_LLVM_IR) {
        if (optimized_file) {
            // Move optimized file to output
            if (rename(optimized_file, output_file) != 0) {
                result.success = false;
                result.stderr_output = strdup("Failed to move optimized IR to output");
            }
            free(optimized_file);
        }
        return result;
    }

    // Generate assembly or object code
    if (format == ASTHRA_FORMAT_ASSEMBLY || format == ASTHRA_FORMAT_OBJECT) {
        AsthraLLVMToolOptions compile_options = {
            .output_format = format,
            .output_file = output_file,
            .opt_level = ASTHRA_OPT_NONE, // Already optimized
            .target_arch = options->target_arch,
            .target_triple = asthra_llvm_target_triple(options->target_arch),
            .debug_info = options->debug_info,
            .verbose = options->verbose,
            .coverage = options->coverage};

        result = asthra_llvm_compile(compile_input, &compile_options);
    }

    // Generate executable
    else if (format == ASTHRA_FORMAT_EXECUTABLE) {
        // If coverage is enabled, use clang directly to compile IR to executable
        // This ensures proper coverage instrumentation
        if (options->coverage) {
            const char *clang_path = asthra_llvm_tool_path("clang");
            if (!clang_path) {
                result.stderr_output = strdup("clang tool not found in PATH");
                return result;
            }

            // Build command for clang with coverage
            const char *argv[32];
            int argc = 0;

            argv[argc++] = clang_path;
            argv[argc++] = ir_file; // Use original IR file, not optimized BC
            argv[argc++] = "-o";
            argv[argc++] = output_file;

            // Add coverage flags
            argv[argc++] = "-fprofile-instr-generate";
            argv[argc++] = "-fcoverage-mapping";

            // Add optimization level
            argv[argc++] = asthra_llvm_opt_level_flag(options->opt_level);

            // Add target if specified
            if (options->target_arch != ASTHRA_TARGET_NATIVE) {
                argv[argc++] = "-target";
                argv[argc++] = asthra_llvm_target_triple(options->target_arch);
            }

            // Add debug info if requested
            if (options->debug_info) {
                argv[argc++] = "-g";
            }

            // Add PIE flags based on the mode
            if (options->pie_mode == ASTHRA_PIE_FORCE_ENABLED) {
                argv[argc++] = "-pie";
                argv[argc++] = "-fPIE";
            } else if (options->pie_mode == ASTHRA_PIE_FORCE_DISABLED) {
                argv[argc++] = "-no-pie";
                argv[argc++] = "-fno-PIE";
            }

            argv[argc] = NULL;

            // Debug: print the command being executed
            if (options->verbose) {
                fprintf(stderr, "Coverage compilation command: ");
                for (int i = 0; argv[i]; i++) {
                    fprintf(stderr, "%s ", argv[i]);
                }
                fprintf(stderr, "\n");
            }

            result = execute_command(argv, options->verbose);
        } else {
            // Normal path: compile to object then link
            char *object_file = asthra_llvm_get_output_filename(ir_file, ASTHRA_FORMAT_OBJECT);
            AsthraLLVMToolOptions compile_options = {
                .output_format = ASTHRA_FORMAT_OBJECT,
                .output_file = object_file,
                .opt_level = ASTHRA_OPT_NONE, // Already optimized
                .target_arch = options->target_arch,
                .target_triple = asthra_llvm_target_triple(options->target_arch),
                .debug_info = options->debug_info,
                .verbose = options->verbose,
                .coverage = options->coverage,
                .pie_mode = options->pie_mode};

            result = asthra_llvm_compile(compile_input, &compile_options);
            if (result.success) {
                // Link to executable
                const char *objects[] = {object_file};
                AsthraLLVMToolOptions link_options = {.output_file = output_file,
                                                      .opt_level = ASTHRA_OPT_NONE,
                                                      .target_triple =
                                                          compile_options.target_triple,
                                                      .verbose = options->verbose,
                                                      .coverage = options->coverage,
                                                      .pie_mode = options->pie_mode};

                asthra_llvm_tool_result_free(&result);
                result = asthra_llvm_link(objects, 1, &link_options);
            }

            // Clean up temporary object file
            unlink(object_file);
            free(object_file);
        }
    }

    // Clean up temporary optimized file
    if (optimized_file) {
        unlink(optimized_file);
        free(optimized_file);
    }

    return result;
}