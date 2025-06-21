/**
 * Asthra Programming Language Compiler
 * Shared types and constants for modular lexer scanning
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef LEXER_SCAN_TYPES_H
#define LEXER_SCAN_TYPES_H

#include "lexer_internal.h"
#include <stdbool.h>

// =============================================================================
// SHARED CONSTANTS
// =============================================================================

#define LEXER_SCAN_INITIAL_STRING_BUFFER_SIZE 1024
#define LEXER_SCAN_INITIAL_MULTILINE_BUFFER_SIZE 2048

// =============================================================================
// SHARED FUNCTION DECLARATIONS
// =============================================================================

/**
 * Helper function to process escape sequences
 * Returns the processed character, or sets error on invalid sequence
 */
char process_escape_sequence(Lexer *lexer, char escaped_char, bool *error_occurred);

#endif // LEXER_SCAN_TYPES_H
