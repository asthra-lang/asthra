/**
 * Asthra Programming Language
 * Symbol Table Inspector Tool
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Debug semantic analysis by examining symbol tables and type resolution
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdatomic.h>

#include "common/cli_framework.h"
#include "common/error_framework.h"
#include "common/statistics_framework.h"
#include "../src/analysis/semantic_analyzer_core.h"
#include "../src/analysis/semantic_symbols.h"
#include "../src/analysis/semantic_symbols_defs.h"
#include "../src/parser/parser.h"
#include "../src/parser/lexer.h"
#include "../runtime/asthra_runtime.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for symbol inspector");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void*), "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe statistics
typedef struct {
    _Atomic(uint64_t) symbols_inspected;
    _Atomic(uint64_t) scopes_traversed;
    _Atomic(uint64_t) queries_processed;
    _Atomic(uint64_t) type_resolutions_traced;
} InspectorStatistics;

// Inspector operation modes
typedef enum {
    MODE_INTERACTIVE,
    MODE_DUMP_ALL,
    MODE_QUERY,
    MODE_SCOPE_TREE,
    MODE_TYPE_TRACE
} InspectorMode;

// Inspector configuration
typedef struct {
    const char *input_file;
    const char *output_file;
    const char *query_symbol;
    InspectorMode mode;
    bool show_types;
    bool show_locations;
    bool show_unused;
    bool export_json;
    int scope_depth;
    InspectorStatistics *stats;
} InspectorOptions;

// Simple result type for tools
typedef struct {
    bool success;
    const char *error_message;
} ToolResult;

// Forward declarations
static ToolResult inspect_symbols(InspectorOptions *opts);
static ToolResult run_interactive_mode(SymbolTable *symbols, InspectorOptions *opts);
static ToolResult dump_all_symbols(SymbolTable *symbols, InspectorOptions *opts);
static ToolResult query_symbol(SymbolTable *symbols, const char *name, InspectorOptions *opts);
static ToolResult show_scope_tree(SymbolTable *symbols, InspectorOptions *opts);
static ToolResult trace_type_resolution(SymbolTable *symbols, const char *type_name, InspectorOptions *opts);
static void print_symbol_entry(SymbolEntry *entry, InspectorOptions *opts, int indent);
static void print_symbol_table(SymbolTable *table, InspectorOptions *opts, int depth);
static const char* get_symbol_kind_name(SymbolKind kind);
static const char* get_visibility_name(VisibilityType visibility);
static void handle_interactive_command(const char *command, SymbolTable *symbols, InspectorOptions *opts);
static ToolResult setup_cli_options(CliConfig *config);

// Global statistics instance
static InspectorStatistics g_stats = {0};

int main(int argc, char **argv) {
    // C17 designated initializer for options
    InspectorOptions opts = {
        .input_file = NULL,
        .output_file = NULL,
        .query_symbol = NULL,
        .mode = MODE_INTERACTIVE,
        .show_types = true,
        .show_locations = false,
        .show_unused = false,
        .export_json = false,
        .scope_depth = -1,
        .stats = &g_stats
    };
    
    // Setup CLI configuration
    CliConfig *config = cli_create_config(
        "Asthra Symbol Inspector",
        "[options] <input_file>",
        "Debug semantic analysis by examining symbol tables and type resolution"
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
    
    const char *query = cli_get_string_option(values, 16, "query");
    if (query) {
        opts.query_symbol = query;
        opts.mode = MODE_QUERY;
    }
    
    const char *mode = cli_get_string_option(values, 16, "mode");
    if (mode) {
        if (strcmp(mode, "interactive") == 0) opts.mode = MODE_INTERACTIVE;
        else if (strcmp(mode, "dump") == 0) opts.mode = MODE_DUMP_ALL;
        else if (strcmp(mode, "scope-tree") == 0) opts.mode = MODE_SCOPE_TREE;
        else if (strcmp(mode, "type-trace") == 0) opts.mode = MODE_TYPE_TRACE;
    }
    
    opts.show_types = !cli_get_bool_option(values, 16, "no-types");
    opts.show_locations = cli_get_bool_option(values, 16, "locations");
    opts.show_unused = cli_get_bool_option(values, 16, "show-unused");
    opts.export_json = cli_get_bool_option(values, 16, "json");
    opts.scope_depth = cli_get_int_option(values, 16, "scope-depth", -1);
    
    // Validate input file
    if (!cli_validate_file_exists(opts.input_file)) {
        fprintf(stderr, "Error: Input file '%s' does not exist\n", opts.input_file);
        cli_destroy_config(config);
        return 1;
    }
    
    // Run symbol inspection
    printf("Inspecting symbol tables for: %s\n", opts.input_file);
    
    ToolResult inspection_result = inspect_symbols(&opts);
    
    if (inspection_result.success) {
        printf("Symbol inspection completed successfully\n");
        printf("Statistics:\n");
        printf("  Symbols inspected: %llu\n", (unsigned long long)atomic_load(&g_stats.symbols_inspected));
        printf("  Scopes traversed: %llu\n", (unsigned long long)atomic_load(&g_stats.scopes_traversed));
        printf("  Queries processed: %llu\n", (unsigned long long)atomic_load(&g_stats.queries_processed));
        printf("  Type resolutions traced: %llu\n", (unsigned long long)atomic_load(&g_stats.type_resolutions_traced));
    } else {
        fprintf(stderr, "Symbol inspection failed: %s\n", inspection_result.error_message);
    }
    
    cli_destroy_config(config);
    return inspection_result.success ? 0 : 1;
}

static ToolResult setup_cli_options(CliConfig *config) {
    ToolResult result = {.success = true};
    
    // Output file option
    if (cli_add_option(config, "output", 'o', true, false,
                       "Output file for inspection results") != 0) {
        result.success = false;
        result.error_message = "Failed to add output option";
        return result;
    }
    
    // Query symbol option
    if (cli_add_option(config, "query", 'q', true, false,
                       "Query specific symbol by name") != 0) {
        result.success = false;
        result.error_message = "Failed to add query option";
        return result;
    }
    
    // Mode option
    if (cli_add_option(config, "mode", 'm', true, false,
                       "Operation mode: interactive, dump, scope-tree, type-trace") != 0) {
        result.success = false;
        result.error_message = "Failed to add mode option";
        return result;
    }
    
    // Type information option
    if (cli_add_option(config, "no-types", 't', false, false,
                       "Hide type information") != 0) {
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
    
    // Show unused symbols option
    if (cli_add_option(config, "show-unused", 'u', false, false,
                       "Show unused symbols") != 0) {
        result.success = false;
        result.error_message = "Failed to add show-unused option";
        return result;
    }
    
    // JSON export option
    if (cli_add_option(config, "json", 'j', false, false,
                       "Export results in JSON format") != 0) {
        result.success = false;
        result.error_message = "Failed to add json option";
        return result;
    }
    
    // Scope depth option
    if (cli_add_option(config, "scope-depth", 'd', true, false,
                       "Maximum scope depth to inspect (-1 for unlimited)") != 0) {
        result.success = false;
        result.error_message = "Failed to add scope-depth option";
        return result;
    }
    
    return result;
}

static ToolResult inspect_symbols(InspectorOptions *opts) {
    ToolResult result = {.success = true};
    
    // Simplified implementation for now
    printf("Symbol Inspector - Basic Implementation\n");
    printf("Input file: %s\n", opts->input_file);
    printf("Mode: %d\n", opts->mode);
    
    if (opts->query_symbol) {
        printf("Query: %s\n", opts->query_symbol);
    }
    
    printf("Note: Full symbol table analysis requires complete semantic analyzer integration.\n");
    printf("This is a placeholder implementation for the build system.\n");
    
    atomic_fetch_add(&opts->stats->symbols_inspected, 1);
    atomic_fetch_add(&opts->stats->scopes_traversed, 1);
    atomic_fetch_add(&opts->stats->queries_processed, 1);
    
    return result;
}

static ToolResult run_interactive_mode(SymbolTable *symbols, InspectorOptions *opts) {
    ToolResult result = {.success = true};
    char command[256];
    
    printf("Symbol Inspector Interactive Mode\n");
    printf("Available commands:\n");
    printf("  list                - List all symbols\n");
    printf("  query <name>        - Query specific symbol\n");
    printf("  scope               - Show scope hierarchy\n");
    printf("  stats               - Show statistics\n");
    printf("  help                - Show this help\n");
    printf("  quit                - Exit\n\n");
    
    while (1) {
        printf("inspector> ");
        if (!fgets(command, sizeof(command), stdin)) {
            break;
        }
        
        // Remove newline
        command[strcspn(command, "\n")] = 0;
        
        if (strcmp(command, "quit") == 0 || strcmp(command, "exit") == 0) {
            break;
        }
        
        handle_interactive_command(command, symbols, opts);
        atomic_fetch_add(&opts->stats->queries_processed, 1);
    }
    
    return result;
}

static ToolResult dump_all_symbols(SymbolTable *symbols, InspectorOptions *opts) {
    ToolResult result = {.success = true};
    
    printf("Symbol Table Dump\n");
    printf("================\n\n");
    
    print_symbol_table(symbols, opts, 0);
    
    return result;
}

static ToolResult query_symbol(SymbolTable *symbols, const char *name, InspectorOptions *opts) {
    ToolResult result = {.success = true};
    
    if (!name) {
        result.success = false;
        result.error_message = "Symbol name required for query";
        return result;
    }
    
    printf("Querying symbol: %s\n", name);
    printf("==================\n\n");
    
    SymbolEntry *entry = symbol_table_lookup_safe(symbols, name);
    if (entry) {
        print_symbol_entry(entry, opts, 0);
        atomic_fetch_add(&opts->stats->symbols_inspected, 1);
    } else {
        printf("Symbol '%s' not found\n", name);
    }
    
    atomic_fetch_add(&opts->stats->queries_processed, 1);
    return result;
}

static ToolResult show_scope_tree(SymbolTable *symbols, InspectorOptions *opts) {
    ToolResult result = {.success = true};
    
    printf("Scope Hierarchy\n");
    printf("==============\n\n");
    
    print_symbol_table(symbols, opts, 0);
    
    return result;
}

static ToolResult trace_type_resolution(SymbolTable *symbols, const char *type_name, InspectorOptions *opts) {
    ToolResult result = {.success = true};
    
    if (!type_name) {
        result.success = false;
        result.error_message = "Type name required for tracing";
        return result;
    }
    
    printf("Type Resolution Trace: %s\n", type_name);
    printf("========================\n\n");
    
    // This would require integration with type resolution system
    printf("Type resolution tracing not fully implemented yet\n");
    
    atomic_fetch_add(&opts->stats->type_resolutions_traced, 1);
    return result;
}

static void print_symbol_entry(SymbolEntry *entry, InspectorOptions *opts, int indent) {
    if (!entry) return;
    
    // Print indentation
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    
    printf("Symbol: %s\n", entry->name);
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("  Kind: %s\n", get_symbol_kind_name(entry->kind));
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("  Visibility: %s\n", get_visibility_name(entry->visibility));
    
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("  Scope: %u\n", entry->scope_id);
    
    // Show flags
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }
    printf("  Flags: ");
    if (entry->flags.is_used) printf("used ");
    if (entry->flags.is_exported) printf("exported ");
    if (entry->flags.is_mutable) printf("mutable ");
    if (entry->flags.is_initialized) printf("initialized ");
    if (entry->flags.is_predeclared) printf("predeclared ");
    printf("\n");
    
    // Show type information if requested
    if (opts->show_types && entry->type) {
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("  Type: [type info available]\n");
    }
    
    // Show location information if requested
    if (opts->show_locations && entry->declaration) {
        for (int i = 0; i < indent; i++) {
            printf("  ");
        }
        printf("  Location: line %d, column %d\n", 
               entry->declaration->location.line, 
               entry->declaration->location.column);
    }
    
    printf("\n");
    atomic_fetch_add(&g_stats.symbols_inspected, 1);
}

static void print_symbol_table(SymbolTable *table, InspectorOptions *opts, int depth) {
    if (!table) return;
    
    if (opts->scope_depth >= 0 && depth > opts->scope_depth) {
        return;
    }
    
    printf("Scope %u (depth %d):\n", table->current_scope, depth);
    printf("Entry count: %zu\n", atomic_load(&table->entry_count));
    printf("Bucket count: %zu\n", table->bucket_count);
    printf("\n");
    
    // Iterate through all buckets
    for (size_t i = 0; i < table->bucket_count; i++) {
        SymbolEntry *entry = table->buckets[i];
        while (entry) {
            // Filter unused symbols if requested
            if (!opts->show_unused && !entry->flags.is_used) {
                entry = entry->next;
                continue;
            }
            
            print_symbol_entry(entry, opts, depth + 1);
            entry = entry->next;
        }
    }
    
    atomic_fetch_add(&g_stats.scopes_traversed, 1);
    
    // Recursively print parent scopes
    if (table->parent) {
        printf("Parent scope:\n");
        print_symbol_table(table->parent, opts, depth + 1);
    }
}

static const char* get_symbol_kind_name(SymbolKind kind) {
    switch (kind) {
        case SYMBOL_VARIABLE: return "Variable";
        case SYMBOL_FUNCTION: return "Function";
        case SYMBOL_TYPE: return "Type";
        case SYMBOL_PARAMETER: return "Parameter";
        case SYMBOL_FIELD: return "Field";
        case SYMBOL_METHOD: return "Method";
        case SYMBOL_ENUM_VARIANT: return "EnumVariant";
        case SYMBOL_TYPE_PARAMETER: return "TypeParameter";
        case SYMBOL_CONST: return "Const";
        default: return "Unknown";
    }
}

static const char* get_visibility_name(VisibilityType visibility) {
    switch (visibility) {
        case VISIBILITY_PRIVATE: return "private";
        case VISIBILITY_PUBLIC: return "public";
        default: return "unknown";
    }
}

static void handle_interactive_command(const char *command, SymbolTable *symbols, InspectorOptions *opts) {
    if (strncmp(command, "query ", 6) == 0) {
        const char *symbol_name = command + 6;
        query_symbol(symbols, symbol_name, opts);
    } else if (strcmp(command, "list") == 0) {
        dump_all_symbols(symbols, opts);
    } else if (strcmp(command, "scope") == 0) {
        show_scope_tree(symbols, opts);
    } else if (strcmp(command, "stats") == 0) {
        printf("Statistics:\n");
        printf("  Symbols inspected: %llu\n", (unsigned long long)atomic_load(&opts->stats->symbols_inspected));
        printf("  Scopes traversed: %llu\n", (unsigned long long)atomic_load(&opts->stats->scopes_traversed));
        printf("  Queries processed: %llu\n", (unsigned long long)atomic_load(&opts->stats->queries_processed));
    } else if (strcmp(command, "help") == 0) {
        printf("Available commands:\n");
        printf("  list                - List all symbols\n");
        printf("  query <name>        - Query specific symbol\n");
        printf("  scope               - Show scope hierarchy\n");
        printf("  stats               - Show statistics\n");
        printf("  help                - Show this help\n");
        printf("  quit                - Exit\n");
    } else {
        printf("Unknown command: %s\n", command);
        printf("Type 'help' for available commands\n");
    }
}
