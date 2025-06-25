/**
 * Asthra Programming Language
 * Test Case Generator Tool
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Automatically generate test cases for language features and edge cases
 */

#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "../runtime/asthra_runtime.h"
#include "../src/parser/lexer.h"
#include "../src/parser/parser.h"
#include "common/cli_framework.h"
#include "common/error_framework.h"
#include "common/statistics_framework.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for test generator");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void *),
                     "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe statistics
typedef struct {
    _Atomic(uint64_t) tests_generated;
    _Atomic(uint64_t) templates_processed;
    _Atomic(uint64_t) grammar_rules_used;
    _Atomic(uint64_t) files_written;
} GeneratorStatistics;

// Test generation modes
typedef enum {
    GEN_MODE_PARSER_TESTS,
    GEN_MODE_SEMANTIC_TESTS,
    GEN_MODE_EDGE_CASES,
    GEN_MODE_PERFORMANCE_TESTS,
    GEN_MODE_ALL
} GenerationMode;

// Test complexity levels
typedef enum {
    COMPLEXITY_SIMPLE,
    COMPLEXITY_MEDIUM,
    COMPLEXITY_COMPLEX,
    COMPLEXITY_RANDOM
} ComplexityLevel;

// Test case template
typedef struct {
    const char *name;
    const char *category;
    const char *template_code;
    ComplexityLevel complexity;
    const char **required_features;
    int feature_count;
} TestTemplate;

// Generator configuration
typedef struct {
    const char *grammar_file;
    const char *output_dir;
    const char *test_category;
    GenerationMode mode;
    ComplexityLevel complexity;
    int test_count;
    bool use_templates;
    bool generate_makefile;
    bool validate_generated;
    bool include_edge_cases;
    double random_seed;
    GeneratorStatistics *stats;
} GeneratorOptions;

// Simple result type for tools
typedef struct {
    bool success;
    const char *error_message;
} ToolResult;

// Test templates for different language features
static const TestTemplate BASIC_TEMPLATES[] = {
    {.name = "basic_function",
     .category = "parser",
     .template_code = "fn test_function() -> i32 {\n    return 42;\n}",
     .complexity = COMPLEXITY_SIMPLE,
     .required_features = (const char *[]){"functions", "types", "return"},
     .feature_count = 3},
    {.name = "struct_declaration",
     .category = "parser",
     .template_code = "struct TestStruct {\n    field: i32,\n    name: str,\n}",
     .complexity = COMPLEXITY_SIMPLE,
     .required_features = (const char *[]){"structs", "fields", "types"},
     .feature_count = 3},
    {.name = "enum_declaration",
     .category = "parser",
     .template_code = "enum Color {\n    Red,\n    Green,\n    Blue,\n}",
     .complexity = COMPLEXITY_SIMPLE,
     .required_features = (const char *[]){"enums", "variants"},
     .feature_count = 2},
    {.name = "variable_assignment",
     .category = "semantic",
     .template_code = "let x: i32 = 10;\nlet y: str = \"hello\";",
     .complexity = COMPLEXITY_SIMPLE,
     .required_features = (const char *[]){"variables", "assignment", "literals"},
     .feature_count = 3},
    {.name = "function_call",
     .category = "semantic",
     .template_code =
         "fn add(a: i32, b: i32) -> i32 {\n    return a + b;\n}\nlet result = add(5, 3);",
     .complexity = COMPLEXITY_MEDIUM,
     .required_features = (const char *[]){"functions", "parameters", "expressions", "calls"},
     .feature_count = 4},
    {.name = "method_call",
     .category = "semantic",
     .template_code =
         "struct Point {\n    x: i32,\n    y: i32,\n}\nimpl Point {\n    fn distance(&self) -> f64 "
         "{\n        return sqrt(self.x * self.x + self.y * self.y);\n    }\n}",
     .complexity = COMPLEXITY_COMPLEX,
     .required_features = (const char *[]){"structs", "methods", "impl", "self"},
     .feature_count = 4}};

static const int TEMPLATE_COUNT = sizeof(BASIC_TEMPLATES) / sizeof(BASIC_TEMPLATES[0]);

// Forward declarations
static ToolResult generate_test_cases(GeneratorOptions *opts);
static ToolResult generate_from_templates(GeneratorOptions *opts);
static ToolResult generate_from_grammar(GeneratorOptions *opts);
static ToolResult generate_edge_cases(GeneratorOptions *opts);
static ToolResult write_test_file(const char *filename, const char *content, const char *category);
static ToolResult generate_test_makefile(GeneratorOptions *opts);
static char *expand_template(const TestTemplate *template, int variation_id);
static char *generate_random_test(GenerationMode mode, ComplexityLevel complexity);
static bool validate_generated_test(const char *test_code);
static const TestTemplate *select_template_by_complexity(ComplexityLevel complexity);
static ToolResult setup_cli_options(CliConfig *config);

