/**
 * Asthra Programming Language Compiler
 * Unified parser header - includes all parser modules
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This header provides a single include point for all parser functionality
 * while maintaining modular organization through separate header files.
 */

#ifndef ASTHRA_PARSER_H
#define ASTHRA_PARSER_H

// Core parser types and main interface
#include "parser_core.h"

// Specialized parser modules
#include "parser_ast_helpers.h"
#include "parser_concurrency.h"
#include "parser_context.h"
#include "parser_error.h"
#include "parser_generic.h"
#include "parser_grammar.h"
#include "parser_operators.h"
#include "parser_symbol.h"
#include "parser_token.h"
#include "parser_validation.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONVENIENCE FUNCTIONS (maintained for backward compatibility)
// =============================================================================

// Parser configuration and statistics
void parser_set_config(Parser *parser, ParserConfig config);
ParserConfig parser_get_config(const Parser *parser);
ParseStatistics parser_get_statistics(const Parser *parser);
void parser_reset_statistics(Parser *parser);

// Legacy configuration functions (maintained for compatibility)
void parser_set_strict_mode(Parser *parser, bool strict);
void parser_set_max_errors(Parser *parser, size_t max_errors);
void parser_set_allow_incomplete(Parser *parser, bool allow);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_H
