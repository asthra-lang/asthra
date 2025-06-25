/**
 * Asthra Programming Language
 * Language Server Foundation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Foundation for IDE integration and developer tooling using Language Server Protocol (LSP)
 */

#include <errno.h>
#include <fcntl.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <time.h>
#include <unistd.h>

#include "../runtime/asthra_runtime.h"
#include "../src/analysis/semantic_analyzer_core.h"
#include "../src/parser/lexer.h"
#include "../src/parser/parser.h"
#include "common/cli_framework.h"
#include "common/error_framework.h"
#include "common/statistics_framework.h"

// C17 feature detection and static assertions
ASTHRA_STATIC_ASSERT(sizeof(int) >= 4, "int must be at least 32 bits for language server");
ASTHRA_STATIC_ASSERT(sizeof(size_t) >= sizeof(void *),
                     "size_t must be at least as large as pointer");

// C17 atomic counters for thread-safe statistics
typedef struct {
    _Atomic(uint64_t) requests_processed;
    _Atomic(uint64_t) documents_parsed;
    _Atomic(uint64_t) diagnostics_generated;
    _Atomic(uint64_t) completions_provided;
    _Atomic(uint64_t) hover_requests;
    _Atomic(uint64_t) definition_requests;
    _Atomic(uint64_t) errors_encountered;
} LanguageServerStatistics;

// LSP message types
typedef enum { LSP_REQUEST, LSP_RESPONSE, LSP_NOTIFICATION } LSPMessageType;

// LSP methods
typedef enum {
    LSP_INITIALIZE,
    LSP_INITIALIZED,
    LSP_SHUTDOWN,
    LSP_EXIT,
    LSP_TEXT_DOCUMENT_DID_OPEN,
    LSP_TEXT_DOCUMENT_DID_CHANGE,
    LSP_TEXT_DOCUMENT_DID_CLOSE,
    LSP_TEXT_DOCUMENT_COMPLETION,
    LSP_TEXT_DOCUMENT_HOVER,
    LSP_TEXT_DOCUMENT_DEFINITION,
    LSP_TEXT_DOCUMENT_DIAGNOSTIC,
    LSP_WORKSPACE_SYMBOL,
    LSP_UNKNOWN
} LSPMethod;

// Diagnostic severity levels
typedef enum {
    DIAGNOSTIC_ERROR = 1,
    DIAGNOSTIC_WARNING = 2,
    DIAGNOSTIC_INFORMATION = 3,
    DIAGNOSTIC_HINT = 4
} DiagnosticSeverity;

// Position in document
typedef struct {
    int line;
    int character;
} Position;

// Range in document
typedef struct {
    Position start;
    Position end;
} Range;

// Diagnostic information
typedef struct {
    Range range;
    DiagnosticSeverity severity;
    char *message;
    char *source;
    char *code;
} Diagnostic;

// Document text content
typedef struct {
    char *uri;
    char *content;
    size_t content_length;
    int version;
    Lexer *lexer;
    Parser *parser;
    ASTNode *ast_root;
    Diagnostic *diagnostics;
    int diagnostic_count;
    time_t last_modified;
} TextDocument;

// Completion item
typedef struct {
    char *label;
    char *detail;
    char *documentation;
    int kind; // CompletionItemKind
    char *insert_text;
} CompletionItem;

// Symbol information
typedef struct {
    char *name;
    int kind; // SymbolKind
    Range range;
    char *container_name;
} SymbolInfo;

// LSP message
typedef struct {
    LSPMessageType type;
    LSPMethod method;
    int id;
    char *content;
    size_t content_length;
    char *params;
} LSPMessage;

// Server configuration
typedef struct {
    bool enable_diagnostics;
    bool enable_completion;
    bool enable_hover;
    bool enable_definition;
    bool enable_symbol_search;
    bool real_time_validation;
    bool verbose_logging;
    int max_documents;
    int completion_limit;
    const char *log_file;
    LanguageServerStatistics *stats;
} ServerConfig;

// Simple result type for tools
typedef struct {
    bool success;
    const char *error_message;
} ToolResult;

