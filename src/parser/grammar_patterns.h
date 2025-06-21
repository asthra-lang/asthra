/**
 * Asthra Programming Language Compiler - Pattern Grammar Productions
 * Pattern matching for match statements and destructuring
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_GRAMMAR_PATTERNS_H
#define ASTHRA_GRAMMAR_PATTERNS_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PATTERN GRAMMAR PRODUCTIONS
// =============================================================================

// Core pattern parsing
ASTNode *parse_pattern(Parser *parser);

// Specific pattern types
ASTNode *parse_enum_pattern(Parser *parser);
ASTNode *parse_struct_pattern(Parser *parser);
ASTNode *parse_field_pattern(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GRAMMAR_PATTERNS_H 
