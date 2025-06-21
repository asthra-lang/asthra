/**
 * Asthra Programming Language Compiler
 * Core token scanning functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef LEXER_SCAN_CORE_H
#define LEXER_SCAN_CORE_H

#include "lexer_scan_types.h"

// =============================================================================
// CORE TOKEN SCANNING FUNCTIONS
// =============================================================================

/**
 * Main token scanning function
 */
Token scan_token(Lexer *lexer);

/**
 * Scan string literals (including multi-line strings)
 */
Token scan_string(Lexer *lexer);

/**
 * Scan identifiers and keywords
 */
Token scan_identifier(Lexer *lexer);

/**
 * Scan character literals
 */
Token scan_character(Lexer *lexer);

#endif // LEXER_SCAN_CORE_H 