// Language server context
typedef struct {
    TextDocument *documents;
    int document_count;
    bool server_running;
    bool initialized;
    FILE *log_file;
    ServerConfig config;
} LanguageServerContext;

// Forward declarations
static ToolResult run_language_server(ServerConfig *config);
static ToolResult process_lsp_message(const LSPMessage *message, LanguageServerContext *ctx);
static ToolResult handle_initialize(const LSPMessage *message, LanguageServerContext *ctx);
static ToolResult handle_text_document_did_open(const LSPMessage *message,
                                                LanguageServerContext *ctx);
static ToolResult handle_text_document_did_change(const LSPMessage *message,
                                                  LanguageServerContext *ctx);
static ToolResult handle_text_document_completion(const LSPMessage *message,
                                                  LanguageServerContext *ctx);
static ToolResult handle_text_document_hover(const LSPMessage *message, LanguageServerContext *ctx);
static ToolResult handle_text_document_definition(const LSPMessage *message,
                                                  LanguageServerContext *ctx);
static ToolResult handle_text_document_diagnostic(const LSPMessage *message,
                                                  LanguageServerContext *ctx);
static ToolResult parse_document(TextDocument *document, LanguageServerContext *ctx);
static ToolResult generate_diagnostics(TextDocument *document, LanguageServerContext *ctx);
static ToolResult provide_completions(const Position *position, TextDocument *document,
                                      CompletionItem **items, int *count);
static ToolResult provide_hover_info(const Position *position, TextDocument *document,
                                     char **hover_text);
static ToolResult find_definition(const Position *position, TextDocument *document,
                                  Position *definition_pos);
static LSPMessage *read_lsp_message(void);
static ToolResult send_lsp_response(int id, const char *result);
static ToolResult send_lsp_notification(const char *method, const char *params);
static char *create_json_response(int id, const char *result);
static char *create_json_notification(const char *method, const char *params);
static LSPMethod parse_lsp_method(const char *method_name);
static TextDocument *find_document(const char *uri, LanguageServerContext *ctx);
static ToolResult add_document(const char *uri, const char *content, LanguageServerContext *ctx);
static ToolResult update_document(const char *uri, const char *content, LanguageServerContext *ctx);
static ToolResult remove_document(const char *uri, LanguageServerContext *ctx);
static void log_message(LanguageServerContext *ctx, const char *level, const char *message);
static ToolResult setup_cli_options(CliConfig *config);

// Global statistics and server context
static LanguageServerStatistics g_stats = {0};
static LanguageServerContext g_server_ctx = {0};

