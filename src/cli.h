/**
 * Asthra Programming Language Compiler
 * Command Line Interface - Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CLI_H
#define ASTHRA_CLI_H

#include "compiler.h"
#include "platform.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CLI OPTION STRUCTURE
// =============================================================================

typedef struct {
    AsthraCompilerOptions compiler_options;
    bool test_mode;
    bool show_version;
    bool show_help;
    int exit_code;
} CliOptions;

// =============================================================================
// CLI FUNCTIONS
// =============================================================================

/**
 * Print program usage information
 */
void cli_print_usage(const char *program_name);

/**
 * Print version information
 */
void cli_print_version(void);

/**
 * Parse target architecture string
 */
AsthraTargetArch cli_parse_target_arch(const char *arch_str);

/**
 * Parse optimization level string
 */
AsthraOptimizationLevel cli_parse_optimization_level(const char *opt_str);

/**
 * Parse backend type string
 */

/**
 * Parse command line arguments
 * Returns 0 on success, non-zero on error or early exit (help/version)
 */
int cli_parse_arguments(int argc, char *argv[], CliOptions *options);

/**
 * Initialize CLI options with defaults
 */
CliOptions cli_options_init(void);

/**
 * Cleanup CLI options (free allocated memory)
 */
void cli_options_cleanup(CliOptions *options);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CLI_H
