/**
 * Asthra Programming Language Compiler
 * Token scanning functions for Asthra grammar (modular version)
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

/**
 * This file is intentionally empty. All scanning functionality has been moved to:
 * 
 * - lexer_scan_escape.c: Escape sequence processing  
 * - lexer_scan_core.c: Core token scanning functions
 * - lexer_scan_numbers.c: Numeric literal scanners
 * - lexer_scan_strings.c: String and multi-line string scanners
 *
 * The functions are available through the individual headers or the umbrella
 * header lexer_scan_modular.h.
 *
 * This file is kept in the build system for backward compatibility.
 */

// Dummy declaration to satisfy ISO C translation unit requirement
extern int _dummy_lexer_scan;
int _dummy_lexer_scan = 0; 
