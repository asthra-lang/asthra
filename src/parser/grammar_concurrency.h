/**
 * Asthra Programming Language Compiler - Concurrency Grammar Productions
 * Advanced concurrency parsing including channels, select, and worker pools
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_GRAMMAR_CONCURRENCY_H
#define ASTHRA_GRAMMAR_CONCURRENCY_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONCURRENCY GRAMMAR PRODUCTIONS
// =============================================================================

// Tier 1 concurrency statements (Core & Simple)
ASTNode *parse_spawn_with_handle_stmt(Parser *parser);

// Note: Tier 3 concurrency functions moved to stdlib:
// parse_select_stmt, parse_send_stmt, parse_close_stmt,
// parse_channel_decl, parse_worker_pool_decl,
// parse_channel_type, parse_task_handle_type

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GRAMMAR_CONCURRENCY_H 
