/**
 * Asthra Programming Language Compiler
 * Escape sequence processing functions for Asthra grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef LEXER_SCAN_ESCAPE_H
#define LEXER_SCAN_ESCAPE_H

#include "lexer_scan_types.h"

// =============================================================================
// ESCAPE SEQUENCE PROCESSING
// =============================================================================

/**
 * Process escape sequences in string and character literals
 * Returns the processed character, or sets error on invalid sequence
 */
char process_escape_sequence(Lexer *lexer, char escaped_char, bool *error_occurred);

#endif // LEXER_SCAN_ESCAPE_H 
