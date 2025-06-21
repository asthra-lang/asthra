/**
 * Asthra Programming Language
 * Grammar Validation Tool
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Validates parser implementation against grammar.txt PEG grammar
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>
#include <ctype.h>

#include "common/cli_framework.h"
#include "common/error_framework.h"
#include "common/statistics_framework.h"
#include "../src/parser/parser.h"
#include "../src/parser/lexer.h"
#include "../runtime/asthra_runtime.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for grammar validator");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void*), "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe statistics
typedef struct {
    _Atomic(uint64_t) grammar_rules_parsed;
    _Atomic(uint64_t) parser_rules_checked;
    _Atomic(uint64_t) violations_found;
    _Atomic(uint64_t) compliance_score;
} ValidatorStatistics;

// Grammar rule representation
typedef struct GrammarRule {
    char *name;
    char *definition;
    bool is_terminal;
    bool is_implemented;
    int line_number;
    struct GrammarRule *next;
} GrammarRule;

// Validator configuration
typedef struct {
    const char *grammar_file;
    const char *output_file;
    const char *parser_source_dir;
    bool verbose;
    bool strict_mode;
    bool generate_report;
    bool check_coverage;
    ValidatorStatistics *stats;
} ValidatorOptions;

// Simple result type for tools
typedef struct {
    bool success;
    const char *error_message;
} ToolResult;

// Forward declarations
static ToolResult validate_grammar_compliance(ValidatorOptions *opts);
static ToolResult parse_grammar_file(const char *filename, GrammarRule **rules);
static ToolResult check_parser_implementation(GrammarRule *rules, ValidatorOptions *opts);
static ToolResult generate_compliance_report(GrammarRule *rules, ValidatorOptions *opts);
static void free_grammar_rules(GrammarRule *rules);
static bool is_rule_implemented_in_parser(const char *rule_name, const char *parser_dir);
static int calculate_compliance_score(GrammarRule *rules);
static void print_violation(const char *rule_name, const char *issue, ValidatorOptions *opts);
static ToolResult setup_cli_options(CliConfig *config);

// Global statistics instance
static ValidatorStatistics g_stats = {0};

int main(int argc, char **argv) {
    // C17 designated initializer for options
    ValidatorOptions opts = {
        .grammar_file = "grammar.txt",
        .output_file = "grammar_compliance_report.txt",
        .parser_source_dir = "src/parser",
        .verbose = false,
        .strict_mode = false,
        .generate_report = true,
        .check_coverage = true,
        .stats = &g_stats
    };
    
    // Setup CLI configuration
    CliConfig *config = cli_create_config(
        "Asthra Grammar Validator",
        "[options]",
        "Validate parser implementation against grammar.txt PEG grammar"
    );
    
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
    if (grammar) opts.grammar_file = grammar;
    
    const char *output = cli_get_string_option(values, 16, "output");
    if (output) opts.output_file = output;
    
    const char *parser_dir = cli_get_string_option(values, 16, "parser-dir");
    if (parser_dir) opts.parser_source_dir = parser_dir;
    
    opts.verbose = cli_get_bool_option(values, 16, "verbose");
    opts.strict_mode = cli_get_bool_option(values, 16, "strict");
    opts.generate_report = !cli_get_bool_option(values, 16, "no-report");
    opts.check_coverage = !cli_get_bool_option(values, 16, "no-coverage");
    
    // Validate input files
    if (!cli_validate_file_exists(opts.grammar_file)) {
        fprintf(stderr, "Error: Grammar file '%s' does not exist\n", opts.grammar_file);
        cli_destroy_config(config);
        return 1;
    }
    
    if (!cli_validate_directory_exists(opts.parser_source_dir)) {
        fprintf(stderr, "Error: Parser source directory '%s' does not exist\n", opts.parser_source_dir);
        cli_destroy_config(config);
        return 1;
    }
    
    // Run grammar validation
    printf("Validating grammar compliance...\n");
    printf("Grammar file: %s\n", opts.grammar_file);
    printf("Parser directory: %s\n", opts.parser_source_dir);
    
    ToolResult validation_result = validate_grammar_compliance(&opts);
    
    if (validation_result.success) {
        printf("Grammar validation completed successfully\n");
        printf("Statistics:\n");
        printf("  Grammar rules parsed: %llu\n", (unsigned long long)atomic_load(&g_stats.grammar_rules_parsed));
        printf("  Parser rules checked: %llu\n", (unsigned long long)atomic_load(&g_stats.parser_rules_checked));
        printf("  Violations found: %llu\n", (unsigned long long)atomic_load(&g_stats.violations_found));
        printf("  Compliance score: %llu%%\n", (unsigned long long)atomic_load(&g_stats.compliance_score));
        
        if (opts.generate_report) {
            printf("Compliance report generated: %s\n", opts.output_file);
        }
    } else {
        fprintf(stderr, "Grammar validation failed: %s\n", validation_result.error_message);
    }
    
    cli_destroy_config(config);
    return validation_result.success ? 0 : 1;
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
    
    // Output file option
    if (cli_add_option(config, "output", 'o', true, false,
                       "Output report file (default: grammar_compliance_report.txt)") != 0) {
        result.success = false;
        result.error_message = "Failed to add output option";
        return result;
    }
    
    // Parser directory option
    if (cli_add_option(config, "parser-dir", 'p', true, false,
                       "Parser source directory (default: src/parser)") != 0) {
        result.success = false;
        result.error_message = "Failed to add parser-dir option";
        return result;
    }
    
    // Verbose option
    if (cli_add_option(config, "verbose", 'v', false, false,
                       "Enable verbose output") != 0) {
        result.success = false;
        result.error_message = "Failed to add verbose option";
        return result;
    }
    
    // Strict mode option
    if (cli_add_option(config, "strict", 's', false, false,
                       "Enable strict validation mode") != 0) {
        result.success = false;
        result.error_message = "Failed to add strict option";
        return result;
    }
    
    // No report option
    if (cli_add_option(config, "no-report", 'n', false, false,
                       "Skip generating compliance report") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-report option";
        return result;
    }
    
    // No coverage option
    if (cli_add_option(config, "no-coverage", 'c', false, false,
                       "Skip checking implementation coverage") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-coverage option";
        return result;
    }
    
    return result;
}

static ToolResult validate_grammar_compliance(ValidatorOptions *opts) {
    ToolResult result = {.success = true};
    GrammarRule *rules = NULL;
    
    // Parse grammar file
    result = parse_grammar_file(opts->grammar_file, &rules);
    if (!result.success) {
        return result;
    }
    
    if (opts->verbose) {
        printf("Parsed %llu grammar rules\n", (unsigned long long)atomic_load(&g_stats.grammar_rules_parsed));
    }
    
    // Check parser implementation
    if (opts->check_coverage) {
        result = check_parser_implementation(rules, opts);
        if (!result.success) {
            free_grammar_rules(rules);
            return result;
        }
    }
    
    // Calculate compliance score
    int score = calculate_compliance_score(rules);
    atomic_store(&g_stats.compliance_score, score);
    
    // Generate report
    if (opts->generate_report) {
        result = generate_compliance_report(rules, opts);
        if (!result.success) {
            free_grammar_rules(rules);
            return result;
        }
    }
    
    free_grammar_rules(rules);
    return result;
}

static ToolResult parse_grammar_file(const char *filename, GrammarRule **rules) {
    ToolResult result = {.success = true};
    FILE *file = fopen(filename, "r");
    
    if (!file) {
        result.success = false;
        result.error_message = "Failed to open grammar file";
        return result;
    }
    
    char line[1024];
    int line_number = 0;
    GrammarRule *current = NULL;
    
    while (fgets(line, sizeof(line), file)) {
        line_number++;
        
        // Skip comments and empty lines
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }
        
        // Look for grammar rule definitions (contains <-)
        char *arrow = strstr(line, "<-");
        if (arrow) {
            // Extract rule name (before <-)
            char *rule_name = strtok(line, " \t<-");
            if (rule_name) {
                // Create new rule
                GrammarRule *rule = malloc(sizeof(GrammarRule));
                if (!rule) {
                    result.success = false;
                    result.error_message = "Memory allocation failed";
                    fclose(file);
                    return result;
                }
                
                rule->name = strdup(rule_name);
                rule->definition = strdup(arrow + 2); // Skip "<-"
                rule->is_terminal = islower(rule_name[0]); // Convention: terminals start lowercase
                rule->is_implemented = false;
                rule->line_number = line_number;
                rule->next = NULL;
                
                // Add to list
                if (!*rules) {
                    *rules = rule;
                    current = rule;
                } else {
                    current->next = rule;
                    current = rule;
                }
                
                atomic_fetch_add(&g_stats.grammar_rules_parsed, 1);
            }
        }
    }
    
    fclose(file);
    return result;
}

static ToolResult check_parser_implementation(GrammarRule *rules, ValidatorOptions *opts) {
    ToolResult result = {.success = true};
    
    for (GrammarRule *rule = rules; rule; rule = rule->next) {
        if (is_rule_implemented_in_parser(rule->name, opts->parser_source_dir)) {
            rule->is_implemented = true;
        } else {
            if (opts->verbose) {
                print_violation(rule->name, "Not implemented in parser", opts);
            }
            atomic_fetch_add(&g_stats.violations_found, 1);
        }
        atomic_fetch_add(&g_stats.parser_rules_checked, 1);
    }
    
    return result;
}

static ToolResult generate_compliance_report(GrammarRule *rules, ValidatorOptions *opts) {
    ToolResult result = {.success = true};
    FILE *report = fopen(opts->output_file, "w");
    
    if (!report) {
        result.success = false;
        result.error_message = "Failed to create report file";
        return result;
    }
    
    // Write report header
    fprintf(report, "Asthra Grammar Compliance Report\n");
    fprintf(report, "==============================\n\n");
    fprintf(report, "Grammar file: %s\n", opts->grammar_file);
    fprintf(report, "Parser directory: %s\n", opts->parser_source_dir);
    fprintf(report, "Generated: %s\n", __DATE__ " " __TIME__);
    fprintf(report, "\nStatistics:\n");
    fprintf(report, "  Grammar rules parsed: %llu\n", (unsigned long long)atomic_load(&g_stats.grammar_rules_parsed));
    fprintf(report, "  Parser rules checked: %llu\n", (unsigned long long)atomic_load(&g_stats.parser_rules_checked));
    fprintf(report, "  Violations found: %llu\n", (unsigned long long)atomic_load(&g_stats.violations_found));
    fprintf(report, "  Compliance score: %llu%%\n\n", (unsigned long long)atomic_load(&g_stats.compliance_score));
    
    // Write detailed rule analysis
    fprintf(report, "Rule Analysis:\n");
    fprintf(report, "=============\n\n");
    
    for (GrammarRule *rule = rules; rule; rule = rule->next) {
        fprintf(report, "Rule: %s (line %d)\n", rule->name, rule->line_number);
        fprintf(report, "  Status: %s\n", rule->is_implemented ? "IMPLEMENTED" : "NOT IMPLEMENTED");
        fprintf(report, "  Type: %s\n", rule->is_terminal ? "Terminal" : "Non-terminal");
        fprintf(report, "  Definition: %s", rule->definition);
        fprintf(report, "\n");
    }
    
    // Write recommendations
    fprintf(report, "\nRecommendations:\n");
    fprintf(report, "===============\n\n");
    
    for (GrammarRule *rule = rules; rule; rule = rule->next) {
        if (!rule->is_implemented) {
            fprintf(report, "- Implement parser support for rule '%s'\n", rule->name);
        }
    }
    
    fclose(report);
    return result;
}

static bool is_rule_implemented_in_parser(const char *rule_name, const char *parser_dir) {
    // Simple heuristic: check if rule name appears in parser source files
    char command[512];
    snprintf(command, sizeof(command), "grep -r \"parse_%s\\|%s\" %s > /dev/null 2>&1", 
             rule_name, rule_name, parser_dir);
    
    return system(command) == 0;
}

static int calculate_compliance_score(GrammarRule *rules) {
    int total_rules = 0;
    int implemented_rules = 0;
    
    for (GrammarRule *rule = rules; rule; rule = rule->next) {
        total_rules++;
        if (rule->is_implemented) {
            implemented_rules++;
        }
    }
    
    if (total_rules == 0) return 0;
    return (implemented_rules * 100) / total_rules;
}

static void print_violation(const char *rule_name, const char *issue, ValidatorOptions *opts) {
    printf("VIOLATION: Rule '%s' - %s\n", rule_name, issue);
}

static void free_grammar_rules(GrammarRule *rules) {
    while (rules) {
        GrammarRule *next = rules->next;
        free(rules->name);
        free(rules->definition);
        free(rules);
        rules = next;
    }
}