int main(int argc, char **argv) {
    // C17 designated initializer for configuration
    ServerConfig config = {.enable_diagnostics = true,
                           .enable_completion = true,
                           .enable_hover = true,
                           .enable_definition = true,
                           .enable_symbol_search = true,
                           .real_time_validation = true,
                           .verbose_logging = false,
                           .max_documents = 100,
                           .completion_limit = 50,
                           .log_file = "language_server.log",
                           .stats = &g_stats};

    // Setup CLI configuration
    CliConfig *cli_config = cli_create_config(
        "Asthra Language Server", "[options]",
        "Foundation for IDE integration and developer tooling using Language Server Protocol");

    ToolResult setup_result = setup_cli_options(cli_config);
    if (!setup_result.success) {
        fprintf(stderr, "Failed to setup CLI options: %s\n", setup_result.error_message);
        cli_destroy_config(cli_config);
        return 1;
    }

    // Parse command line arguments
    CliOptionValue values[16];
    CliParseResult parse_result = cli_parse_args(cli_config, argc, argv, values, 16);

    if (parse_result.help_requested) {
        cli_print_help(cli_config);
        cli_destroy_config(cli_config);
        return 0;
    }

    if (parse_result.error_occurred) {
        cli_print_error(cli_config, parse_result.error_message);
        cli_destroy_config(cli_config);
        return 1;
    }

    // Process optional parameters
    config.enable_diagnostics = !cli_get_bool_option(values, 16, "no-diagnostics");
    config.enable_completion = !cli_get_bool_option(values, 16, "no-completion");
    config.enable_hover = !cli_get_bool_option(values, 16, "no-hover");
    config.enable_definition = !cli_get_bool_option(values, 16, "no-definition");
    config.enable_symbol_search = !cli_get_bool_option(values, 16, "no-symbols");
    config.real_time_validation = !cli_get_bool_option(values, 16, "no-realtime");
    config.verbose_logging = cli_get_bool_option(values, 16, "verbose");

    config.max_documents = cli_get_int_option(values, 16, "max-docs", 100);
    config.completion_limit = cli_get_int_option(values, 16, "completion-limit", 50);

    const char *log_file = cli_get_string_option(values, 16, "log-file");
    if (log_file)
        config.log_file = log_file;

    // Initialize server context
    g_server_ctx.documents = malloc(sizeof(TextDocument) * (size_t)config.max_documents);
    if (!g_server_ctx.documents) {
        fprintf(stderr, "Failed to allocate memory for document storage\n");
        cli_destroy_config(cli_config);
        return 1;
    }

    g_server_ctx.config = config;
    g_server_ctx.document_count = 0;
    g_server_ctx.server_running = false;
    g_server_ctx.initialized = false;

    // Open log file
    g_server_ctx.log_file = fopen(config.log_file, "w");
    if (!g_server_ctx.log_file) {
        fprintf(stderr, "Warning: Could not open log file %s\n", config.log_file);
    }

    // Run language server
    printf("Starting Asthra Language Server...\n");
    printf("Configuration:\n");
    printf("  Diagnostics: %s\n", config.enable_diagnostics ? "enabled" : "disabled");
    printf("  Completion: %s\n", config.enable_completion ? "enabled" : "disabled");
    printf("  Hover: %s\n", config.enable_hover ? "enabled" : "disabled");
    printf("  Definition: %s\n", config.enable_definition ? "enabled" : "disabled");
    printf("  Symbol search: %s\n", config.enable_symbol_search ? "enabled" : "disabled");
    printf("  Real-time validation: %s\n", config.real_time_validation ? "enabled" : "disabled");
    printf("  Max documents: %d\n", config.max_documents);
    printf("  Log file: %s\n", config.log_file);
    printf("\n");

    log_message(&g_server_ctx, "INFO", "Asthra Language Server starting");

    ToolResult server_result = run_language_server(&config);

    if (server_result.success) {
        printf("Language server completed successfully\n");
        printf("Statistics:\n");
        printf("  Requests processed: %llu\n",
               (unsigned long long)atomic_load(&g_stats.requests_processed));
        printf("  Documents parsed: %llu\n",
               (unsigned long long)atomic_load(&g_stats.documents_parsed));
        printf("  Diagnostics generated: %llu\n",
               (unsigned long long)atomic_load(&g_stats.diagnostics_generated));
        printf("  Completions provided: %llu\n",
               (unsigned long long)atomic_load(&g_stats.completions_provided));
        printf("  Hover requests: %llu\n",
               (unsigned long long)atomic_load(&g_stats.hover_requests));
        printf("  Definition requests: %llu\n",
               (unsigned long long)atomic_load(&g_stats.definition_requests));
        printf("  Errors encountered: %llu\n",
               (unsigned long long)atomic_load(&g_stats.errors_encountered));
    } else {
        fprintf(stderr, "Language server failed: %s\n", server_result.error_message);
    }

    // Cleanup
    if (g_server_ctx.log_file) {
        fclose(g_server_ctx.log_file);
    }
    free(g_server_ctx.documents);
    cli_destroy_config(cli_config);
    return server_result.success ? 0 : 1;
}

