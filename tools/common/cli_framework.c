/**
 * Asthra Programming Language
 * Common CLI Framework for Tools - Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "cli_framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

CliConfig *cli_create_config(const char *program_name, const char *usage_format,
                             const char *description) {
    CliConfig *config = malloc(sizeof(CliConfig));
    if (!config) {
        return NULL;
    }

    config->program_name = program_name;
    config->usage_format = usage_format;
    config->description = description;
    config->option_count = 0;

    // Always add help option
    cli_add_option(config, "help", 'h', false, false, "Show this help message");

    return config;
}

void cli_destroy_config(CliConfig *config) {
    if (config) {
        free(config);
    }
}

int cli_add_option(CliConfig *config, const char *name, char short_opt, bool has_arg, bool required,
                   const char *description) {
    if (!config || config->option_count >= CLI_MAX_OPTIONS) {
        return -1;
    }

    CliOptionDef *opt = &config->options[config->option_count];
    opt->name = name;
    opt->short_opt = short_opt;
    opt->has_arg = has_arg;
    opt->required = required;
    opt->description = description;

    config->option_count++;
    return 0;
}

CliParseResult cli_parse_args(CliConfig *config, int argc, char **argv, CliOptionValue *values,
                              size_t max_values) {
    CliParseResult result = {0};

    // Build getopt_long structures
    struct option long_options[CLI_MAX_OPTIONS + 1];
    char short_options[CLI_MAX_OPTIONS * 2 + 1];
    size_t short_idx = 0;

    // Initialize values array
    for (size_t i = 0; i < max_values && i < config->option_count; i++) {
        values[i].name = config->options[i].name;
        values[i].is_set = false;
        values[i].value = NULL;
        values[i].bool_value = false;
    }

    // Build option structures
    for (size_t i = 0; i < config->option_count; i++) {
        const CliOptionDef *opt = &config->options[i];

        // Long option
        long_options[i].name = opt->name;
        long_options[i].has_arg = opt->has_arg ? required_argument : no_argument;
        long_options[i].flag = 0;
        long_options[i].val = opt->short_opt;

        // Short option
        short_options[short_idx++] = opt->short_opt;
        if (opt->has_arg) {
            short_options[short_idx++] = ':';
        }
    }

    // Terminate arrays
    memset(&long_options[config->option_count], 0, sizeof(struct option));
    short_options[short_idx] = '\0';

    // Parse options
    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1) {
        if (c == 'h') {
            result.help_requested = true;
            return result;
        }

        if (c == '?') {
            result.error_occurred = true;
            result.error_message = "Invalid option";
            return result;
        }

        // Find matching option
        bool found = false;
        for (size_t i = 0; i < config->option_count && i < max_values; i++) {
            if (config->options[i].short_opt == c) {
                values[i].is_set = true;
                if (config->options[i].has_arg) {
                    values[i].value = optarg;
                } else {
                    values[i].bool_value = true;
                }
                found = true;
                break;
            }
        }

        if (!found) {
            result.error_occurred = true;
            result.error_message = "Unknown option";
            return result;
        }
    }

    // Check required options
    for (size_t i = 0; i < config->option_count && i < max_values; i++) {
        if (config->options[i].required && !values[i].is_set) {
            result.error_occurred = true;
            result.error_message = "Missing required option";
            return result;
        }
    }

    // Store remaining arguments
    if (optind < argc) {
        result.remaining_args = (const char **)&argv[optind];
        result.remaining_count = argc - optind;
    }

    return result;
}

void cli_print_help(const CliConfig *config) {
    printf("%s\n", config->program_name);
    printf("Usage: %s %s\n\n", config->program_name, config->usage_format);

    if (config->description) {
        printf("%s\n\n", config->description);
    }

    printf("Options:\n");
    for (size_t i = 0; i < config->option_count; i++) {
        const CliOptionDef *opt = &config->options[i];
        printf("  -%c, --%-15s %s%s\n", opt->short_opt, opt->name, opt->description,
               opt->required ? " (required)" : "");
    }
}

void cli_print_error(const CliConfig *config, const char *error_message) {
    fprintf(stderr, "Error: %s\n", error_message);
    fprintf(stderr, "Use '%s --help' for usage information.\n", config->program_name);
}

bool cli_get_bool_option(const CliOptionValue *values, size_t count, const char *name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(values[i].name, name) == 0) {
            return values[i].is_set && values[i].bool_value;
        }
    }
    return false;
}

const char *cli_get_string_option(const CliOptionValue *values, size_t count, const char *name) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(values[i].name, name) == 0 && values[i].is_set) {
            return values[i].value;
        }
    }
    return NULL;
}

int cli_get_int_option(const CliOptionValue *values, size_t count, const char *name,
                       int default_value) {
    const char *str_value = cli_get_string_option(values, count, name);
    if (str_value) {
        return atoi(str_value);
    }
    return default_value;
}

bool cli_validate_file_exists(const char *filename) {
    return filename && access(filename, R_OK) == 0;
}

bool cli_validate_directory_exists(const char *dirname) {
    return dirname && access(dirname, R_OK) == 0;
}

bool cli_validate_int_range(int value, int min, int max) {
    return value >= min && value <= max;
}
