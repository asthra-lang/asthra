/**
 * Asthra Programming Language Compiler
 * Context management and symbol table functionality
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "parser.h"
#include <stdlib.h>
#include <string.h>

// Forward declarations for internal types
typedef struct SymbolEntry {
    char *name;
    ASTNode *declaration;
    struct SymbolEntry *next;
} SymbolEntry;

struct SymbolTable {
    SymbolEntry *entries;
    struct SymbolTable *parent;
};

typedef struct ParseContextFrame {
    ParseContext context;
    struct ParseContextFrame *next;
} ParseContextFrame;

typedef struct {
    Parser base;
    ParseContextFrame *context_stack;
    SymbolTable *current_scope;
} ExtendedParser;

// =============================================================================
// CONTEXT MANAGEMENT
// =============================================================================

void push_parse_context(Parser *parser, ParseContext context) {
    if (!parser)
        return;

    ExtendedParser *ext_parser = (ExtendedParser *)parser;
    ParseContextFrame *frame = malloc(sizeof(ParseContextFrame));
    if (!frame)
        return;

    frame->context = context;
    frame->next = ext_parser->context_stack;
    ext_parser->context_stack = frame;
}

void pop_parse_context(Parser *parser) {
    if (!parser)
        return;

    ExtendedParser *ext_parser = (ExtendedParser *)parser;
    if (!ext_parser->context_stack)
        return;

    ParseContextFrame *frame = ext_parser->context_stack;
    ext_parser->context_stack = frame->next;
    free(frame);
}

ParseContext current_parse_context(Parser *parser) {
    if (!parser)
        return PARSE_CONTEXT_EXPRESSION;

    ExtendedParser *ext_parser = (ExtendedParser *)parser;
    if (!ext_parser->context_stack)
        return PARSE_CONTEXT_EXPRESSION;

    return ext_parser->context_stack->context;
}

// =============================================================================
// SYMBOL TABLE MANAGEMENT
// =============================================================================

void register_symbol(Parser *parser, const char *name, ASTNode *declaration) {
    if (!parser || !name || !declaration)
        return;

    ExtendedParser *ext_parser = (ExtendedParser *)parser;
    if (!ext_parser->current_scope)
        return;

    SymbolEntry *entry = malloc(sizeof(SymbolEntry));
    if (!entry)
        return;

    entry->name = strdup(name);
    entry->declaration = declaration;
    entry->next = ext_parser->current_scope->entries;
    ext_parser->current_scope->entries = entry;
}

ASTNode *lookup_symbol(Parser *parser, const char *name) {
    if (!parser || !name)
        return NULL;

    ExtendedParser *ext_parser = (ExtendedParser *)parser;
    SymbolTable *scope = ext_parser->current_scope;

    while (scope) {
        SymbolEntry *entry = scope->entries;
        while (entry) {
            if (strcmp(entry->name, name) == 0) {
                return entry->declaration;
            }
            entry = entry->next;
        }
        scope = scope->parent;
    }

    return NULL;
}

bool is_symbol_defined(Parser *parser, const char *name) {
    return lookup_symbol(parser, name) != NULL;
}

// =============================================================================
// CONTEXT DETECTION HELPERS
// =============================================================================

bool is_postfix_suffix_start(Parser *parser) {
    if (!parser)
        return false;

    TokenType current = parser->current_token.type;
    return current == TOKEN_LEFT_PAREN || current == TOKEN_DOT || current == TOKEN_LEFT_BRACKET;
}

bool is_lvalue_context(Parser *parser) {
    if (!parser)
        return false;

    // Look ahead to check if we might be in an assignment context
    Parser parser_copy = *parser;

    // Skip the current expression (potential lvalue)
    // This is a simplified approach - in a real parser we'd track the current context
    // For now, we'll just check if the next token is an assignment operator
    int paren_level = 0;
    int bracket_level = 0;
    int brace_level = 0;

    while (!at_end(&parser_copy)) {
        TokenType type = parser_copy.current_token.type;

        // Skip balanced delimiters
        if (type == TOKEN_LEFT_PAREN)
            paren_level++;
        else if (type == TOKEN_RIGHT_PAREN)
            paren_level--;
        else if (type == TOKEN_LEFT_BRACKET)
            bracket_level++;
        else if (type == TOKEN_RIGHT_BRACKET)
            bracket_level--;
        else if (type == TOKEN_LEFT_BRACE)
            brace_level++;
        else if (type == TOKEN_RIGHT_BRACE)
            brace_level--;

        // Stop at expression boundaries when delimiters are balanced
        else if ((type == TOKEN_SEMICOLON || type == TOKEN_COMMA) && paren_level == 0 &&
                 bracket_level == 0 && brace_level == 0) {
            break;
        }

        // Check if we're at an assignment operator
        else if (type == TOKEN_ASSIGN && paren_level == 0 && bracket_level == 0 &&
                 brace_level == 0) {
            return true;
        }

        advance_token(&parser_copy);
    }

    return false;
}

bool is_function_call_start(Parser *parser) {
    if (!parser)
        return false;

    return match_token(parser, TOKEN_LEFT_PAREN);
}
