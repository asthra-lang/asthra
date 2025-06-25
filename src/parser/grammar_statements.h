/**
 * Asthra Programming Language Compiler - Statement Grammar Productions
 * Statement parsing including control flow and declarations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_GRAMMAR_STATEMENTS_H
#define ASTHRA_GRAMMAR_STATEMENTS_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STATEMENT GRAMMAR PRODUCTIONS
// =============================================================================

// Core statements
ASTNode *parse_statement(Parser *parser);
ASTNode *parse_var_decl(Parser *parser);
ASTNode *parse_return_stmt(Parser *parser);
ASTNode *parse_expr_stmt(Parser *parser);
ASTNode *parse_assign_stmt(Parser *parser);
ASTNode *parse_block(Parser *parser);
ASTNode *parse_unsafe_block(Parser *parser);

// Control flow statements
ASTNode *parse_if_stmt(Parser *parser);
ASTNode *parse_if_let_stmt(Parser *parser);
ASTNode *parse_for_stmt(Parser *parser);
ASTNode *parse_match_stmt(Parser *parser);
ASTNode *parse_match_arm(Parser *parser);
ASTNode *parse_break_stmt(Parser *parser);
ASTNode *parse_continue_stmt(Parser *parser);

// Lvalue parsing for assignments
ASTNode *parse_lvalue(Parser *parser);

// Concurrency statements
ASTNode *parse_spawn_stmt(Parser *parser);

// Type parsing
ASTNode *parse_type(Parser *parser);

// Helper functions
bool is_statement_start(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GRAMMAR_STATEMENTS_H
