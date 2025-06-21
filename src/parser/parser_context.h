/**
 * Asthra Programming Language Compiler
 * Parser context-sensitive parsing functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_CONTEXT_H
#define ASTHRA_PARSER_CONTEXT_H

#include "ast.h"
#include "parser_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// CONTEXT-SENSITIVE PARSING
// =============================================================================

// Context-sensitive parsing
ASTNode *parse_in_context(Parser *parser, ParseContext context);
void push_parse_context(Parser *parser, ParseContext context);
void pop_parse_context(Parser *parser);
ParseContext current_parse_context(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_CONTEXT_H 
