/**
 * Asthra Programming Language Compiler
 * String and multi-line string scanning functions for Asthra grammar
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef LEXER_SCAN_STRINGS_H
#define LEXER_SCAN_STRINGS_H

#include "lexer_scan_types.h"

/**
 * Check if the current position starts a multi-line string
 * Checks for both raw (r""") and processed (""") multi-line strings
 */
bool is_multiline_string_start(Lexer *lexer);

/**
 * Scan raw multi-line string (r"""content""")
 * Assumes the lexer is positioned at the 'r' character
 * No escape sequence processing is performed
 */
Token scan_multiline_raw_string(Lexer *lexer);

/**
 * Scan processed multi-line string ("""content""")
 * Assumes the lexer is positioned at the first '"' character
 * Escape sequences are processed like regular strings
 */
Token scan_multiline_processed_string(Lexer *lexer);

#endif // LEXER_SCAN_STRINGS_H 
