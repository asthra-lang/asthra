/**
 * Asthra Programming Language Compiler
 * Parser grammar production function declarations
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_GRAMMAR_H
#define ASTHRA_PARSER_GRAMMAR_H

#include "ast.h"
#include "lexer.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration
typedef struct Parser Parser;

// =============================================================================
// GRAMMAR PRODUCTION FUNCTIONS (Asthra PEG Grammar) - ENHANCED
// =============================================================================

// Top-level productions with enhanced error handling
ASTNode *parse_program(Parser *parser);
ASTNode *parse_package_decl(Parser *parser);
ASTNode *parse_import_decl(Parser *parser);
ASTNode *parse_top_level_decl(Parser *parser);

// Annotations and attributes with validation
ASTNode *parse_semantic_tag(Parser *parser);
ASTNode *parse_human_review_tag(Parser *parser);
ASTNode *parse_security_tag(Parser *parser);
ASTNode *parse_ownership_tag(Parser *parser);
ASTNode *parse_ffi_transfer_tag(Parser *parser);
ASTNode *parse_lifetime_tag(Parser *parser);
ASTNode *parse_ffi_param_annotation(Parser *parser);

// Declarations with enhanced type checking
ASTNode *parse_param_list(Parser *parser);
ASTNode *parse_param(Parser *parser);
ASTNode *parse_extern_param_list(Parser *parser);
ASTNode *parse_extern_param(Parser *parser);

// Types with compile-time validation
ASTNode *parse_type(Parser *parser);
ASTNode *parse_base_type(Parser *parser);
ASTNode *parse_slice_type(Parser *parser);
ASTNode *parse_struct_type(Parser *parser);
ASTNode *parse_ptr_type(Parser *parser);
ASTNode *parse_result_type(Parser *parser);

// Statements and blocks with enhanced recovery
ASTNode *parse_block(Parser *parser);
ASTNode *parse_unsafe_block(Parser *parser);
ASTNode *parse_statement(Parser *parser);
ASTNode *parse_var_decl(Parser *parser);
ASTNode *parse_assign_stmt(Parser *parser);
ASTNode *parse_if_stmt(Parser *parser);
ASTNode *parse_if_let_stmt(Parser *parser);
ASTNode *parse_for_stmt(Parser *parser);
ASTNode *parse_return_stmt(Parser *parser);
ASTNode *parse_expr_stmt(Parser *parser);
ASTNode *parse_spawn_stmt(Parser *parser);
ASTNode *parse_match_stmt(Parser *parser);

// Pattern matching with exhaustiveness checking
ASTNode *parse_match_arm(Parser *parser);
ASTNode *parse_pattern(Parser *parser);
ASTNode *parse_enum_pattern(Parser *parser);
ASTNode *parse_struct_pattern(Parser *parser);
ASTNode *parse_field_pattern(Parser *parser);

// Expressions (disambiguated hierarchy) with precedence validation
ASTNode *parse_expr(Parser *parser);
ASTNode *parse_logic_or(Parser *parser);
ASTNode *parse_logic_and(Parser *parser);
ASTNode *parse_compare(Parser *parser);
ASTNode *parse_add(Parser *parser);
ASTNode *parse_mult(Parser *parser);
ASTNode *parse_unary(Parser *parser);
ASTNode *parse_postfix_expr(Parser *parser);
ASTNode *parse_postfix_suffix(Parser *parser);
ASTNode *parse_primary(Parser *parser);
ASTNode *parse_sizeof(Parser *parser);

// Function calls and L-values
ASTNode *parse_arg_list(Parser *parser);
ASTNode *parse_lvalue(Parser *parser);
ASTNode *parse_lvalue_suffix(Parser *parser);

// Literals with enhanced validation
ASTNode *parse_literal(Parser *parser);
ASTNode *parse_array_literal(Parser *parser);

// Built-in logging types
ASTNode *parse_log_type(Parser *parser);

// Package system declarations

ASTNode *parse_visibility_modifier(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_GRAMMAR_H