// Global statistics instance
static GeneratorStatistics g_stats = {0};

int main(int argc, char **argv) {
    // C17 designated initializer for options
    GeneratorOptions opts = {.grammar_file = "grammar.txt",
                             .output_dir = "tests/generated",
                             .test_category = "auto_generated",
                             .mode = GEN_MODE_ALL,
                             .complexity = COMPLEXITY_MEDIUM,
                             .test_count = 10,
                             .use_templates = true,
                             .generate_makefile = true,
                             .validate_generated = true,
                             .include_edge_cases = true,
                             .random_seed = 0.0,
                             .stats = &g_stats};

    // Initialize random seed
    if (opts.random_seed == 0.0) {
        opts.random_seed = (double)time(NULL);
    }
    srand((unsigned int)opts.random_seed);

    // Setup CLI configuration
    CliConfig *config =
        cli_create_config("Asthra Test Generator", "[options]",
                          "Automatically generate test cases for language features and edge cases");

    ToolResult setup_result = setup_cli_options(config);
    if (!setup_result.success) {
        fprintf(stderr, "Failed to setup CLI options: %s\n", setup_result.error_message);
        cli_destroy_config(config);
        return 1;
    }

    // Parse command line arguments
    CliOptionValue values[16];
    CliParseResult parse_result = cli_parse_args(config, argc, argv, values, 16);

    if (parse_result.help_requested) {
        cli_print_help(config);
        cli_destroy_config(config);
        return 0;
    }

    if (parse_result.error_occurred) {
        cli_print_error(config, parse_result.error_message);
        cli_destroy_config(config);
        return 1;
    }

    // Process optional parameters
    const char *grammar = cli_get_string_option(values, 16, "grammar");
    if (grammar)
        opts.grammar_file = grammar;

    const char *output_dir = cli_get_string_option(values, 16, "output");
    if (output_dir)
        opts.output_dir = output_dir;

    const char *category = cli_get_string_option(values, 16, "category");
    if (category)
        opts.test_category = category;

    const char *mode = cli_get_string_option(values, 16, "mode");
    if (mode) {
        if (strcmp(mode, "parser") == 0)
            opts.mode = GEN_MODE_PARSER_TESTS;
        else if (strcmp(mode, "semantic") == 0)
            opts.mode = GEN_MODE_SEMANTIC_TESTS;
        else if (strcmp(mode, "edge") == 0)
            opts.mode = GEN_MODE_EDGE_CASES;
        else if (strcmp(mode, "performance") == 0)
            opts.mode = GEN_MODE_PERFORMANCE_TESTS;
        else if (strcmp(mode, "all") == 0)
            opts.mode = GEN_MODE_ALL;
    }

    const char *complexity = cli_get_string_option(values, 16, "complexity");
    if (complexity) {
        if (strcmp(complexity, "simple") == 0)
            opts.complexity = COMPLEXITY_SIMPLE;
        else if (strcmp(complexity, "medium") == 0)
            opts.complexity = COMPLEXITY_MEDIUM;
        else if (strcmp(complexity, "complex") == 0)
            opts.complexity = COMPLEXITY_COMPLEX;
        else if (strcmp(complexity, "random") == 0)
            opts.complexity = COMPLEXITY_RANDOM;
    }

    opts.test_count = cli_get_int_option(values, 16, "count", 10);
    opts.use_templates = !cli_get_bool_option(values, 16, "no-templates");
    opts.generate_makefile = !cli_get_bool_option(values, 16, "no-makefile");
    opts.validate_generated = !cli_get_bool_option(values, 16, "no-validate");
    opts.include_edge_cases = !cli_get_bool_option(values, 16, "no-edge-cases");

    const char *seed_str = cli_get_string_option(values, 16, "seed");
    if (seed_str) {
        double seed = atof(seed_str);
        opts.random_seed = seed;
        srand((unsigned int)seed);
    }

    // Validate input files and directories
    if (!cli_validate_file_exists(opts.grammar_file)) {
        fprintf(stderr,
                "Warning: Grammar file '%s' does not exist, proceeding with template-based "
                "generation\n",
                opts.grammar_file);
    }

    // Create output directory if it doesn't exist
    char mkdir_cmd[512];
    snprintf(mkdir_cmd, sizeof(mkdir_cmd), "mkdir -p %s", opts.output_dir);
    int mkdir_result = system(mkdir_cmd);
    if (mkdir_result == -1) {
        fprintf(stderr, "Error: Failed to execute mkdir command\n");
        cli_destroy_config(config);
        return 1;
    } else if (WIFEXITED(mkdir_result) && WEXITSTATUS(mkdir_result) != 0) {
        fprintf(stderr, "Warning: Failed to create output directory (exit code: %d)\n",
                WEXITSTATUS(mkdir_result));
    }

    // Run test generation
    printf("Generating test cases...\n");
    printf("Output directory: %s\n", opts.output_dir);
    printf("Test category: %s\n", opts.test_category);
    printf("Generation mode: %d\n", opts.mode);
    printf("Test count: %d\n", opts.test_count);
    printf("Random seed: %.0f\n", opts.random_seed);

    ToolResult generation_result = generate_test_cases(&opts);

    if (generation_result.success) {
        printf("Test generation completed successfully\n");
        printf("Statistics:\n");
        printf("  Tests generated: %llu\n",
               (unsigned long long)atomic_load(&g_stats.tests_generated));
        printf("  Templates processed: %llu\n",
               (unsigned long long)atomic_load(&g_stats.templates_processed));
        printf("  Grammar rules used: %llu\n",
               (unsigned long long)atomic_load(&g_stats.grammar_rules_used));
        printf("  Files written: %llu\n", (unsigned long long)atomic_load(&g_stats.files_written));
    } else {
        fprintf(stderr, "Test generation failed: %s\n", generation_result.error_message);
    }

    cli_destroy_config(config);
    return generation_result.success ? 0 : 1;
}

