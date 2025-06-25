/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions Implementation
 * Utility functions for top-level grammar
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_toplevel.h"
#include <stdlib.h>

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

bool is_declaration_start(Parser *parser) {
    if (!parser)
        return false;

    switch (parser->current_token.type) {
    case TOKEN_FN:
    case TOKEN_STRUCT:
    case TOKEN_ENUM:
    case TOKEN_EXTERN:
    case TOKEN_IMPL:
    case TOKEN_CONST: // Const declarations
    case TOKEN_PUB:   // Visibility modifier
    case TOKEN_HASH:  // Annotations
        return true;
    default:
        return false;
    }
}
