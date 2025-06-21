/**
 * Asthra Programming Language Compiler
 * Numeric literal scanning functions for Asthra grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef LEXER_SCAN_NUMBERS_H
#define LEXER_SCAN_NUMBERS_H

#include "lexer_scan_types.h"

/**
 * Scan hexadecimal literal (0x prefix)
 * Assumes the "0x" prefix has already been consumed
 */
Token scan_hex_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);

/**
 * Scan binary literal (0b prefix)
 * Assumes the "0b" prefix has already been consumed
 */
Token scan_binary_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);

/**
 * Scan octal literal (0o prefix)
 * Assumes the "0o" prefix has already been consumed
 */
Token scan_octal_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);

/**
 * Scan decimal literal (integers and floats)
 * Handles both integer and floating-point numbers
 */
Token scan_decimal_literal(Lexer *lexer, SourceLocation start_loc, size_t start_pos);

#endif // LEXER_SCAN_NUMBERS_H 