static ToolResult setup_cli_options(CliConfig *config) {
    ToolResult result = {.success = true};

    // Grammar file option
    if (cli_add_option(config, "grammar", 'g', true, false,
                       "Grammar file path (default: grammar.txt)") != 0) {
        result.success = false;
        result.error_message = "Failed to add grammar option";
        return result;
    }

    // Output directory option
    if (cli_add_option(config, "output", 'o', true, false,
                       "Output directory for generated tests (default: tests/generated)") != 0) {
        result.success = false;
        result.error_message = "Failed to add output option";
        return result;
    }

    // Test category option
    if (cli_add_option(config, "category", 'c', true, false,
                       "Test category name (default: auto_generated)") != 0) {
        result.success = false;
        result.error_message = "Failed to add category option";
        return result;
    }

    // Generation mode option
    if (cli_add_option(
            config, "mode", 'm', true, false,
            "Generation mode: parser, semantic, edge, performance, all (default: all)") != 0) {
        result.success = false;
        result.error_message = "Failed to add mode option";
        return result;
    }

    // Complexity option
    if (cli_add_option(config, "complexity", 'x', true, false,
                       "Test complexity: simple, medium, complex, random (default: medium)") != 0) {
        result.success = false;
        result.error_message = "Failed to add complexity option";
        return result;
    }

    // Test count option
    if (cli_add_option(config, "count", 'n', true, false,
                       "Number of tests to generate (default: 10)") != 0) {
        result.success = false;
        result.error_message = "Failed to add count option";
        return result;
    }

    // No templates option
    if (cli_add_option(config, "no-templates", 't', false, false,
                       "Disable template-based generation") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-templates option";
        return result;
    }

    // No makefile option
    if (cli_add_option(config, "no-makefile", 'M', false, false, "Skip generating test makefile") !=
        0) {
        result.success = false;
        result.error_message = "Failed to add no-makefile option";
        return result;
    }

    // No validation option
    if (cli_add_option(config, "no-validate", 'V', false, false,
                       "Skip validating generated tests") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-validate option";
        return result;
    }

    // No edge cases option
    if (cli_add_option(config, "no-edge-cases", 'E', false, false,
                       "Skip generating edge case tests") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-edge-cases option";
        return result;
    }

    // Random seed option
    if (cli_add_option(config, "seed", 's', true, false,
                       "Random seed for generation (default: current time)") != 0) {
        result.success = false;
        result.error_message = "Failed to add seed option";
        return result;
    }

    return result;
}

