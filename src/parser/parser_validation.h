/**
 * Asthra Programming Language Compiler
 * Parser validation and semantic check functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_VALIDATION_H
#define ASTHRA_PARSER_VALIDATION_H

#include "parser_core.h"
#include "ast.h"
#include "lexer.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// TYPE AND CONTEXT CHECKING
// =============================================================================

// Type checking helpers (for parser validation)
bool is_valid_type_name(const char *name);
bool is_primitive_type(TokenType token_type);
bool is_pointer_type_start(Parser *parser);
bool is_slice_type_start(Parser *parser);
bool is_result_type_start(Parser *parser);

// Annotation parsing helpers
bool is_annotation_start(Parser *parser);
bool is_semantic_tag_start(Parser *parser);
bool is_ownership_tag_start(Parser *parser);
bool is_ffi_annotation_start(Parser *parser);
bool is_security_tag_start(Parser *parser);

// Statement and expression classification
bool is_statement_start(Parser *parser);
bool is_expression_start(Parser *parser);
bool is_declaration_start(Parser *parser);
bool is_pattern_start(Parser *parser);

// Enhanced context checking
bool is_valid_in_context(Parser *parser, ASTNodeType node_type, ParseContext context);
bool is_lvalue_expression(Parser *parser, ASTNode *expr);

// String interpolation validation
bool validate_interpolation_variables(Parser *parser, char **variable_names, size_t count);

// Function parameter validation
bool is_variadic_param_start(Parser *parser);

// =============================================================================
// DISAMBIGUATION HELPERS
// =============================================================================

// Resolve parsing ambiguities according to v1.25 grammar
bool is_postfix_suffix_start(Parser *parser);
bool is_lvalue_context(Parser *parser);
bool is_function_call_start(Parser *parser);
bool is_field_access_start(Parser *parser);
bool is_array_index_start(Parser *parser);
bool is_slice_length_access(Parser *parser);

// =============================================================================
// VALIDATION AND SEMANTIC CHECKS
// =============================================================================

// Grammar validation (ensures zero ambiguities)
bool validate_grammar_rules(Parser *parser);
bool validate_precedence_rules(Parser *parser);
bool validate_disambiguation_rules(Parser *parser);

// Semantic validation during parsing
bool validate_ffi_annotations(Parser *parser, ASTNode *annotations);
bool validate_ownership_consistency(Parser *parser, ASTNode *node);
bool validate_pattern_exhaustiveness(Parser *parser, ASTNode *match_stmt);
bool validate_result_type_usage(Parser *parser, ASTNode *result_type);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_VALIDATION_H 
