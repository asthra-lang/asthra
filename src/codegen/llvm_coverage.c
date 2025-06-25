/**
 * Asthra Programming Language LLVM Coverage
 * Implementation of code coverage support using LLVM tools
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_coverage.h"
#include "llvm_process_execution.h"
#include "llvm_tool_detection.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// COVERAGE SUPPORT IMPLEMENTATION
// =============================================================================

bool asthra_llvm_coverage_available(void) {
    static bool checked = false;
    static bool available = false;

    if (!checked) {
        const char *llvm_cov = asthra_llvm_tool_path("llvm-cov");
        const char *llvm_profdata = asthra_llvm_tool_path("llvm-profdata");

        available = (llvm_cov != NULL) && (llvm_profdata != NULL);
        checked = true;

        if (available) {
            fprintf(stderr, "LLVM coverage tools available:\n");
            fprintf(stderr, "  llvm-cov: %s\n", llvm_cov);
            fprintf(stderr, "  llvm-profdata: %s\n", llvm_profdata);
        }
    }

    return available;
}

AsthraLLVMToolResult asthra_llvm_merge_profile_data(const char **profraw_files, size_t num_files,
                                                    const char *output_profdata) {
    AsthraLLVMToolResult result = {0};

    if (!profraw_files || num_files == 0 || !output_profdata) {
        result.success = false;
        result.stderr_output = strdup("Invalid parameters for profile merge");
        return result;
    }

    const char *llvm_profdata = asthra_llvm_tool_path("llvm-profdata");
    if (!llvm_profdata) {
        result.success = false;
        result.stderr_output = strdup("llvm-profdata not found");
        return result;
    }

    // Build command: llvm-profdata merge -sparse file1.profraw file2.profraw -o output.profdata
    char **args = malloc(sizeof(char *) * (num_files + 6));
    if (!args) {
        result.success = false;
        result.stderr_output = strdup("Memory allocation failed");
        return result;
    }

    size_t arg_idx = 0;
    args[arg_idx++] = (char *)llvm_profdata;
    args[arg_idx++] = "merge";
    args[arg_idx++] = "-sparse";

    // Add all input files
    for (size_t i = 0; i < num_files; i++) {
        args[arg_idx++] = (char *)profraw_files[i];
    }

    args[arg_idx++] = "-o";
    args[arg_idx++] = (char *)output_profdata;
    args[arg_idx] = NULL;

    result = execute_command(args, false);
    free(args);

    return result;
}

AsthraLLVMToolResult asthra_llvm_coverage_report(const char *executable, const char *profdata,
                                                 const char *format, const char *output_path,
                                                 const char **source_filters, size_t num_filters) {
    AsthraLLVMToolResult result = {0};

    if (!executable || !profdata || !format || !output_path) {
        result.success = false;
        result.stderr_output = strdup("Invalid parameters for coverage report");
        return result;
    }

    const char *llvm_cov = asthra_llvm_tool_path("llvm-cov");
    if (!llvm_cov) {
        result.success = false;
        result.stderr_output = strdup("llvm-cov not found");
        return result;
    }

    // Determine subcommand based on format
    const char *subcommand = NULL;
    if (strcmp(format, "html") == 0 || strcmp(format, "text") == 0) {
        subcommand = "show";
    } else if (strcmp(format, "lcov") == 0 || strcmp(format, "json") == 0) {
        subcommand = "export";
    } else {
        result.success = false;
        result.stderr_output = strdup("Unsupported format. Use: html, text, lcov, or json");
        return result;
    }

    // Build command
    size_t max_args = 20 + (num_filters * 2);
    char **args = calloc(max_args, sizeof(char *));
    if (!args) {
        result.success = false;
        result.stderr_output = strdup("Memory allocation failed");
        return result;
    }

    size_t arg_idx = 0;
    args[arg_idx++] = (char *)llvm_cov;
    args[arg_idx++] = (char *)subcommand;
    args[arg_idx++] = (char *)executable;
    args[arg_idx++] = "-instr-profile";
    args[arg_idx++] = (char *)profdata;

    if (strcmp(subcommand, "show") == 0) {
        // For show command
        if (strcmp(format, "html") == 0) {
            args[arg_idx++] = "-format=html";
            args[arg_idx++] = "-output-dir";
            args[arg_idx++] = (char *)output_path;
            args[arg_idx++] = "-show-line-counts-or-regions";
            args[arg_idx++] = "-show-expansions";
            args[arg_idx++] = "-show-instantiations";
        } else {
            args[arg_idx++] = "-format=text";
        }
    } else {
        // For export command
        if (strcmp(format, "lcov") == 0) {
            args[arg_idx++] = "-format=lcov";
        } else {
            args[arg_idx++] = "-format=text"; // JSON
        }
    }

    // Add source filters
    if (source_filters && num_filters > 0) {
        for (size_t i = 0; i < num_filters; i++) {
            args[arg_idx++] = (char *)source_filters[i];
        }
    }

    // Add ignore patterns
    args[arg_idx++] = "-ignore-filename-regex=(tests/|third-party/|build/)";
    args[arg_idx] = NULL;

    // Execute and handle output redirection for text formats
    if (strcmp(subcommand, "show") == 0 && strcmp(format, "text") == 0) {
        // Redirect stdout to file
        result = execute_command(args, false);
        if (result.success && result.stdout_output) {
            FILE *out = fopen(output_path, "w");
            if (out) {
                fputs(result.stdout_output, out);
                fclose(out);
            } else {
                result.success = false;
                result.stderr_output = strdup("Failed to write output file");
            }
        }
    } else if (strcmp(subcommand, "export") == 0) {
        // Export commands output to stdout, redirect to file
        result = execute_command(args, false);
        if (result.success && result.stdout_output) {
            FILE *out = fopen(output_path, "w");
            if (out) {
                fputs(result.stdout_output, out);
                fclose(out);
            } else {
                result.success = false;
                result.stderr_output = strdup("Failed to write output file");
            }
        }
    } else {
        // HTML output is written directly by llvm-cov
        result = execute_command(args, false);
    }

    free(args);
    return result;
}

bool asthra_llvm_coverage_summary(const char *executable, const char *profdata,
                                  double *line_coverage, double *function_coverage,
                                  double *region_coverage) {
    if (!executable || !profdata) {
        return false;
    }

    const char *llvm_cov = asthra_llvm_tool_path("llvm-cov");
    if (!llvm_cov) {
        return false;
    }

    // Build command: llvm-cov report <executable> -instr-profile=<profdata>
    char *args[] = {(char *)llvm_cov,
                    "report",
                    (char *)executable,
                    "-instr-profile",
                    (char *)profdata,
                    "-ignore-filename-regex=(tests/|third-party/|build/)",
                    NULL};

    AsthraLLVMToolResult result = execute_command(args, false);
    if (!result.success || !result.stdout_output) {
        asthra_llvm_tool_result_free(&result);
        return false;
    }

    // Parse the output to extract coverage percentages
    // Look for the TOTAL line which has format:
    // TOTAL      1234      567    89.12%     234     56    76.34%    345    123    64.35%
    //            Lines             Line%      Funcs         Func%     Regions       Region%

    char *total_line = strstr(result.stdout_output, "TOTAL");
    if (!total_line) {
        asthra_llvm_tool_result_free(&result);
        return false;
    }

    // Skip "TOTAL" and parse the numbers
    char *ptr = total_line + 5;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Skip lines executed
    while (*ptr && !isspace(*ptr))
        ptr++;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Skip lines total
    while (*ptr && !isspace(*ptr))
        ptr++;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Parse line coverage percentage
    if (line_coverage) {
        *line_coverage = strtod(ptr, &ptr);
        if (*ptr == '%')
            ptr++;
    }
    while (*ptr && isspace(*ptr))
        ptr++;

    // Skip functions executed
    while (*ptr && !isspace(*ptr))
        ptr++;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Skip functions total
    while (*ptr && !isspace(*ptr))
        ptr++;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Parse function coverage percentage
    if (function_coverage) {
        *function_coverage = strtod(ptr, &ptr);
        if (*ptr == '%')
            ptr++;
    }
    while (*ptr && isspace(*ptr))
        ptr++;

    // Skip regions executed
    while (*ptr && !isspace(*ptr))
        ptr++;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Skip regions total
    while (*ptr && !isspace(*ptr))
        ptr++;
    while (*ptr && isspace(*ptr))
        ptr++;

    // Parse region coverage percentage
    if (region_coverage) {
        *region_coverage = strtod(ptr, &ptr);
    }

    asthra_llvm_tool_result_free(&result);
    return true;
}