static ToolResult setup_cli_options(CliConfig *config) {
    ToolResult result = {.success = true};

    // Disable diagnostics option
    if (cli_add_option(config, "no-diagnostics", 'D', false, false,
                       "Disable diagnostic reporting") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-diagnostics option";
        return result;
    }

    // Disable completion option
    if (cli_add_option(config, "no-completion", 'C', false, false, "Disable code completion") !=
        0) {
        result.success = false;
        result.error_message = "Failed to add no-completion option";
        return result;
    }

    // Disable hover option
    if (cli_add_option(config, "no-hover", 'H', false, false, "Disable hover information") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-hover option";
        return result;
    }

    // Disable definition option
    if (cli_add_option(config, "no-definition", 'G', false, false, "Disable go-to-definition") !=
        0) {
        result.success = false;
        result.error_message = "Failed to add no-definition option";
        return result;
    }

    // Disable symbol search option
    if (cli_add_option(config, "no-symbols", 'S', false, false, "Disable symbol search") != 0) {
        result.success = false;
        result.error_message = "Failed to add no-symbols option";
        return result;
    }

    // Disable real-time validation option
    if (cli_add_option(config, "no-realtime", 'R', false, false, "Disable real-time validation") !=
        0) {
        result.success = false;
        result.error_message = "Failed to add no-realtime option";
        return result;
    }

    // Verbose logging option
    if (cli_add_option(config, "verbose", 'v', false, false, "Enable verbose logging") != 0) {
        result.success = false;
        result.error_message = "Failed to add verbose option";
        return result;
    }

    // Max documents option
    if (cli_add_option(config, "max-docs", 'm', true, false,
                       "Maximum number of documents to track (default: 100)") != 0) {
        result.success = false;
        result.error_message = "Failed to add max-docs option";
        return result;
    }

    // Completion limit option
    if (cli_add_option(config, "completion-limit", 'l', true, false,
                       "Maximum number of completion items (default: 50)") != 0) {
        result.success = false;
        result.error_message = "Failed to add completion-limit option";
        return result;
    }

    // Log file option
    if (cli_add_option(config, "log-file", 'L', true, false,
                       "Log file path (default: language_server.log)") != 0) {
        result.success = false;
        result.error_message = "Failed to add log-file option";
        return result;
    }

    return result;
}

static ToolResult run_language_server(ServerConfig *config) {
    ToolResult result = {.success = true};

    g_server_ctx.server_running = true;

    log_message(&g_server_ctx, "INFO", "Language server main loop starting");

    // Main message processing loop
    while (g_server_ctx.server_running) {
        // Read LSP message from stdin
        LSPMessage *message = read_lsp_message();
        if (!message) {
            if (errno == EAGAIN) {
                usleep(10000); // 10ms sleep if no message available
                continue;
            }
            break; // EOF or error
        }

        log_message(&g_server_ctx, "DEBUG", "Processing LSP message");

        // Process the message
        ToolResult process_result = process_lsp_message(message, &g_server_ctx);
        if (!process_result.success) {
            log_message(&g_server_ctx, "ERROR", process_result.error_message);
            atomic_fetch_add(&config->stats->errors_encountered, 1);
        }

        atomic_fetch_add(&config->stats->requests_processed, 1);

        // Cleanup message
        free(message->content);
        free(message->params);
        free(message);
    }

    log_message(&g_server_ctx, "INFO", "Language server shutting down");

    return result;
}

static ToolResult process_lsp_message(const LSPMessage *message, LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    switch (message->method) {
    case LSP_INITIALIZE:
        result = handle_initialize(message, ctx);
        break;

    case LSP_INITIALIZED:
        ctx->initialized = true;
        log_message(ctx, "INFO", "Client initialized");
        break;

    case LSP_SHUTDOWN:
        log_message(ctx, "INFO", "Shutdown requested");
        send_lsp_response(message->id, "null");
        break;

    case LSP_EXIT:
        log_message(ctx, "INFO", "Exit requested");
        ctx->server_running = false;
        break;

    case LSP_TEXT_DOCUMENT_DID_OPEN:
        result = handle_text_document_did_open(message, ctx);
        break;

    case LSP_TEXT_DOCUMENT_DID_CHANGE:
        result = handle_text_document_did_change(message, ctx);
        break;

    case LSP_TEXT_DOCUMENT_COMPLETION:
        result = handle_text_document_completion(message, ctx);
        break;

    case LSP_TEXT_DOCUMENT_HOVER:
        result = handle_text_document_hover(message, ctx);
        break;

    case LSP_TEXT_DOCUMENT_DEFINITION:
        result = handle_text_document_definition(message, ctx);
        break;

    case LSP_TEXT_DOCUMENT_DIAGNOSTIC:
        result = handle_text_document_diagnostic(message, ctx);
        break;

    default:
        log_message(ctx, "WARN", "Unknown LSP method");
        result.success = false;
        result.error_message = "Unknown LSP method";
        break;
    }

    return result;
}

