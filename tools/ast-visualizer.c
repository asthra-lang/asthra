/**
 * Asthra Programming Language
 * AST Visualization Tool
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Generates visual representations of AST structures for debugging parser issues
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

#include "common/cli_framework.h"
#include "common/error_framework.h"
#include "common/statistics_framework.h"
#include "../src/parser/ast.h"
#include "../src/parser/parser.h"
#include "../src/parser/lexer.h"
#include "../runtime/asthra_runtime.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for AST visualizer");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void*), "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe statistics
typedef struct {
    _Atomic(uint64_t) nodes_processed;
    _Atomic(uint64_t) edges_generated;
    _Atomic(uint64_t) bytes_written;
    _Atomic(uint64_t) max_depth_reached;
} VisualizerStatistics;

// Visualizer configuration
typedef struct {
    const char *input_file;
    const char *output_file;
    const char *format;
    bool show_types;
    bool show_locations;
    bool compact_view;
    bool interactive;
    int max_depth;
    const char *filter_pattern;
    VisualizerStatistics *stats;
} VisualizerOptions;

// Simple result type for tools
typedef struct {
    bool success;
    const char *error_message;
} ToolResult;

// Forward declarations
static ToolResult visualize_ast_tree(ASTNode *root, VisualizerOptions *opts);
static ToolResult generate_dot_output(ASTNode *root, FILE *output, VisualizerOptions *opts);
static ToolResult traverse_ast_node(ASTNode *node, FILE *output, VisualizerOptions *opts, int depth);
static const char* get_node_type_name(ASTNodeType type);
static const char* get_node_label(ASTNode *node, VisualizerOptions *opts);
static bool should_include_node(ASTNode *node, VisualizerOptions *opts);
static void write_node_attributes(ASTNode *node, FILE *output, VisualizerOptions *opts);
static void write_edge(ASTNode *parent, ASTNode *child, FILE *output, const char *label);
static ToolResult setup_cli_options(CliConfig *config);

// Global statistics instance
static VisualizerStatistics g_stats = {0};

int main(int argc, char **argv) {
    // C17 designated initializer for options
    VisualizerOptions opts = {
        .input_file = NULL,
        .output_file = "ast_output.dot",
        .format = "dot",
        .show_types = true,
        .show_locations = false,
        .compact_view = false,
        .interactive = false,
        .max_depth = -1,
        .filter_pattern = NULL,
        .stats = &g_stats
    };
    
    // Setup CLI configuration
    CliConfig *config = cli_create_config(
        "Asthra AST Visualizer",
        "[options] <input_file>",
        "Generate visual representations of AST structures for debugging parser issues"
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
    
    // Extract options from parsed values
    if (parse_result.remaining_count < 1) {
        cli_print_error(config, "Input file required");
        cli_destroy_config(config);
        return 1;
    }
    
    opts.input_file = parse_result.remaining_args[0];
    
    // Process optional parameters
    const char *output = cli_get_string_option(values, 16, "output");
    if (output) opts.output_file = output;
    
    const char *format = cli_get_string_option(values, 16, "format");
    if (format) opts.format = format;
    
    opts.show_types = !cli_get_bool_option(values, 16, "no-types");
    opts.show_locations = cli_get_bool_option(values, 16, "locations");
    opts.compact_view = cli_get_bool_option(values, 16, "compact");
    opts.interactive = cli_get_bool_option(values, 16, "interactive");
    opts.max_depth = cli_get_int_option(values, 16, "max-depth", -1);
    
    const char *filter = cli_get_string_option(values, 16, "filter");
    if (filter) opts.filter_pattern = filter;
    
    // Validate input file
    if (!cli_validate_file_exists(opts.input_file)) {
        fprintf(stderr, "Error: Input file '%s' does not exist\n", opts.input_file);
        cli_destroy_config(config);
        return 1;
    }
    
    // Parse input file and generate AST
    printf("Parsing Asthra source file: %s\n", opts.input_file);
    
    // Read file content
    FILE *file = fopen(opts.input_file, "r");
    if (!file) {
        fprintf(stderr, "Failed to open input file: %s\n", opts.input_file);
        cli_destroy_config(config);
        return 1;
    }
    
    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char *source = malloc((size_t)file_size + 1);
    if (!source) {
        fprintf(stderr, "Failed to allocate memory for source\n");
        fclose(file);
        cli_destroy_config(config);
        return 1;
    }
    
    fread(source, 1, (size_t)file_size, file);
    source[file_size] = '\0';
    fclose(file);
    
    // Create lexer and parser
    Lexer *lexer = lexer_create(source, (size_t)file_size, opts.input_file);
    if (!lexer) {
        fprintf(stderr, "Failed to create lexer\n");
        free(source);
        cli_destroy_config(config);
        return 1;
    }
    
    Parser *parser = parser_create(lexer);
    if (!parser) {
        fprintf(stderr, "Failed to create parser\n");
        lexer_destroy(lexer);
        free(source);
        cli_destroy_config(config);
        return 1;
    }
    
    // Parse the program
    ASTNode *ast_root = parser_parse_program(parser);
    if (!ast_root) {
        fprintf(stderr, "Failed to parse program\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        free(source);
        cli_destroy_config(config);
        return 1;
    }
    
    // Generate visualization
    printf("Generating AST visualization...\n");
    ToolResult viz_result = visualize_ast_tree(ast_root, &opts);
    
    if (viz_result.success) {
        printf("AST visualization generated successfully: %s\n", opts.output_file);
        printf("Statistics:\n");
        printf("  Nodes processed: %llu\n", (unsigned long long)atomic_load(&g_stats.nodes_processed));
        printf("  Edges generated: %llu\n", (unsigned long long)atomic_load(&g_stats.edges_generated));
        printf("  Bytes written: %llu\n", (unsigned long long)atomic_load(&g_stats.bytes_written));
        printf("  Maximum depth reached: %llu\n", (unsigned long long)atomic_load(&g_stats.max_depth_reached));
    } else {
        fprintf(stderr, "Failed to generate visualization: %s\n", viz_result.error_message);
    }
    
    // Cleanup
    ast_free_node(ast_root);
    parser_destroy(parser);
    lexer_destroy(lexer);
    free(source);
    cli_destroy_config(config);
    
    return viz_result.success ? 0 : 1;
}

static ToolResult setup_cli_options(CliConfig *config) {
    ToolResult result = {.success = true};
    
    // Output file option
    if (cli_add_option(config, "output", 'o', true, false, 
                       "Output file path (default: ast_output.dot)") != 0) {
        result.success = false;
        result.error_message = "Failed to add output option";
        return result;
    }
    
    // Format option
    if (cli_add_option(config, "format", 'f', true, false,
                       "Output format: dot, svg, html (default: dot)") != 0) {
        result.success = false;
        result.error_message = "Failed to add format option";
        return result;
    }
    
    // Type information option
    if (cli_add_option(config, "no-types", 't', false, false,
                       "Hide type information in visualization") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-types option";
        return result;
    }
    
    // Location information option
    if (cli_add_option(config, "locations", 'l', false, false,
                       "Show source location information") != 0) {
        result.success = false;
        result.error_message = "Failed to add locations option";
        return result;
    }
    
    // Compact view option
    if (cli_add_option(config, "compact", 'c', false, false,
                       "Generate compact visualization") != 0) {
        result.success = false;
        result.error_message = "Failed to add compact option";
        return result;
    }
    
    // Interactive option
    if (cli_add_option(config, "interactive", 'i', false, false,
                       "Generate interactive HTML output") != 0) {
        result.success = false;
        result.error_message = "Failed to add interactive option";
        return result;
    }
    
    // Max depth option
    if (cli_add_option(config, "max-depth", 'd', true, false,
                       "Maximum traversal depth (-1 for unlimited)") != 0) {
        result.success = false;
        result.error_message = "Failed to add max-depth option";
        return result;
    }
    
    // Filter pattern option
    if (cli_add_option(config, "filter", 'F', true, false,
                       "Filter nodes by pattern (node type or name)") != 0) {
        result.success = false;
        result.error_message = "Failed to add filter option";
        return result;
    }
    
    return result;
}

static ToolResult visualize_ast_tree(ASTNode *root, VisualizerOptions *opts) {
    ToolResult result = {.success = true};
    
    if (!root) {
        result.success = false;
        result.error_message = "AST root is NULL";
        return result;
    }
    
    // Open output file
    FILE *output = fopen(opts->output_file, "w");
    if (!output) {
        result.success = false;
        result.error_message = "Failed to open output file";
        return result;
    }
    
    // Generate output based on format
    if (strcmp(opts->format, "dot") == 0) {
        result = generate_dot_output(root, output, opts);
    } else {
        result.success = false;
        result.error_message = "Unsupported output format";
    }
    
    fclose(output);
    return result;
}

static ToolResult generate_dot_output(ASTNode *root, FILE *output, VisualizerOptions *opts) {
    ToolResult result = {.success = true};
    
    // Write DOT header
    fprintf(output, "digraph AST {\n");
    fprintf(output, "    node [shape=box, style=filled, fillcolor=lightblue];\n");
    fprintf(output, "    edge [color=blue];\n");
    fprintf(output, "    rankdir=TB;\n\n");
    
    // Traverse AST and generate nodes/edges
    result = traverse_ast_node(root, output, opts, 0);
    
    // Write DOT footer
    fprintf(output, "}\n");
    
    return result;
}

static ToolResult traverse_ast_node(ASTNode *node, FILE *output, VisualizerOptions *opts, int depth) {
    ToolResult result = {.success = true};
    
    if (!node) return result;
    
    // Check depth limit
    if (opts->max_depth >= 0 && depth > opts->max_depth) {
        return result;
    }
    
    // Check filter
    if (!should_include_node(node, opts)) {
        return result;
    }
    
    // Update statistics
    atomic_fetch_add(&opts->stats->nodes_processed, 1);
    if ((uint64_t)depth > atomic_load(&opts->stats->max_depth_reached)) {
        atomic_store(&opts->stats->max_depth_reached, depth);
    }
    
    // Generate node
    const char *label = get_node_label(node, opts);
    fprintf(output, "    node_%p [label=\"%s\"", (void*)node, label);
    write_node_attributes(node, output, opts);
    fprintf(output, "];\n");
    
    // Update bytes written statistic
    atomic_fetch_add(&opts->stats->bytes_written, strlen(label) + 50); // Approximate
    
    // Basic traversal - for a working implementation, we'll keep it simple
    // Note: Full AST traversal would require handling all union members
    // This provides basic functionality for demonstration
    
    return result;
}

static const char* get_node_type_name(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "Program";
        case AST_PACKAGE_DECL: return "Package";
        case AST_IMPORT_DECL: return "Import";
        case AST_FUNCTION_DECL: return "Function";
        case AST_STRUCT_DECL: return "Struct";
        case AST_ENUM_DECL: return "Enum";
        case AST_BLOCK: return "Block";
        case AST_BINARY_EXPR: return "BinaryExpr";
        case AST_IDENTIFIER: return "Identifier";
        case AST_INTEGER_LITERAL: return "IntLiteral";
        case AST_STRING_LITERAL: return "StringLiteral";
        default: return "Unknown";
    }
}

static const char* get_node_label(ASTNode *node, VisualizerOptions *opts) {
    static char label_buffer[256];
    const char *type_name = get_node_type_name(node->type);
    
    // Base label is the node type
    snprintf(label_buffer, sizeof(label_buffer), "%s", type_name);
    
    // Add type information if requested
    if (opts->show_types && node->type_info) {
        strncat(label_buffer, "\\n[typed]", sizeof(label_buffer) - strlen(label_buffer) - 1);
    }
    
    // Add location information if requested
    if (opts->show_locations) {
        char loc_info[64];
        snprintf(loc_info, sizeof(loc_info), "\\n@%d:%d", 
                 node->location.line, node->location.column);
        strncat(label_buffer, loc_info, sizeof(label_buffer) - strlen(label_buffer) - 1);
    }
    
    return label_buffer;
}

static bool should_include_node(ASTNode *node, VisualizerOptions *opts) {
    if (!opts->filter_pattern) return true;
    
    const char *type_name = get_node_type_name(node->type);
    
    // Simple pattern matching - can be enhanced
    return strstr(type_name, opts->filter_pattern) != NULL;
}

static void write_node_attributes(ASTNode *node, FILE *output, VisualizerOptions *opts) {
    // Different colors for different node types
    switch (node->type) {
        case AST_PROGRAM:
            fprintf(output, ", fillcolor=lightgreen");
            break;
        case AST_FUNCTION_DECL:
            fprintf(output, ", fillcolor=lightcoral");
            break;
        case AST_STRUCT_DECL:
            fprintf(output, ", fillcolor=lightyellow");
            break;
        case AST_BINARY_EXPR:
            fprintf(output, ", fillcolor=lightgray");
            break;
        case AST_IDENTIFIER:
            fprintf(output, ", fillcolor=lightpink");
            break;
        default:
            fprintf(output, ", fillcolor=lightblue");
            break;
    }
    
    // Add shape variations for compact view
    if (opts->compact_view) {
        fprintf(output, ", shape=ellipse");
    }
}

static void write_edge(ASTNode *parent, ASTNode *child, FILE *output, const char *label) {
    fprintf(output, "    node_%p -> node_%p [label=\"%s\"];\n", 
            (void*)parent, (void*)child, label);
    
    // Update edge statistics
    // Note: We need to access the global stats since this is a static function
    atomic_fetch_add(&g_stats.edges_generated, 1);
}
