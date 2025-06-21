/**
 * Asthra Programming Language Compiler - Statement Grammar Utilities
 * Helper functions and utilities for statement parsing
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "grammar_statements.h"
#include "grammar_expressions.h"
#include "grammar_patterns.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// =============================================================================
// HELPER FUNCTIONS
// =============================================================================

bool is_statement_start(Parser *parser) {
    if (!parser) return false;
    
    switch (parser->current_token.type) {
        case TOKEN_LET:
        case TOKEN_RETURN:
        case TOKEN_IF:
        case TOKEN_FOR:
        case TOKEN_MATCH:
        case TOKEN_SPAWN:
        case TOKEN_LEFT_BRACE:
        case TOKEN_IDENTIFIER:  // Could be assignment or expression statement
            return true;
        default:
            return is_expression_start(parser);
    }
} 
