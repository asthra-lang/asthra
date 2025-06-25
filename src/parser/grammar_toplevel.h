/**
 * Asthra Programming Language Compiler - Top-Level Grammar Productions
 * Package declarations, imports, and top-level constructs
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_GRAMMAR_TOPLEVEL_H
#define ASTHRA_GRAMMAR_TOPLEVEL_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TOP-LEVEL GRAMMAR PRODUCTIONS
// =============================================================================

// Program structure
ASTNode *parse_program(Parser *parser);
ASTNode *parse_package_decl(Parser *parser);
ASTNode *parse_import_decl(Parser *parser);

ASTNode *parse_top_level_decl(Parser *parser);

// Declaration types
ASTNode *parse_function_decl(Parser *parser);
ASTNode *parse_struct_decl(Parser *parser);
ASTNode *parse_enum_decl(Parser *parser);
ASTNode *parse_extern_decl(Parser *parser);
ASTNode *parse_impl_block(Parser *parser);
ASTNode *parse_method_decl(Parser *parser);
ASTNode *parse_const_decl(Parser *parser);
ASTNode *parse_trait_decl(Parser *parser);

// Parameter parsing (v1.18 Grammar Implementation)
ASTNode *parse_param(Parser *parser);
ASTNode *parse_extern_param(Parser *parser);

// Package system
ASTNode *parse_visibility_modifier(Parser *parser);

// Helper functions
bool is_declaration_start(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GRAMMAR_TOPLEVEL_H