static ToolResult handle_initialize(const LSPMessage *message, LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    // Create initialization response
    const char *capabilities =
        "{"
        "\"textDocumentSync\": 1,"
        "\"completionProvider\": {\"triggerCharacters\": [\".\", \":\", \"(\", \" \"]},"
        "\"hoverProvider\": true,"
        "\"definitionProvider\": true,"
        "\"diagnosticProvider\": true"
        "}";

    char response[1024];
    snprintf(response, sizeof(response), "{\"capabilities\": %s}", capabilities);

    send_lsp_response(message->id, response);

    log_message(ctx, "INFO", "Initialization response sent");

    return result;
}

static ToolResult handle_text_document_did_open(const LSPMessage *message,
                                                LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    // Extract URI and content from params (simplified)
    // In a full implementation, this would parse JSON properly
    const char *uri = "file://example.asthra";
    const char *content = "fn main() {}";

    result = add_document(uri, content, ctx);
    if (result.success) {
        log_message(ctx, "INFO", "Document opened");
        atomic_fetch_add(&ctx->config.stats->documents_parsed, 1);

        // Generate initial diagnostics if enabled
        if (ctx->config.enable_diagnostics && ctx->config.real_time_validation) {
            TextDocument *doc = find_document(uri, ctx);
            if (doc) {
                generate_diagnostics(doc, ctx);
            }
        }
    }

    return result;
}

static ToolResult handle_text_document_did_change(const LSPMessage *message,
                                                  LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    // Extract URI and content from params (simplified)
    const char *uri = "file://example.asthra";
    const char *content = "fn main() { let x = 42; }";

    result = update_document(uri, content, ctx);
    if (result.success) {
        log_message(ctx, "DEBUG", "Document updated");

        // Re-generate diagnostics if real-time validation is enabled
        if (ctx->config.enable_diagnostics && ctx->config.real_time_validation) {
            TextDocument *doc = find_document(uri, ctx);
            if (doc) {
                generate_diagnostics(doc, ctx);
            }
        }
    }

    return result;
}

static ToolResult handle_text_document_completion(const LSPMessage *message,
                                                  LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    if (!ctx->config.enable_completion) {
        result.success = false;
        result.error_message = "Completion disabled";
        return result;
    }

    // Extract position and document URI (simplified)
    Position position = {.line = 0, .character = 10};
    const char *uri = "file://example.asthra";

    TextDocument *doc = find_document(uri, ctx);
    if (!doc) {
        result.success = false;
        result.error_message = "Document not found";
        return result;
    }

    CompletionItem *items = NULL;
    int item_count = 0;

    result = provide_completions(&position, doc, &items, &item_count);
    if (result.success) {
        // Create completion response (simplified)
        char response[2048] = "{\"items\": [";

        for (int i = 0; i < item_count && i < ctx->config.completion_limit; i++) {
            char item_json[256];
            snprintf(item_json, sizeof(item_json), "{\"label\": \"%s\", \"kind\": %d}",
                     items[i].label, items[i].kind);

            if (i > 0)
                strcat(response, ", ");
            strcat(response, item_json);
        }
        strcat(response, "]}");

        send_lsp_response(message->id, response);
        atomic_fetch_add(&ctx->config.stats->completions_provided, 1);

        // Cleanup
        free(items);
    }

    return result;
}

