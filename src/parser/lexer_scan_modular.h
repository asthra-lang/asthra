/**
 * Asthra Programming Language Compiler
 * Modular token scanning header - includes all scanning modules
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef LEXER_SCAN_MODULAR_H
#define LEXER_SCAN_MODULAR_H

// Include all modular scanning components
#include "lexer_scan_types.h"
#include "lexer_scan_escape.h"
#include "lexer_scan_core.h"
#include "lexer_scan_numbers.h"
#include "lexer_scan_strings.h"

/**
 * This header provides a single include point for all lexer scanning
 * functionality that was previously contained in lexer_scan.c.
 * 
 * The scanning functions are now organized into logical modules:
 * - lexer_scan_types.h: Shared types and constants
 * - lexer_scan_escape.h: Escape sequence processing
 * - lexer_scan_core.h: Core token scanning (scan_token, scan_number, etc.)
 * - lexer_scan_numbers.h: Specialized numeric literal scanners
 * - lexer_scan_strings.h: String and multi-line string scanners
 */

#endif // LEXER_SCAN_MODULAR_H 
