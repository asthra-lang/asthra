/**
 * Asthra Programming Language Compiler
 * Core parser functionality - creation, destruction, and basic utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_annotations.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include "grammar_statements.h"
#include "grammar_toplevel.h"
#include "parser.h"
#include "parser_ast_helpers.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Simple symbol table for parser-time symbol tracking
typedef struct SymbolEntry {
    char *name;
    ASTNode *declaration;
    struct SymbolEntry *next;
} SymbolEntry;

struct SymbolTable {
    SymbolEntry *entries;
    struct SymbolTable *parent;
};

// Parse context stack for disambiguation
typedef struct ParseContextFrame {
    ParseContext context;
    struct ParseContextFrame *next;
} ParseContextFrame;

// Extended parser structure with context stack
typedef struct {
    Parser base;
    ParseContextFrame *context_stack;
    SymbolTable *current_scope;
} ExtendedParser;

// =============================================================================
// PARSER CREATION AND MANAGEMENT
// =============================================================================

Parser *parser_create(Lexer *lexer) {
    if (!lexer)
        return NULL;

    ExtendedParser *ext_parser = malloc(sizeof(ExtendedParser));
    if (!ext_parser)
        return NULL;

    Parser *parser = &ext_parser->base;
    memset(parser, 0, sizeof(Parser));

    parser->lexer = lexer;
    parser->errors = NULL;
    parser->last_error = NULL;
    parser->error_count = 0;
    parser->panic_mode = false;
    parser->had_error = false;

    // Initialize config with default values
    parser->config.allow_incomplete_parse = false;
    parser->config.strict_mode = true;
    parser->config.enable_recovery = true;
    parser->config.collect_statistics = true;
    parser->config.max_errors = 100;
    parser->config.max_recursion_depth = 1000;
    parser->config.initial_token_buffer_size = 256;

    // Initialize extended fields
    ext_parser->context_stack = NULL;
    ext_parser->current_scope = malloc(sizeof(SymbolTable));
    if (ext_parser->current_scope) {
        ext_parser->current_scope->entries = NULL;
        ext_parser->current_scope->parent = NULL;
    }

    parser->symbol_table = ext_parser->current_scope;

    // Get first token
    parser->current_token = lexer_next_token(lexer);

    // Skip any initial newline tokens
    while (parser->current_token.type == TOKEN_NEWLINE && parser->current_token.type != TOKEN_EOF) {
        parser->current_token = lexer_next_token(lexer);
    }

    return parser;
}

void parser_destroy(Parser *parser) {
    if (!parser)
        return;

    ExtendedParser *ext_parser = (ExtendedParser *)parser;

    // Free the current token to prevent memory leaks
    token_free(&parser->current_token);

    // Free errors
    ParseError *error = parser->errors;
    while (error) {
        ParseError *next = error->next;
        if (error->message) {
            free(error->message);
        }
        free(error);
        error = next;
    }

    // Free context stack
    ParseContextFrame *frame = ext_parser->context_stack;
    while (frame) {
        ParseContextFrame *next = frame->next;
        free(frame);
        frame = next;
    }

    // Free symbol table
    SymbolTable *scope = ext_parser->current_scope;
    while (scope) {
        SymbolEntry *entry = scope->entries;
        while (entry) {
            SymbolEntry *next = entry->next;
            if (entry->name) {
                free(entry->name);
            }
            free(entry);
            entry = next;
        }
        SymbolTable *parent = scope->parent;
        free(scope);
        scope = parent;
    }

    free(ext_parser);
}

// =============================================================================
// TOKEN UTILITIES
// =============================================================================

bool expect_token(Parser *parser, TokenType expected) {
    if (!parser)
        return false;

    if (parser->current_token.type == expected) {
        advance_token(parser);
        return true;
    }

    // Special handling for >> when expecting >
    // This handles nested generics like Vec<Vec<T>>
    if (expected == TOKEN_GREATER_THAN && parser->current_token.type == TOKEN_RIGHT_SHIFT) {
        // Convert >> to > by modifying the current token
        // We consume one > and leave the second > for the next parse
        parser->current_token.type = TOKEN_GREATER_THAN;
        // Don't advance, just return true to consume the first >
        return true;
    }

    char error_msg[256];
    snprintf(error_msg, sizeof(error_msg), "expected '%s' but found '%s'",
             token_type_display_name(expected),
             token_type_display_name(parser->current_token.type));
    report_error(parser, error_msg);

    return false;
}

bool match_token(Parser *parser, TokenType expected) {
    if (!parser)
        return false;
    return parser->current_token.type == expected;
}

bool consume_token(Parser *parser, TokenType expected) {
    if (!parser)
        return false;

    if (parser->current_token.type == expected) {
        advance_token(parser);
        return true;
    }
    return false;
}

Token advance_token(Parser *parser) {
    if (!parser) {
        Token error = {0};
        error.type = TOKEN_ERROR;
        return error;
    }

    Token previous = parser->current_token;

    if (!at_end(parser)) {
        do {
            // Free the current token before overwriting it to prevent memory leaks
            token_free(&parser->current_token);
            parser->current_token = lexer_next_token(parser->lexer);
        } while (parser->current_token.type == TOKEN_NEWLINE && !at_end(parser));
    }

    return previous;
}

Token peek_token(Parser *parser) {
    if (!parser) {
        Token error = {0};
        error.type = TOKEN_ERROR;
        return error;
    }

    return lexer_peek_token(parser->lexer);
}

Token peek_token_ahead(Parser *parser, size_t offset) {
    if (!parser) {
        Token error = {0};
        error.type = TOKEN_ERROR;
        return error;
    }

    // For simplicity, if offset is 0, just return current token
    if (offset == 0) {
        return parser->current_token;
    }

    // For offset of 1, return the peek token
    if (offset == 1) {
        return lexer_peek_token(parser->lexer);
    }

    // For offset of 2, return EOF for now (not supported yet)
    if (offset == 2) {
        Token eof_token = {0};
        eof_token.type = TOKEN_EOF;
        return eof_token;
    }

    // For larger offsets, we'd need more complex multi-token lookahead
    // For now, just return an EOF token to indicate we can't look that far ahead
    Token eof_token = {0};
    eof_token.type = TOKEN_EOF;
    return eof_token;
}

bool check_token(Parser *parser, TokenType expected) {
    if (!parser)
        return false;
    return parser->current_token.type == expected;
}

bool at_end(Parser *parser) {
    if (!parser)
        return true;
    return parser->current_token.type == TOKEN_EOF;
}

// =============================================================================
// PARSER INTERFACE FUNCTIONS
// =============================================================================

// Main parsing interface function - wrapper for modular parse_program
ASTNode *parser_parse_program(Parser *parser) {
    if (!parser)
        return NULL;

    // Clear any previous errors and reset statistics
    parser_clear_errors(parser);
    if (parser->config.collect_statistics) {
        parser_reset_statistics(parser);
    }

    // Call the modular parse_program function from grammar_toplevel.c
    ASTNode *program = parse_program(parser);

    // Update statistics if enabled
    if (parser->config.collect_statistics && program) {
        // Statistics are updated automatically by the grammar functions
    }

    return program;
}

const ParseError *parser_get_errors(Parser *parser) {
    if (!parser)
        return NULL;
    return parser->errors;
}

size_t parser_get_error_count(Parser *parser) {
    if (!parser)
        return 0;
    return parser->error_count;
}

void parser_clear_errors(Parser *parser) {
    if (!parser)
        return;

    ParseError *error = parser->errors;
    while (error) {
        ParseError *next = error->next;
        if (error->message) {
            free(error->message);
        }
        free(error);
        error = next;
    }

    parser->errors = NULL;
    parser->last_error = NULL;
    parser->error_count = 0;
    parser->had_error = false;
}

bool parser_had_error(Parser *parser) {
    if (!parser)
        return true;
    return parser->had_error;
}

void parser_set_strict_mode(Parser *parser, bool strict) {
    if (parser) {
        parser->config.strict_mode = strict;
    }
}

void parser_set_max_errors(Parser *parser, size_t max_errors) {
    if (parser) {
        parser->config.max_errors = max_errors;
    }
}

void parser_set_allow_incomplete(Parser *parser, bool allow) {
    if (parser) {
        parser->config.allow_incomplete_parse = allow;
    }
}

void parser_reset_statistics(Parser *parser) {
    if (!parser)
        return;

    // Reset all statistics counters to zero
    parser->stats.nodes_created = 0;
    parser->stats.tokens_consumed = 0;
    parser->stats.errors_reported = 0;
    parser->stats.warnings_reported = 0;
    parser->stats.parse_time_ms = 0;
    parser->stats.max_recursion_depth = 0;
    parser->stats.current_recursion_depth = 0;
}

// =============================================================================
// MISSING PARSER INTERFACE FUNCTIONS (Phase 2 Implementation)
// =============================================================================

// Expression parsing wrapper function
ASTNode *parser_parse_expression(Parser *parser) {
    if (!parser)
        return NULL;

    // Call the main expression parsing function from grammar_expressions.c
    return parse_expr(parser);
}

// Statement parsing wrapper function
ASTNode *parser_parse_statement(Parser *parser) {
    if (!parser)
        return NULL;

    // Call the main statement parsing function from grammar_statements.c
    return parse_statement(parser);
}

// TODO: Implement these parser functions - currently stubs for testing
ASTNode *parse_trait_decl(Parser *parser) {
    if (!parser)
        return NULL;

    // Traits are not implemented yet
    report_error(parser, "Trait declarations are not yet implemented");
    return NULL;
}

ASTNode *parse_impl_decl(Parser *parser) {
    if (!parser)
        return NULL;

    SourceLocation start_loc = parser->current_token.location;

    if (!expect_token(parser, TOKEN_IMPL)) {
        return NULL; // Not an impl declaration
    }

    // Parse optional trait name (e.g., `impl MyTrait for MyType { ... }`)
    ASTNode *trait_name_node = NULL;
    if (match_token(parser, TOKEN_IDENTIFIER)) {
        // This is a simplified check, a real parser would ensure it's a trait name
        trait_name_node = create_identifier(parser, parser->current_token.data.identifier.name);
        advance_token(parser);

        if (!expect_token(parser, TOKEN_FOR)) {
            report_error(parser, "Expected 'for' keyword after trait name in impl declaration");
            ast_free_node(trait_name_node);
            return NULL;
        }
    }

    // Parse the type for which the impl is written (e.g., `MyType` or `MyStruct<T>`)`
    ASTNode *for_type = parse_type(parser);
    if (!for_type) {
        report_error(parser, "Expected type for impl declaration");
        ast_free_node(trait_name_node);
        return NULL;
    }

    // Parse optional annotations before the block
    ASTNodeList *annotations = parse_annotation_list(parser);

    if (!expect_token(parser, TOKEN_LEFT_BRACE)) {
        report_error(parser, "Expected '{' after impl declaration");
        ast_free_node(trait_name_node);
        ast_free_node(for_type);
        if (annotations)
            ast_node_list_destroy(annotations);
        return NULL;
    }

    // Parse methods within the impl block
    ASTNodeList *methods = ast_node_list_create(4); // Initial capacity
    if (!methods) {
        report_error(parser, "Failed to allocate ASTNodeList for methods");
        ast_free_node(trait_name_node);
        ast_free_node(for_type);
        if (annotations)
            ast_node_list_destroy(annotations);
        return NULL;
    }

    while (!match_token(parser, TOKEN_RIGHT_BRACE) && !at_end(parser)) {
        ASTNode *method = parse_method_decl(parser);
        if (method) {
            ast_node_list_add(&methods, method);
        } else {
            // If we fail to parse a method, try to recover by skipping current token
            report_error(parser, "Failed to parse method in impl block");
            advance_token(parser); // Skip problematic token
        }
    }

    if (!expect_token(parser, TOKEN_RIGHT_BRACE)) {
        report_error(parser, "Expected '}' to close impl declaration");
        ast_free_node(trait_name_node);
        ast_free_node(for_type);
        if (annotations)
            ast_node_list_destroy(annotations);
        ast_node_list_destroy(methods);
        return NULL;
    }

    ASTNode *node = ast_create_node(AST_IMPL_BLOCK, start_loc);
    if (!node) {
        ast_free_node(trait_name_node);
        ast_free_node(for_type);
        if (annotations)
            ast_node_list_destroy(annotations);
        ast_node_list_destroy(methods);
        return NULL;
    }

    // Use the correct field names from the AST structure
    node->data.impl_block.struct_name =
        for_type ? strdup("TODO") : NULL; // TODO: Extract name from for_type
    node->data.impl_block.methods = methods;
    node->data.impl_block.annotations = annotations;

    // Clean up temporary nodes
    ast_free_node(trait_name_node);
    ast_free_node(for_type);

    return node;
}