static ToolResult handle_text_document_hover(const LSPMessage *message,
                                             LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    if (!ctx->config.enable_hover) {
        result.success = false;
        result.error_message = "Hover disabled";
        return result;
    }

    // Extract position and document URI (simplified)
    Position position = {.line = 0, .character = 5};
    const char *uri = "file://example.asthra";

    TextDocument *doc = find_document(uri, ctx);
    if (!doc) {
        result.success = false;
        result.error_message = "Document not found";
        return result;
    }

    char *hover_text = NULL;
    result = provide_hover_info(&position, doc, &hover_text);

    if (result.success && hover_text) {
        char response[1024];
        snprintf(response, sizeof(response),
                 "{\"contents\": {\"kind\": \"markdown\", \"value\": \"%s\"}}", hover_text);

        send_lsp_response(message->id, response);
        atomic_fetch_add(&ctx->config.stats->hover_requests, 1);

        free(hover_text);
    } else {
        send_lsp_response(message->id, "null");
    }

    return result;
}

static ToolResult handle_text_document_definition(const LSPMessage *message,
                                                  LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    if (!ctx->config.enable_definition) {
        result.success = false;
        result.error_message = "Definition lookup disabled";
        return result;
    }

    // Extract position and document URI (simplified)
    Position position = {.line = 0, .character = 5};
    const char *uri = "file://example.asthra";

    TextDocument *doc = find_document(uri, ctx);
    if (!doc) {
        result.success = false;
        result.error_message = "Document not found";
        return result;
    }

    Position definition_pos;
    result = find_definition(&position, doc, &definition_pos);

    if (result.success) {
        char response[512];
        snprintf(response, sizeof(response),
                 "{\"uri\": \"%s\", \"range\": {\"start\": {\"line\": %d, \"character\": %d}, "
                 "\"end\": {\"line\": %d, \"character\": %d}}}",
                 uri, definition_pos.line, definition_pos.character, definition_pos.line,
                 definition_pos.character + 10);

        send_lsp_response(message->id, response);
        atomic_fetch_add(&ctx->config.stats->definition_requests, 1);
    } else {
        send_lsp_response(message->id, "null");
    }

    return result;
}

static ToolResult handle_text_document_diagnostic(const LSPMessage *message,
                                                  LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    if (!ctx->config.enable_diagnostics) {
        result.success = false;
        result.error_message = "Diagnostics disabled";
        return result;
    }

    // Extract document URI (simplified)
    const char *uri = "file://example.asthra";

    TextDocument *doc = find_document(uri, ctx);
    if (!doc) {
        result.success = false;
        result.error_message = "Document not found";
        return result;
    }

    result = generate_diagnostics(doc, ctx);
    if (result.success) {
        atomic_fetch_add(&ctx->config.stats->diagnostics_generated, 1);
    }

    return result;
}

static ToolResult parse_document(TextDocument *document, LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    // Clean up existing parser state
    if (document->ast_root) {
        ast_free_node(document->ast_root);
        document->ast_root = NULL;
    }
    if (document->parser) {
        parser_destroy(document->parser);
        document->parser = NULL;
    }
    if (document->lexer) {
        lexer_destroy(document->lexer);
        document->lexer = NULL;
    }

    // Create new lexer and parser
    document->lexer = lexer_create(document->content, document->content_length, document->uri);
    if (!document->lexer) {
        result.success = false;
        result.error_message = "Failed to create lexer";
        return result;
    }

    document->parser = parser_create(document->lexer);
    if (!document->parser) {
        result.success = false;
        result.error_message = "Failed to create parser";
        return result;
    }

    // Parse the document
    document->ast_root = parser_parse_program(document->parser);
    document->last_modified = time(NULL);

    return result;
}

