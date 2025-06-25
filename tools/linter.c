/**
 * Asthra Programming Language
 * Code linter tool
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <getopt.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../runtime/asthra_runtime.h"
#include "../src/compiler.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for linter");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void *),
                     "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe linting statistics
typedef struct {
    _Atomic(uint64_t) files_analyzed;
    _Atomic(uint64_t) lines_analyzed;
    _Atomic(uint64_t) warnings_found;
    _Atomic(uint64_t) errors_found;
    _Atomic(uint64_t) suggestions_made;
} LinterStatistics;

// Lint severity levels
typedef enum { LINT_INFO, LINT_WARNING, LINT_ERROR, LINT_CRITICAL } LintSeverity;

// Lint issue structure
typedef struct {
    LintSeverity severity;
    size_t line;
    size_t column;
    const char *message;
    const char *rule_name;
} LintIssue;

// C17 designated initializer for linter options
typedef struct {
    const char *input_file;
    bool verbose;
    bool warnings_as_errors;
    bool show_suggestions;
    bool json_output;
    LintSeverity min_severity;
    LinterStatistics *stats;
} LinterOptions;

// C17 _Generic for polymorphic linting operations
#define lint_check(node)                                                                           \
    _Generic((node),                                                                               \
        char *: lint_check_string,                                                                 \
        const char *: lint_check_string,                                                           \
        int: lint_check_integer,                                                                   \
        default: lint_check_generic)(node)

// Forward declarations for _Generic dispatch
static bool lint_check_string(const char *str);
static bool lint_check_integer(int value);
static bool lint_check_generic(void *node);

// C17 atomic operations for thread-safe statistics
static inline void increment_stat(_Atomic(uint64_t) *counter, uint64_t value) {
    atomic_fetch_add_explicit(counter, value, memory_order_relaxed);
}

static inline uint64_t get_stat(_Atomic(uint64_t) *counter) {
    return atomic_load_explicit(counter, memory_order_relaxed);
}

static void print_usage(const char *program_name) {
    printf("Asthra Code Linter\n");
    printf("Usage: %s [options] <input_file>\n\n", program_name);
    printf("Options:\n");
    printf("  -v, --verbose           Verbose output\n");
    printf("  -W, --warnings-as-errors Treat warnings as errors\n");
    printf("  -s, --suggestions       Show code improvement suggestions\n");
    printf("  -j, --json              Output results in JSON format\n");
    printf("  -l, --level <level>     Minimum severity level (info, warning, error, critical)\n");
    printf("  -h, --help              Show this help message\n");
}

static LinterStatistics *create_linter_statistics(void) {
    LinterStatistics *stats = malloc(sizeof(LinterStatistics));
    if (!stats) {
        return NULL;
    }

    // C17 designated initializer with atomic initialization
    *stats = (LinterStatistics){.files_analyzed = 0,
                                .lines_analyzed = 0,
                                .warnings_found = 0,
                                .errors_found = 0,
                                .suggestions_made = 0};

    return stats;
}

static void destroy_linter_statistics(LinterStatistics *stats) {
    free(stats);
}

static LintSeverity parse_severity_level(const char *level_str) {
    // C17 compound literal for severity mapping
    static const struct {
        const char *name;
        LintSeverity level;
    } severity_map[] = {{"info", LINT_INFO},
                        {"warning", LINT_WARNING},
                        {"error", LINT_ERROR},
                        {"critical", LINT_CRITICAL}};

    for (size_t i = 0; i < sizeof(severity_map) / sizeof(severity_map[0]); i++) {
        if (strcmp(level_str, severity_map[i].name) == 0) {
            return severity_map[i].level;
        }
    }

    fprintf(stderr, "Warning: Unknown severity level '%s', using 'warning'\n", level_str);
    return LINT_WARNING;
}

static int parse_arguments(int argc, char *argv[], LinterOptions *options) {
    // C17 designated initializer for long options
    static struct option long_options[] = {
        {.name = "verbose", .has_arg = no_argument, .flag = 0, .val = 'v'},
        {.name = "warnings-as-errors", .has_arg = no_argument, .flag = 0, .val = 'W'},
        {.name = "suggestions", .has_arg = no_argument, .flag = 0, .val = 's'},
        {.name = "json", .has_arg = no_argument, .flag = 0, .val = 'j'},
        {.name = "level", .has_arg = required_argument, .flag = 0, .val = 'l'},
        {.name = "help", .has_arg = no_argument, .flag = 0, .val = 'h'},
        {0, 0, 0, 0}};

    // C17 designated initializer for default options
    *options = (LinterOptions){.input_file = NULL,
                               .verbose = false,
                               .warnings_as_errors = false,
                               .show_suggestions = false,
                               .json_output = false,
                               .min_severity = LINT_WARNING,
                               .stats = create_linter_statistics()};

    if (!options->stats) {
        fprintf(stderr, "Error: Failed to create linter statistics\n");
        return -1;
    }

    int c;
    int option_index = 0;

    while ((c = getopt_long(argc, argv, "vWsjl:h", long_options, &option_index)) != -1) {
        switch (c) {
        case 'v':
            options->verbose = true;
            break;
        case 'W':
            options->warnings_as_errors = true;
            break;
        case 's':
            options->show_suggestions = true;
            break;
        case 'j':
            options->json_output = true;
            break;
        case 'l':
            options->min_severity = parse_severity_level(optarg);
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

    return 0;
}

// C17 _Generic dispatch implementations
static bool lint_check_string(const char *str) {
    if (!str)
        return false;

    // Basic string validation
    size_t len = strlen(str);
    return len > 0 && len < 1024; // Reasonable string length
}

static bool lint_check_integer(int value) {
    // Basic integer validation
    return value >= 0 && value <= 1000000; // Reasonable range
}

static bool lint_check_generic(void *node) {
    (void)node;  // Suppress unused parameter warning
    return true; // Generic fallback - assume valid
}

// C17 compound literal for lint rules
typedef struct {
    const char *rule_name;
    const char *pattern;
    LintSeverity severity;
    const char *message;
    const char *suggestion;
} LintRule;

static const LintRule *get_lint_rules(void) {
    // C17 compound literal array
    static const LintRule rules[] = {
        {"naming-convention", "^[a-z][a-z0-9_]*$", LINT_WARNING,
         "Variable names should use snake_case",
         "Consider renaming to follow snake_case convention"},
        {"line-length", NULL, LINT_INFO, "Line exceeds 100 characters",
         "Consider breaking long lines for better readability"},
        {"unused-variable", NULL, LINT_WARNING, "Variable declared but never used",
         "Remove unused variable or mark with (void) if intentional"},
        {"magic-number", NULL, LINT_INFO, "Magic number detected",
         "Consider defining a named constant"},
        {"missing-documentation", NULL, LINT_INFO, "Function lacks documentation",
         "Add documentation comment describing function purpose"},
        {NULL, NULL, LINT_INFO, NULL, NULL} // Sentinel
    };
    return rules;
}

static void add_lint_issue(LintIssue **issues, size_t *count, size_t *capacity,
                           LintSeverity severity, size_t line, size_t column, const char *message,
                           const char *rule_name) {
    // Resize array if needed
    if (*count >= *capacity) {
        *capacity = (*capacity == 0) ? 16 : (*capacity * 2);
        LintIssue *new_issues = realloc(*issues, *capacity * sizeof(LintIssue));
        if (!new_issues) {
            return; // Out of memory
        }
        *issues = new_issues;
    }

    // C17 compound literal for issue initialization
    (*issues)[(*count)++] = (LintIssue){.severity = severity,
                                        .line = line,
                                        .column = column,
                                        .message = message,
                                        .rule_name = rule_name};
}

static LintIssue *analyze_file(const char *filename, LinterOptions *options, size_t *issue_count) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        *issue_count = 0;
        return NULL;
    }

    LintIssue *issues = NULL;
    size_t capacity = 0;
    *issue_count = 0;

    const LintRule *rules = get_lint_rules();
    char line_buffer[1024];
    size_t line_number = 0;
    uint64_t total_lines = 0;

    while (fgets(line_buffer, sizeof(line_buffer), file)) {
        line_number++;
        total_lines++;

        size_t line_length = strlen(line_buffer);

        // Check line length
        if (line_length > 100) {
            add_lint_issue(&issues, issue_count, &capacity, LINT_INFO, line_number, line_length,
                           "Line exceeds 100 characters", "line-length");
            increment_stat(&options->stats->suggestions_made, 1);
        }

        // Check for magic numbers (simplified)
        if (strstr(line_buffer, "42") || strstr(line_buffer, "100") ||
            strstr(line_buffer, "1024")) {
            add_lint_issue(&issues, issue_count, &capacity, LINT_INFO, line_number, 0,
                           "Potential magic number detected", "magic-number");
            increment_stat(&options->stats->suggestions_made, 1);
        }

        // Check for TODO/FIXME comments
        if (strstr(line_buffer, "TODO") || strstr(line_buffer, "FIXME")) {
            add_lint_issue(&issues, issue_count, &capacity, LINT_WARNING, line_number, 0,
                           "TODO/FIXME comment found", "todo-comment");
            increment_stat(&options->stats->warnings_found, 1);
        }

        // Check for potential unused variables (simplified)
        if (strstr(line_buffer, "int ") && !strstr(line_buffer, "=") && !strstr(line_buffer, "(")) {
            add_lint_issue(&issues, issue_count, &capacity, LINT_WARNING, line_number, 0,
                           "Potentially unused variable", "unused-variable");
            increment_stat(&options->stats->warnings_found, 1);
        }
    }

    fclose(file);

    // Update statistics atomically
    increment_stat(&options->stats->files_analyzed, 1);
    increment_stat(&options->stats->lines_analyzed, total_lines);

    return issues;
}

static const char *severity_to_string(LintSeverity severity) {
    // C17 compound literal for severity string mapping
    static const struct {
        LintSeverity severity;
        const char *string;
    } severity_strings[] = {{LINT_INFO, "info"},
                            {LINT_WARNING, "warning"},
                            {LINT_ERROR, "error"},
                            {LINT_CRITICAL, "critical"}};

    for (size_t i = 0; i < sizeof(severity_strings) / sizeof(severity_strings[0]); i++) {
        if (severity_strings[i].severity == severity) {
            return severity_strings[i].string;
        }
    }

    return "unknown";
}

static void print_issues_text(const LintIssue *issues, size_t count, const LinterOptions *options) {
    for (size_t i = 0; i < count; i++) {
        const LintIssue *issue = &issues[i];

        // Skip issues below minimum severity
        if (issue->severity < options->min_severity) {
            continue;
        }

        printf("%s:%zu:%zu: %s: %s [%s]\n", options->input_file, issue->line, issue->column,
               severity_to_string(issue->severity), issue->message, issue->rule_name);

        // Show suggestions if enabled
        if (options->show_suggestions && issue->severity == LINT_INFO) {
            printf("  Suggestion: Consider improving code quality\n");
        }
    }
}

static void print_issues_json(const LintIssue *issues, size_t count, const LinterOptions *options) {
    printf("{\n");
    printf("  \"file\": \"%s\",\n", options->input_file);
    printf("  \"issues\": [\n");

    bool first = true;
    for (size_t i = 0; i < count; i++) {
        const LintIssue *issue = &issues[i];

        // Skip issues below minimum severity
        if (issue->severity < options->min_severity) {
            continue;
        }

        if (!first) {
            printf(",\n");
        }
        first = false;

        printf("    {\n");
        printf("      \"line\": %zu,\n", issue->line);
        printf("      \"column\": %zu,\n", issue->column);
        printf("      \"severity\": \"%s\",\n", severity_to_string(issue->severity));
        printf("      \"message\": \"%s\",\n", issue->message);
        printf("      \"rule\": \"%s\"\n", issue->rule_name);
        printf("    }");
    }

    printf("\n  ]\n");
    printf("}\n");
}

static void print_statistics(const LinterStatistics *stats, bool verbose) {
    if (!verbose)
        return;

    printf("\nLinter Statistics:\n");
    printf("  Files analyzed: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->files_analyzed));
    printf("  Lines analyzed: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->lines_analyzed));
    printf("  Warnings found: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->warnings_found));
    printf("  Errors found: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->errors_found));
    printf("  Suggestions made: %llu\n",
           (unsigned long long)get_stat((_Atomic(uint64_t) *)&stats->suggestions_made));
}

int main(int argc, char *argv[]) {
    LinterOptions options;

    // Parse command line arguments
    int parse_result = parse_arguments(argc, argv, &options);
    if (parse_result != 0) {
        if (options.stats) {
            destroy_linter_statistics(options.stats);
        }
        return (parse_result > 0) ? 0 : 1; // 1 for help, -1 for error
    }

    if (options.verbose) {
        printf("Analyzing %s...\n", options.input_file);
    }

    // Analyze the file
    size_t issue_count;
    LintIssue *issues = analyze_file(options.input_file, &options, &issue_count);

    if (!issues && issue_count > 0) {
        fprintf(stderr, "Error: Failed to analyze file: %s\n", options.input_file);
        destroy_linter_statistics(options.stats);
        return 1;
    }

    // Print results
    if (options.json_output) {
        print_issues_json(issues, issue_count, &options);
    } else {
        if (issue_count == 0) {
            if (options.verbose) {
                printf("No issues found in %s\n", options.input_file);
            }
        } else {
            print_issues_text(issues, issue_count, &options);
        }
    }

    print_statistics(options.stats, options.verbose);

    // Determine exit code
    int exit_code = 0;
    uint64_t error_count = get_stat(&options.stats->errors_found);
    uint64_t warning_count = get_stat(&options.stats->warnings_found);

    if (error_count > 0) {
        exit_code = 1;
    } else if (options.warnings_as_errors && warning_count > 0) {
        exit_code = 1;
    }

    // Cleanup
    free(issues);
    destroy_linter_statistics(options.stats);

    return exit_code;
}
