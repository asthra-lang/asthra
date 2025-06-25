/**
 * Asthra Programming Language
 * Code formatter tool
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <getopt.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../runtime/asthra_runtime.h"
#include "../src/compiler.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for formatter");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void *),
                     "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe statistics
typedef struct {
    _Atomic(uint64_t) lines_formatted;
    _Atomic(uint64_t) bytes_processed;
    _Atomic(uint64_t) indentation_changes;
    _Atomic(uint64_t) whitespace_normalized;
} FormatterStatistics;

// C17 designated initializer for formatter options
typedef struct {
    const char *input_file;
    const char *output_file;
    bool in_place;
    bool verbose;
    bool check_only;
    int indent_size;
    FormatterStatistics *stats;
} FormatterOptions;

// C17 _Generic for polymorphic formatting operations
#define format_token(token)                                                                        \
    _Generic((token),                                                                              \
        char *: format_string_token,                                                               \
        const char *: format_string_token,                                                         \
        int: format_integer_token,                                                                 \
        default: format_generic_token)(token)

// Forward declarations for _Generic dispatch
static char *format_string_token(const char *token);
static char *format_integer_token(int token);
static char *format_generic_token(void *token);

// C17 atomic operations for thread-safe statistics
static inline void increment_stat(_Atomic(uint64_t) *counter, uint64_t value) {
    atomic_fetch_add_explicit(counter, value, memory_order_relaxed);
}

static inline uint64_t get_stat(_Atomic(uint64_t) *counter) {
    return atomic_load_explicit(counter, memory_order_relaxed);
}

static void print_usage(const char *program_name) {
    printf("Asthra Code Formatter\n");
    printf("Usage: %s [options] <input_file>\n\n", program_name);
    printf("Options:\n");
    printf("  -i, --in-place          Format file in place\n");
    printf("  -o, --output <file>     Output file (default: stdout)\n");
    printf("  -c, --check             Check if file is formatted (exit code 1 if not)\n");
    printf("  -s, --indent-size <n>   Indentation size (default: 4)\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -h, --help              Show this help message\n");
}

static FormatterStatistics *create_formatter_statistics(void) {
    FormatterStatistics *stats = malloc(sizeof(FormatterStatistics));
    if (!stats) {
        return NULL;
    }

    // C17 designated initializer with atomic initialization
    *stats = (FormatterStatistics){.lines_formatted = 0,
                                   .bytes_processed = 0,
                                   .indentation_changes = 0,
                                   .whitespace_normalized = 0};

    return stats;
}

static void destroy_formatter_statistics(FormatterStatistics *stats) {
    free(stats);
}

static int parse_arguments(int argc, char *argv[], FormatterOptions *options) {
    // C17 designated initializer for long options
    static struct option long_options[] = {
        {.name = "in-place", .has_arg = no_argument, .flag = 0, .val = 'i'},
        {.name = "output", .has_arg = required_argument, .flag = 0, .val = 'o'},
        {.name = "check", .has_arg = no_argument, .flag = 0, .val = 'c'},
        {.name = "indent-size", .has_arg = required_argument, .flag = 0, .val = 's'},
        {.name = "verbose", .has_arg = no_argument, .flag = 0, .val = 'v'},
        {.name = "help", .has_arg = no_argument, .flag = 0, .val = 'h'},
        {0, 0, 0, 0}};

    // C17 designated initializer for default options
    *options = (FormatterOptions){.input_file = NULL,
                                  .output_file = NULL,
                                  .in_place = false,
                                  .verbose = false,
                                  .check_only = false,
                                  .indent_size = 4,
                                  .stats = create_formatter_statistics()};

    if (!options->stats) {
        fprintf(stderr, "Error: Failed to create formatter statistics\n");
        return -1;
    }

    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "io:cs:vh", long_options, &option_index)) != -1) {
        switch (c) {
        case 'i':
            options->in_place = true;
            break;
        case 'o':
            options->output_file = optarg;
            break;
        case 'c':
            options->check_only = true;
            break;
        case 's':
            options->indent_size = atoi(optarg);
            if (options->indent_size < 1 || options->indent_size > 8) {
                fprintf(stderr, "Error: Invalid indent size: %s\n", optarg);
                return -1;
            }
            break;
        case 'v':
            options->verbose = true;
            break;
        case 'h':
            print_usage(argv[0]);
            return 1;
        case '?':
            return -1;
        default:
            abort();
        }
    }

    // Check for input file
    if (optind >= argc) {
        fprintf(stderr, "Error: No input file specified\n");
        print_usage(argv[0]);
        return -1;
    }

    options->input_file = argv[optind];

    // Validate input file exists
    if (access(options->input_file, R_OK) != 0) {
        perror(options->input_file);
        return -1;
    }

    // Validate options
    if (options->in_place && options->output_file) {
        fprintf(stderr, "Error: Cannot use both --in-place and --output\n");
        return -1;
    }

    return 0;
}

static char *read_file(const char *filename, FormatterStatistics *stats) {
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

    // Read file
    size_t read_size = fread(content, 1, size, file);
    if (read_size != (size_t)size && !feof(file)) {
        free(content);
        fclose(file);
        return NULL;
    }
    content[read_size] = '\0';

    // Update statistics atomically
    increment_stat(&stats->bytes_processed, read_size);

    fclose(file);
    return content;
}

static int write_file(const char *filename, const char *content, FormatterStatistics *stats) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        return -1;
    }

    size_t len = strlen(content);
    size_t written = fwrite(content, 1, len, file);
    fclose(file);

    // Update statistics atomically
    if (written == len) {
        increment_stat(&stats->bytes_processed, written);
    }

    return (written == len) ? 0 : -1;
}

// C17 _Generic dispatch implementations
static char *format_string_token(const char *token) {
    if (!token)
        return NULL;

    size_t len = strlen(token);
    char *formatted = malloc(len + 1);
    if (formatted) {
        strcpy(formatted, token);
    }
    return formatted;
}

static char *format_integer_token(int token) {
    char *formatted = malloc(32); // Sufficient for int
    if (formatted) {
        snprintf(formatted, 32, "%d", token);
    }
    return formatted;
}

static char *format_generic_token(void *token) {
    (void)token; // Suppress unused parameter warning
    return NULL; // Generic fallback
}

// C17 compound literal for formatting rules
typedef struct {
    char trigger;
    const char *replacement;
    bool increase_indent;
    bool decrease_indent;
} FormatRule;

static const FormatRule *get_format_rules(void) {
    // C17 compound literal array
    static const FormatRule rules[] = {
        {'{', " {\n", true, false},
        {'}', "}\n", false, true},
        {';', ";\n", false, false},
        {',', ", ", false, false},
        {0, NULL, false, false} // Sentinel
    };
    return rules;
}

static char *format_asthra_code(const char *input, FormatterOptions *options) {
    // This is a simplified formatter implementation
    // A full implementation would parse the AST and reformat based on syntax

    size_t input_len = strlen(input);
    size_t output_capacity = input_len * 2; // Generous buffer
    char *output = malloc(output_capacity);
    if (!output) {
        return NULL;
    }

    size_t output_pos = 0;
    int indent_level = 0;
    bool at_line_start = true;
    bool in_string = false;
    bool in_comment = false;
    uint64_t lines_count = 0;
    uint64_t indent_changes = 0;
    uint64_t whitespace_changes = 0;

    const FormatRule *rules = get_format_rules();

    for (size_t i = 0; i < input_len; i++) {
        char c = input[i];
        char next_c = (i + 1 < input_len) ? input[i + 1] : '\0';

        // Handle string literals
        if (c == '"' && !in_comment) {
            in_string = !in_string;
        }

        // Handle comments
        if (!in_string && c == '/' && next_c == '/') {
            in_comment = true;
        }
        if (in_comment && c == '\n') {
            in_comment = false;
        }

        // Skip processing inside strings and comments
        if (in_string || in_comment) {
            output[output_pos++] = c;
            if (c == '\n') {
                at_line_start = true;
                lines_count++;
            } else {
                at_line_start = false;
            }
            continue;
        }

        // Handle indentation
        if (at_line_start && c != '\n' && c != ' ' && c != '\t') {
            // Add proper indentation
            for (int j = 0; j < indent_level * options->indent_size; j++) {
                output[output_pos++] = ' ';
            }
            at_line_start = false;
            indent_changes++;
        }

        // Apply formatting rules using C17 compound literal lookup
        bool rule_applied = false;
        for (const FormatRule *rule = rules; rule->trigger != 0; rule++) {
            if (c == rule->trigger) {
                // Apply rule
                if (rule->decrease_indent && indent_level > 0) {
                    indent_level--;
                    indent_changes++;
                }

                if (rule->replacement) {
                    size_t repl_len = strlen(rule->replacement);
                    memcpy(output + output_pos, rule->replacement, repl_len);
                    output_pos += repl_len;
                    whitespace_changes++;
                } else {
                    output[output_pos++] = c;
                }

                if (rule->increase_indent) {
                    indent_level++;
                    indent_changes++;
                }

                if (strchr(rule->replacement ? rule->replacement : "", '\n')) {
                    at_line_start = true;
                    lines_count++;
                }

                rule_applied = true;
                break;
            }
        }

        if (!rule_applied) {
            // Handle whitespace normalization
            if (c == ' ' || c == '\t') {
                // Skip multiple whitespace
                if (output_pos > 0 && output[output_pos - 1] != ' ') {
                    output[output_pos++] = ' ';
                    whitespace_changes++;
                }
            } else {
                output[output_pos++] = c;
                if (c == '\n') {
                    at_line_start = true;
                    lines_count++;
                }
            }
        }

        // Ensure buffer capacity
        if (output_pos >= output_capacity - 100) {
            output_capacity *= 2;
            char *new_output = realloc(output, output_capacity);
            if (!new_output) {
                free(output);
                return NULL;
            }
            output = new_output;
        }
    }

    output[output_pos] = '\0';

    // Update statistics atomically
    increment_stat(&options->stats->lines_formatted, lines_count);
    increment_stat(&options->stats->indentation_changes, indent_changes);
    increment_stat(&options->stats->whitespace_normalized, whitespace_changes);

    return output;
}

static void print_statistics(const FormatterStatistics *stats, bool verbose) {
    if (!verbose)
        return;

    printf("\nFormatter Statistics:\n");
    printf("  Lines formatted: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->lines_formatted));
    printf("  Bytes processed: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->bytes_processed));
    printf("  Indentation changes: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->indentation_changes));
    printf("  Whitespace normalized: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->whitespace_normalized));
}

int main(int argc, char *argv[]) {
    FormatterOptions options;

    // Parse command line arguments
    int parse_result = parse_arguments(argc, argv, &options);
    if (parse_result != 0) {
        if (options.stats) {
            destroy_formatter_statistics(options.stats);
        }
        return (parse_result > 0) ? 0 : 1; // 1 for help, -1 for error
    }

    if (options.verbose) {
        printf("Formatting %s...\n", options.input_file);
    }

    // Read input file
    char *input_content = read_file(options.input_file, options.stats);
    if (!input_content) {
        fprintf(stderr, "Error: Failed to read input file: %s\n", options.input_file);
        destroy_formatter_statistics(options.stats);
        return 1;
    }

    // Format the code
    char *formatted_content = format_asthra_code(input_content, &options);
    if (!formatted_content) {
        fprintf(stderr, "Error: Failed to format code\n");
        free(input_content);
        destroy_formatter_statistics(options.stats);
        return 1;
    }

    // Check mode: compare original and formatted
    if (options.check_only) {
        int is_formatted = (strcmp(input_content, formatted_content) == 0);
        if (options.verbose) {
            printf("File %s %s formatted\n", options.input_file,
                   is_formatted ? "is already" : "needs to be");
        }

        print_statistics(options.stats, options.verbose);

        free(input_content);
        free(formatted_content);
        destroy_formatter_statistics(options.stats);
        return is_formatted ? 0 : 1;
    }

    // Write output
    int write_result = 0;
    if (options.in_place) {
        write_result = write_file(options.input_file, formatted_content, options.stats);
        if (write_result == 0 && options.verbose) {
            printf("Formatted %s in place\n", options.input_file);
        }
    } else if (options.output_file) {
        write_result = write_file(options.output_file, formatted_content, options.stats);
        if (write_result == 0 && options.verbose) {
            printf("Formatted code written to %s\n", options.output_file);
        }
    } else {
        // Output to stdout
        printf("%s", formatted_content);
    }

    if (write_result != 0) {
        fprintf(stderr, "Error: Failed to write output\n");
    }

    print_statistics(options.stats, options.verbose);

    // Cleanup
    free(input_content);
    free(formatted_content);
    destroy_formatter_statistics(options.stats);

    return write_result;
}