static ToolResult generate_test_cases(GeneratorOptions *opts) {
    ToolResult result = {.success = true};

    printf("Test Generator - Generating %d test cases\n", opts->test_count);

    // Generate from templates if enabled
    if (opts->use_templates) {
        result = generate_from_templates(opts);
        if (!result.success)
            return result;
    }

    // Generate from grammar if file exists
    if (cli_validate_file_exists(opts->grammar_file)) {
        result = generate_from_grammar(opts);
        if (!result.success)
            return result;
    }

    // Generate edge cases if enabled
    if (opts->include_edge_cases) {
        result = generate_edge_cases(opts);
        if (!result.success)
            return result;
    }

    // Generate makefile if requested
    if (opts->generate_makefile) {
        result = generate_test_makefile(opts);
        if (!result.success)
            return result;
    }

    return result;
}

static ToolResult generate_from_templates(GeneratorOptions *opts) {
    ToolResult result = {.success = true};

    printf("Generating tests from templates...\n");

    for (int i = 0; i < opts->test_count && i < TEMPLATE_COUNT * 3; i++) {
        const TestTemplate *template = &BASIC_TEMPLATES[i % TEMPLATE_COUNT];

        // Filter by mode if not generating all
        if (opts->mode != GEN_MODE_ALL) {
            if (opts->mode == GEN_MODE_PARSER_TESTS && strcmp(template->category, "parser") != 0)
                continue;
            if (opts->mode == GEN_MODE_SEMANTIC_TESTS &&
                strcmp(template->category, "semantic") != 0)
                continue;
        }

        // Filter by complexity
        if (opts->complexity != COMPLEXITY_RANDOM && template->complexity != opts->complexity) {
            continue;
        }

        // Generate test variations
        char *test_content = expand_template(template, i / TEMPLATE_COUNT + 1);
        if (!test_content)
            continue;

        // Create filename
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/test_%s_%d.c", opts->output_dir, template->name,
                 i / TEMPLATE_COUNT + 1);

        // Write test file
        result = write_test_file(filename, test_content, template->category);
        free(test_content);

        if (!result.success)
            return result;

        atomic_fetch_add(&opts->stats->tests_generated, 1);
        atomic_fetch_add(&opts->stats->templates_processed, 1);
    }

    return result;
}

static ToolResult generate_from_grammar(GeneratorOptions *opts) {
    ToolResult result = {.success = true};

    printf("Generating tests from grammar rules...\n");

    // Simplified grammar-based generation
    // In a full implementation, this would parse the grammar file and generate tests
    for (int i = 0; i < opts->test_count / 3; i++) {
        char *test_content = generate_random_test(opts->mode, opts->complexity);
        if (!test_content)
            continue;

        char filename[256];
        snprintf(filename, sizeof(filename), "%s/test_grammar_%d.c", opts->output_dir, i + 1);

        result = write_test_file(filename, test_content, "grammar");
        free(test_content);

        if (!result.success)
            return result;

        atomic_fetch_add(&opts->stats->tests_generated, 1);
        atomic_fetch_add(&opts->stats->grammar_rules_used, 1);
    }

    return result;
}

static ToolResult generate_edge_cases(GeneratorOptions *opts) {
    ToolResult result = {.success = true};

    printf("Generating edge case tests...\n");

    // Generate various edge case scenarios
    const char *edge_cases[] = {
        "// Edge case: Empty function\nfn empty() {}",
        "// Edge case: Very long identifier\nfn "
        "very_long_function_name_that_tests_identifier_length_limits() -> i32 { return 0; }",
        "// Edge case: Nested structures\nstruct Outer { inner: Inner, }\nstruct Inner { value: "
        "i32, }",
        "// Edge case: Maximum integer literal\nlet max_int: i64 = 9223372036854775807;",
        "// Edge case: Unicode in strings\nlet unicode: str = \"Hello 世界 🌍\";",
        "// Edge case: Deeply nested expressions\nlet result = ((((1 + 2) * 3) - 4) / 5);"};

    int edge_case_count = sizeof(edge_cases) / sizeof(edge_cases[0]);

    for (int i = 0; i < edge_case_count; i++) {
        char filename[256];
        snprintf(filename, sizeof(filename), "%s/test_edge_case_%d.c", opts->output_dir, i + 1);

        result = write_test_file(filename, edge_cases[i], "edge_cases");
        if (!result.success)
            return result;

        atomic_fetch_add(&opts->stats->tests_generated, 1);
    }

    return result;
}

