/**
 * Asthra Programming Language
 * Parser Test Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of test utility functions for parser testing
 */

#include "parser_test_utils.h"
#include "performance_test_utils.h"
#include <string.h>

// =============================================================================
// PARSER TEST UTILITIES
// =============================================================================

Parser* create_test_parser(const char* source) {
    if (!source) {
        return NULL;
    }

    // Create lexer first, then parser
    Lexer* lexer = lexer_create(source, strlen(source), "test.astra");
    if (!lexer) {
        return NULL;
    }

    Parser* parser = parser_create(lexer);
    if (parser) {
        track_memory_allocation(sizeof(Parser));
    }
    return parser;
}

void destroy_test_parser(Parser* parser) {
    if (parser) {
        track_memory_deallocation(sizeof(Parser));
        parser_destroy(parser);
    }
}

ASTNode* parse_test_source(const char* source, const char* filename) {
    Parser* parser = create_test_parser(source);
    if (!parser) {
        return NULL;
    }

    // Parse the source and create AST
    ASTNode* ast = parse_program(parser);
    if (ast) {
        track_memory_allocation(sizeof(ASTNode));
    }

    destroy_test_parser(parser);
    return ast;
}
