/**
 * Asthra Programming Language Compiler
 * Parser symbol table integration functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_SYMBOL_H
#define ASTHRA_PARSER_SYMBOL_H

#include "ast.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// SYMBOL TABLE INTEGRATION
// =============================================================================

// Symbol table integration
void register_symbol(Parser *parser, const char *name, ASTNode *declaration);
ASTNode *lookup_symbol(Parser *parser, const char *name);
bool is_symbol_defined(Parser *parser, const char *name);

// Enhanced symbol table operations
bool register_symbol_in_scope(Parser *parser, const char *name, ASTNode *declaration, uint32_t scope_id);
void enter_new_scope(Parser *parser);
void exit_current_scope(Parser *parser);
uint32_t get_current_scope_id(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_SYMBOL_H 
