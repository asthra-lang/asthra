/**
 * Asthra Programming Language Compiler
 * Main parser file - includes all parser modules
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "parser.h"
#include "grammar_annotations.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include "grammar_statements.h"
#include "grammar_toplevel.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations for concurrency parsing functions
ASTNode *parse_spawn_with_handle_stmt(Parser *parser);
ASTNode *parse_await_expr(Parser *parser);
// Note: Tier 3 parser functions moved to stdlib:
// parse_channel_decl, parse_send_stmt, parse_recv_expr, parse_select_stmt,
// parse_select_case, parse_channel_type, parse_task_handle_type,
// parse_worker_pool_decl, parse_close_stmt

// Note: All core parser functionality has been moved to separate files:
// - parser_core.c: Parser creation, destruction, and basic utilities
// - parser_errors.c: Error handling and reporting
// - parser_ast_helpers.c: AST node creation helpers
// - parser_operators.c: Operator utilities and precedence
// - parser_types.c: Type checking helpers
// - parser_context.c: Context management and symbol tables

// This file now serves as the main entry point and includes all modules
