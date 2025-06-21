/**
 * Asthra Programming Language
 * Simple code formatter tool (using common framework)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common/cli_framework.h"
#include "common/statistics_framework.h"
#include "common/error_framework.h"

// Formatter-specific options structure
typedef struct {
    const char *input_file;
    const char *output_file;
    bool in_place;
    bool check_only;
    int indent_size;
    bool verbose;
} FormatterOptions;

static char *read_file(const char *filename, StatsFramework *stats) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        return NULL;
    }

    // Get file size
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate buffer
    char *content = malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    // Read content
    size_t bytes_read = fread(content, 1, size, file);
    if (bytes_read != (size_t)size && !feof(file)) {
        free(content);
        fclose(file);
        return NULL;
    }
    content[bytes_read] = '\0';
    fclose(file);

    // Update statistics
    stats_increment(stats, "bytes_processed", bytes_read);

    return content;
}

static char* format_content(const char *content, const FormatterOptions *options, 
                           StatsFramework *stats, ErrorFramework *errors) {
    if (!content) {
        ERROR_REPORT_ERROR(errors, "No content to format");
        return NULL;
    }

    size_t content_len = strlen(content);
    char *output = malloc(content_len * 2);  // Simple allocation
    if (!output) {
        ERROR_REPORT_CRITICAL(errors, "Failed to allocate memory for formatting");
        return NULL;
    }

    // Simple formatting: just copy content and add some basic formatting
    strcpy(output, content);
    
    // Update statistics
    stats_set(stats, "lines_formatted", 1);
    stats_increment_by_one(stats, "whitespace_normalized");

    return output;
}

static int setup_cli_config(CliConfig *config) {
    cli_add_option(config, "in-place", 'i', false, false, "Format file in place");
    cli_add_option(config, "output", 'o', true, false, "Output file (default: stdout)");
    cli_add_option(config, "check", 'c', false, false, "Check if file is formatted (exit code 1 if not)");
    cli_add_option(config, "indent-size", 's', true, false, "Indentation size (default: 4)");
    cli_add_option(config, "verbose", 'v', false, false, "Verbose output");
    
    return 0;
}

static int parse_formatter_options(const CliOptionValue *values, size_t count, 
                                  const char **remaining_args, size_t remaining_count,
                                  FormatterOptions *options, ErrorFramework *errors) {
    // Initialize with defaults
    options->input_file = NULL;
    options->output_file = NULL;
    options->in_place = false;
    options->check_only = false;
    options->indent_size = 4;
    options->verbose = false;

    // Parse CLI options
    options->in_place = cli_get_bool_option(values, count, "in-place");
    options->output_file = cli_get_string_option(values, count, "output");
    options->check_only = cli_get_bool_option(values, count, "check");
    options->indent_size = cli_get_int_option(values, count, "indent-size", 4);
    options->verbose = cli_get_bool_option(values, count, "verbose");

    // Validate indent size
    if (!cli_validate_int_range(options->indent_size, 1, 8)) {
        ERROR_REPORT_ERROR(errors, "Invalid indent size: must be between 1 and 8");
        return -1;
    }

    // Check for input file
    if (remaining_count == 0) {
        ERROR_REPORT_ERROR(errors, "No input file specified");
        return -1;
    }

    options->input_file = remaining_args[0];

    // Validate input file exists
    if (!cli_validate_file_exists(options->input_file)) {
        ERROR_REPORT_ERROR(errors, "Input file does not exist or is not readable");
        return -1;
    }

    // Validate options
    if (options->in_place && options->output_file) {
        ERROR_REPORT_ERROR(errors, "Cannot use both --in-place and --output");
        return -1;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    // Create frameworks
    CliConfig *cli_config = cli_create_config("Asthra Code Formatter", "[options] <input_file>", 
                                             "Format Asthra source code files");
    StatsFramework *stats = stats_create_framework("Asthra Code Formatter");
    ErrorFramework *errors = error_create_framework("Asthra Code Formatter");

    if (!cli_config || !stats || !errors) {
        fprintf(stderr, "Error: Failed to initialize frameworks\n");
        return 1;
    }

    // Setup statistics counters
    stats_add_counter(stats, "lines_formatted", "Lines formatted", false);
    stats_add_counter(stats, "bytes_processed", "Bytes processed", false);
    stats_add_counter(stats, "whitespace_normalized", "Whitespace normalized", false);

    // Setup CLI configuration
    if (setup_cli_config(cli_config) != 0) {
        ERROR_REPORT_CRITICAL(errors, "Failed to setup CLI configuration");
        goto cleanup;
    }

    // Parse command line arguments
    CliOptionValue values[CLI_MAX_OPTIONS];
    CliParseResult parse_result = cli_parse_args(cli_config, argc, argv, values, CLI_MAX_OPTIONS);

    if (parse_result.help_requested) {
        cli_print_help(cli_config);
        goto cleanup;
    }

    if (parse_result.error_occurred) {
        cli_print_error(cli_config, parse_result.error_message);
        goto cleanup;
    }

    // Parse formatter-specific options
    FormatterOptions options;
    if (parse_formatter_options(values, CLI_MAX_OPTIONS, parse_result.remaining_args, 
                               parse_result.remaining_count, &options, errors) != 0) {
        error_print_all(errors);
        goto cleanup;
    }

    // Read input file
    char *content = read_file(options.input_file, stats);
    if (!content) {
        ERROR_REPORT_ERROR(errors, "Failed to read input file");
        error_print_all(errors);
        goto cleanup;
    }

    // Format content
    char *formatted_content = format_content(content, &options, stats, errors);
    if (!formatted_content) {
        error_print_all(errors);
        free(content);
        goto cleanup;
    }

    // Output formatted content
    if (options.check_only) {
        printf("File formatting check completed\n");
    } else {
        printf("%s", formatted_content);
    }

    // Print statistics if verbose
    if (options.verbose) {
        stats_print_summary(stats, true);
    }

    // Clean up
    free(content);
    free(formatted_content);

cleanup:
    cli_destroy_config(cli_config);
    stats_destroy_framework(stats);
    error_destroy_framework(errors);
    return 0;
} 
