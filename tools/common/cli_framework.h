/**
 * Asthra Programming Language
 * Common CLI Framework for Tools
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CLI_FRAMEWORK_H
#define ASTHRA_CLI_FRAMEWORK_H

#include <stdbool.h>
#include <stddef.h>
#include <getopt.h>

// Maximum number of CLI options per tool
#define CLI_MAX_OPTIONS 16

// CLI option definition structure
typedef struct {
    const char *name;           // Long option name (e.g., "verbose")
    const char *description;    // Help text description
    char short_opt;            // Short option character (e.g., 'v')
    bool has_arg;              // Whether option takes an argument
    bool required;             // Whether option is required
} CliOptionDef;

// CLI configuration structure
typedef struct {
    const char *program_name;   // Tool name (e.g., "Asthra Code Formatter")
    const char *usage_format;   // Usage format string (e.g., "[options] <input_file>")
    const char *description;    // Tool description
    CliOptionDef options[CLI_MAX_OPTIONS];  // Option definitions
    size_t option_count;       // Number of options
} CliConfig;

// CLI parsing result structure
typedef struct {
    bool help_requested;       // Whether help was requested
    bool error_occurred;       // Whether parsing error occurred
    const char *error_message; // Error message if any
    const char **remaining_args; // Non-option arguments
    size_t remaining_count;    // Number of remaining arguments
} CliParseResult;

// CLI option value structure for storing parsed values
typedef struct {
    const char *name;          // Option name
    bool is_set;              // Whether option was provided
    const char *value;        // String value (if has_arg)
    bool bool_value;          // Boolean value (if no_arg)
} CliOptionValue;

// CLI framework functions
CliConfig* cli_create_config(const char *program_name, const char *usage_format, const char *description);
void cli_destroy_config(CliConfig *config);

int cli_add_option(CliConfig *config, const char *name, char short_opt, 
                   bool has_arg, bool required, const char *description);

CliParseResult cli_parse_args(CliConfig *config, int argc, char **argv, 
                             CliOptionValue *values, size_t max_values);

void cli_print_help(const CliConfig *config);
void cli_print_error(const CliConfig *config, const char *error_message);

// Utility functions for common patterns
bool cli_get_bool_option(const CliOptionValue *values, size_t count, const char *name);
const char* cli_get_string_option(const CliOptionValue *values, size_t count, const char *name);
int cli_get_int_option(const CliOptionValue *values, size_t count, const char *name, int default_value);

// Validation functions
bool cli_validate_file_exists(const char *filename);
bool cli_validate_directory_exists(const char *dirname);
bool cli_validate_int_range(int value, int min, int max);

#endif // ASTHRA_CLI_FRAMEWORK_H
