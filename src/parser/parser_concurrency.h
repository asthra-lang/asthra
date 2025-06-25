/**
 * Asthra Programming Language Compiler
 * Parser concurrency parsing functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_CONCURRENCY_H
#define ASTHRA_PARSER_CONCURRENCY_H

#include "ast.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// CONCURRENCY PARSING FUNCTIONS
// =============================================================================

// Tier 1 Concurrency Parsing Functions (Core & Simple)
ASTNode *parse_spawn_with_handle_stmt(Parser *parser);
ASTNode *parse_await_expr(Parser *parser);

// Note: Tier 3 concurrency parsing functions moved to stdlib:
// parse_channel_decl, parse_send_stmt, parse_recv_expr, parse_select_stmt,
// parse_select_case, parse_worker_pool_decl, parse_close_stmt,
// parse_channel_type, parse_task_handle_type, parse_worker_pool_type,
// parse_timeout_expr

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_CONCURRENCY_H
