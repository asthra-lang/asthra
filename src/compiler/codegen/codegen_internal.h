/**
 * Asthra Programming Language Compiler
 * Internal code generation header - shared between codegen modules
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef CODEGEN_INTERNAL_H
#define CODEGEN_INTERNAL_H

#include "../../parser/ast.h"
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SHARED CONTEXT
// =============================================================================

// Global context for tracking current function during code generation
extern const char *current_function_name;
extern bool current_function_returns_void;

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Check if an AST node ends with a return statement
bool ends_with_return(ASTNode *node);

// Convert AST type nodes to C type strings
const char *get_c_type_string(ASTNode *type_node);

// =============================================================================
// MODULE FUNCTIONS
// =============================================================================

// Literals
int c_generate_string_literal(FILE *output, ASTNode *node);
int c_generate_integer_literal(FILE *output, ASTNode *node);
int c_generate_float_literal(FILE *output, ASTNode *node);
int c_generate_bool_literal(FILE *output, ASTNode *node);
int c_generate_unit_literal(FILE *output, ASTNode *node);

// Expressions
int c_generate_binary_expr(FILE *output, ASTNode *node);
int c_generate_unary_expr(FILE *output, ASTNode *node);
int c_generate_call_expr(FILE *output, ASTNode *node);
int c_generate_identifier(FILE *output, ASTNode *node);

// Statements
int c_generate_let_stmt(FILE *output, ASTNode *node);
int c_generate_return_stmt(FILE *output, ASTNode *node);
int c_generate_expr_stmt(FILE *output, ASTNode *node);
int c_generate_block(FILE *output, ASTNode *node);

// Control flow
int c_generate_if_stmt(FILE *output, ASTNode *node);
int c_generate_for_stmt(FILE *output, ASTNode *node);
int c_generate_match_stmt(FILE *output, ASTNode *node);

// Functions
int c_generate_program(FILE *output, ASTNode *node);
int c_generate_function_decl(FILE *output, ASTNode *node);

// Arrays/Slices
int c_generate_array_literal(FILE *output, ASTNode *node);
int c_generate_slice_expr(FILE *output, ASTNode *node);
int c_generate_index_access(FILE *output, ASTNode *node);
int c_generate_slice_length_access(FILE *output, ASTNode *node);

// Forward declaration for the main generator (needed by modules)
int generate_c_code(FILE *output, ASTNode *node);

#ifdef __cplusplus
}
#endif

#endif // CODEGEN_INTERNAL_H