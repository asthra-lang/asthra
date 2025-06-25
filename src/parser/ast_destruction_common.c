/**
 * Asthra Programming Language Compiler
 * AST Node Destruction - Common Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file contains implementation of common utilities for AST node destruction
 */

#include "ast_destruction_common.h"

/**
 * Global destruction statistics
 */
ASTDestructionStats ast_destruction_stats = {0, 0, 0};

/**
 * Initialize destruction statistics
 */
void ast_destruction_stats_init(void) {
    ast_destruction_stats.nodes_destroyed = 0;
    ast_destruction_stats.strings_freed = 0;
    ast_destruction_stats.lists_destroyed = 0;
}

/**
 * Get current destruction statistics
 */
ASTDestructionStats ast_destruction_stats_get(void) {
    return ast_destruction_stats;
}

/**
 * Reset destruction statistics
 */
void ast_destruction_stats_reset(void) {
    ast_destruction_stats_init();
}