static ToolResult generate_diagnostics(TextDocument *document, LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    // Parse document first
    result = parse_document(document, ctx);
    if (!result.success) {
        return result;
    }

    // Clear existing diagnostics
    free(document->diagnostics);
    document->diagnostics = NULL;
    document->diagnostic_count = 0;

    // If parsing failed, create a diagnostic
    if (!document->ast_root) {
        document->diagnostics = malloc(sizeof(Diagnostic));
        if (document->diagnostics) {
            document->diagnostic_count = 1;
            document->diagnostics[0] = (Diagnostic){.range = {{0, 0}, {0, 10}},
                                                    .severity = DIAGNOSTIC_ERROR,
                                                    .message = strdup("Parse error"),
                                                    .source = strdup("asthra-ls"),
                                                    .code = strdup("parse_error")};
        }
    }

    // Send diagnostics notification
    if (document->diagnostic_count > 0) {
        char notification[1024];
        snprintf(notification, sizeof(notification),
                 "{\"uri\": \"%s\", \"diagnostics\": [{\"range\": {\"start\": {\"line\": 0, "
                 "\"character\": 0}, \"end\": {\"line\": 0, \"character\": 10}}, \"severity\": 1, "
                 "\"message\": \"Parse error\"}]}",
                 document->uri);

        send_lsp_notification("textDocument/publishDiagnostics", notification);
    }

    return result;
}

static ToolResult provide_completions(const Position *position, TextDocument *document,
                                      CompletionItem **items, int *count) {
    ToolResult result = {.success = true};

    // Simplified completion - provide basic language keywords
    const char *keywords[] = {"fn",     "let",  "if",   "else",   "while", "for",     "match",
                              "struct", "enum", "impl", "return", "break", "continue"};
    int keyword_count = sizeof(keywords) / sizeof(keywords[0]);

    *items = malloc(sizeof(CompletionItem) * (size_t)keyword_count);
    if (!*items) {
        result.success = false;
        result.error_message = "Failed to allocate completion items";
        return result;
    }

    *count = keyword_count;
    for (int i = 0; i < keyword_count; i++) {
        (*items)[i] = (CompletionItem){.label = strdup(keywords[i]),
                                       .detail = strdup("Keyword"),
                                       .documentation = strdup("Asthra language keyword"),
                                       .kind = 14, // CompletionItemKind.Keyword
                                       .insert_text = strdup(keywords[i])};
    }

    return result;
}

static ToolResult provide_hover_info(const Position *position, TextDocument *document,
                                     char **hover_text) {
    ToolResult result = {.success = true};

    // Simplified hover - provide basic information
    *hover_text = strdup("Asthra language element");

    return result;
}

static ToolResult find_definition(const Position *position, TextDocument *document,
                                  Position *definition_pos) {
    ToolResult result = {.success = true};

    // Simplified definition finding - return same position
    *definition_pos = *position;

    return result;
}

static LSPMessage *read_lsp_message(void) {
    // Simplified LSP message reading
    // In a full implementation, this would read the Content-Length header
    // and then read the exact number of bytes for the JSON-RPC message

    char buffer[4096];
    if (!fgets(buffer, sizeof(buffer), stdin)) {
        return NULL;
    }

    LSPMessage *message = malloc(sizeof(LSPMessage));
    if (!message)
        return NULL;

    message->type = LSP_REQUEST;
    message->method = LSP_INITIALIZE; // Simplified
    message->id = 1;
    message->content = strdup(buffer);
    message->content_length = strlen(buffer);
    message->params = strdup("{}");

    return message;
}

static ToolResult send_lsp_response(int id, const char *result) {
    ToolResult tool_result = {.success = true};

    char *json_response = create_json_response(id, result);
    if (!json_response) {
        tool_result.success = false;
        tool_result.error_message = "Failed to create JSON response";
        return tool_result;
    }

    printf("Content-Length: %zu\r\n\r\n%s", strlen(json_response), json_response);
    fflush(stdout);

    free(json_response);
    return tool_result;
}

static ToolResult send_lsp_notification(const char *method, const char *params) {
    ToolResult result = {.success = true};

    char *json_notification = create_json_notification(method, params);
    if (!json_notification) {
        result.success = false;
        result.error_message = "Failed to create JSON notification";
        return result;
    }

    printf("Content-Length: %zu\r\n\r\n%s", strlen(json_notification), json_notification);
    fflush(stdout);

    free(json_notification);
    return result;
}

static char *create_json_response(int id, const char *result) {
    char *json = malloc(512);
    if (!json)
        return NULL;

    snprintf(json, 512, "{\"jsonrpc\": \"2.0\", \"id\": %d, \"result\": %s}", id, result);
    return json;
}