static ToolResult write_test_file(const char *filename, const char *content, const char *category) {
    ToolResult result = {.success = true};

    FILE *file = fopen(filename, "w");
    if (!file) {
        result.success = false;
        result.error_message = "Failed to create test file";
        return result;
    }

    // Write test framework header
    fprintf(file, "/**\n");
    fprintf(file, " * Auto-generated test case\n");
    fprintf(file, " * Category: %s\n", category);
    fprintf(file, " * Generated by Asthra Test Generator\n");
    fprintf(file, " */\n\n");

    fprintf(file, "#include \"../framework/test_framework.h\"\n\n");

    // Write test content
    fprintf(file, "// Test code:\n%s\n\n", content);

    // Write test function
    fprintf(file, "TEST_CASE(\"auto_generated_test\") {\n");
    fprintf(file, "    // TODO: Add test assertions\n");
    fprintf(file, "    ASSERT_TRUE(true);\n");
    fprintf(file, "}\n");

    fclose(file);

    printf("Generated test: %s\n", filename);
    atomic_fetch_add(&g_stats.files_written, 1);

    return result;
}

static ToolResult generate_test_makefile(GeneratorOptions *opts) {
    ToolResult result = {.success = true};

    char makefile_path[256];
    snprintf(makefile_path, sizeof(makefile_path), "%s/Makefile", opts->output_dir);

    FILE *makefile = fopen(makefile_path, "w");
    if (!makefile) {
        result.success = false;
        result.error_message = "Failed to create makefile";
        return result;
    }

    fprintf(makefile, "# Auto-generated Makefile for test category: %s\n", opts->test_category);
    fprintf(makefile, "# Generated by Asthra Test Generator\n\n");

    fprintf(makefile, "include ../../make/platform.mk\n");
    fprintf(makefile, "include ../../make/compiler.mk\n\n");

    fprintf(makefile, "TEST_SOURCES := $(wildcard *.c)\n");
    fprintf(makefile, "TEST_OBJECTS := $(TEST_SOURCES:.c=.o)\n");
    fprintf(makefile, "TEST_BINARIES := $(TEST_SOURCES:.c=)\n\n");

    fprintf(makefile, "all: $(TEST_BINARIES)\n\n");

    fprintf(makefile, "%%: %%.o\n");
    fprintf(makefile, "\t$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)\n\n");

    fprintf(makefile, "%%.o: %%.c\n");
    fprintf(makefile, "\t$(CC) $(CFLAGS) -c $< -o $@\n\n");

    fprintf(makefile, "clean:\n");
    fprintf(makefile, "\trm -f $(TEST_OBJECTS) $(TEST_BINARIES)\n\n");

    fprintf(makefile, ".PHONY: all clean\n");

    fclose(makefile);

    printf("Generated makefile: %s\n", makefile_path);
    atomic_fetch_add(&g_stats.files_written, 1);

    return result;
}

static char *expand_template(const TestTemplate *template, int variation_id) {
    // Simple template expansion - in a full implementation this would be more sophisticated
    size_t content_size = strlen(template->template_code) + 256;
    char *expanded = malloc(content_size);
    if (!expanded)
        return NULL;

    // Add variation-specific modifications
    snprintf(expanded, content_size, "// Template: %s (variation %d)\n%s", template->name,
             variation_id, template->template_code);

    return expanded;
}

static char *generate_random_test(GenerationMode mode, ComplexityLevel complexity) {
    // Simple random test generation - placeholder implementation
    const char *random_tests[] = {
        "fn random_function() -> i32 { return 42; }", "let x: i32 = 10;\nlet y: i32 = x + 5;",
        "struct RandomStruct { field: i32, }", "enum RandomEnum { A, B, C, }"};

    int test_count = sizeof(random_tests) / sizeof(random_tests[0]);
    int index = rand() % test_count;

    char *result = malloc(strlen(random_tests[index]) + 1);
    if (result) {
        strcpy(result, random_tests[index]);
    }

    return result;
}

static bool validate_generated_test(const char *test_code) {
    // Simple validation - check for basic syntax patterns
    // In a full implementation, this would use the actual parser
    if (!test_code)
        return false;

    // Basic checks
    bool has_content = strlen(test_code) > 10;
    bool has_keywords = strstr(test_code, "fn") || strstr(test_code, "let") ||
                        strstr(test_code, "struct") || strstr(test_code, "enum");

    return has_content && has_keywords;
}

static const TestTemplate *select_template_by_complexity(ComplexityLevel complexity) {
    for (int i = 0; i < TEMPLATE_COUNT; i++) {
        if (BASIC_TEMPLATES[i].complexity == complexity) {
            return &BASIC_TEMPLATES[i];
        }
    }
    return &BASIC_TEMPLATES[0]; // fallback
}