static char *create_json_notification(const char *method, const char *params) {
    char *json = malloc(1024);
    if (!json)
        return NULL;

    snprintf(json, 1024, "{\"jsonrpc\": \"2.0\", \"method\": \"%s\", \"params\": %s}", method,
             params);
    return json;
}

static LSPMethod parse_lsp_method(const char *method_name) {
    if (strcmp(method_name, "initialize") == 0)
        return LSP_INITIALIZE;
    if (strcmp(method_name, "initialized") == 0)
        return LSP_INITIALIZED;
    if (strcmp(method_name, "shutdown") == 0)
        return LSP_SHUTDOWN;
    if (strcmp(method_name, "exit") == 0)
        return LSP_EXIT;
    if (strcmp(method_name, "textDocument/didOpen") == 0)
        return LSP_TEXT_DOCUMENT_DID_OPEN;
    if (strcmp(method_name, "textDocument/didChange") == 0)
        return LSP_TEXT_DOCUMENT_DID_CHANGE;
    if (strcmp(method_name, "textDocument/completion") == 0)
        return LSP_TEXT_DOCUMENT_COMPLETION;
    if (strcmp(method_name, "textDocument/hover") == 0)
        return LSP_TEXT_DOCUMENT_HOVER;
    if (strcmp(method_name, "textDocument/definition") == 0)
        return LSP_TEXT_DOCUMENT_DEFINITION;
    return LSP_UNKNOWN;
}

static TextDocument *find_document(const char *uri, LanguageServerContext *ctx) {
    for (int i = 0; i < ctx->document_count; i++) {
        if (strcmp(ctx->documents[i].uri, uri) == 0) {
            return &ctx->documents[i];
        }
    }
    return NULL;
}

static ToolResult add_document(const char *uri, const char *content, LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    if (ctx->document_count >= ctx->config.max_documents) {
        result.success = false;
        result.error_message = "Maximum documents reached";
        return result;
    }

    TextDocument *doc = &ctx->documents[ctx->document_count];
    doc->uri = strdup(uri);
    doc->content = strdup(content);
    doc->content_length = strlen(content);
    doc->version = 1;
    doc->lexer = NULL;
    doc->parser = NULL;
    doc->ast_root = NULL;
    doc->diagnostics = NULL;
    doc->diagnostic_count = 0;
    doc->last_modified = time(NULL);

    ctx->document_count++;

    return result;
}

static ToolResult update_document(const char *uri, const char *content,
                                  LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    TextDocument *doc = find_document(uri, ctx);
    if (!doc) {
        result.success = false;
        result.error_message = "Document not found";
        return result;
    }

    free(doc->content);
    doc->content = strdup(content);
    doc->content_length = strlen(content);
    doc->version++;
    doc->last_modified = time(NULL);

    return result;
}

static ToolResult remove_document(const char *uri, LanguageServerContext *ctx) {
    ToolResult result = {.success = true};

    for (int i = 0; i < ctx->document_count; i++) {
        if (strcmp(ctx->documents[i].uri, uri) == 0) {
            TextDocument *doc = &ctx->documents[i];

            // Cleanup document resources
            free(doc->uri);
            free(doc->content);
            if (doc->ast_root)
                ast_free_node(doc->ast_root);
            if (doc->parser)
                parser_destroy(doc->parser);
            if (doc->lexer)
                lexer_destroy(doc->lexer);
            free(doc->diagnostics);

            // Shift remaining documents
            for (int j = i; j < ctx->document_count - 1; j++) {
                ctx->documents[j] = ctx->documents[j + 1];
            }
            ctx->document_count--;

            return result;
        }
    }

    result.success = false;
    result.error_message = "Document not found";
    return result;
}

static void log_message(LanguageServerContext *ctx, const char *level, const char *message) {
    if (ctx->log_file) {
        time_t now = time(NULL);
        char timestamp[64];
        strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", localtime(&now));

        fprintf(ctx->log_file, "[%s] %s: %s\n", timestamp, level, message);
        fflush(ctx->log_file);
    }

    if (ctx->config.verbose_logging) {
        printf("[%s] %s\n", level, message);
    }
}