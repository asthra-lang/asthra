/**
 * FFI Test Stubs - AST and Parser
 *
 * Header file for enhanced AST and parser stub functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_STUBS_AST_PARSER_H
#define ASTHRA_FFI_STUBS_AST_PARSER_H

#include "ffi_stubs_types.h"

// =============================================================================
// ENHANCED AST FUNCTIONS
// =============================================================================

// Enhanced AST management with reference counting
EnhancedASTNode *enhanced_ast_create_node(ASTNodeType type, void *data, size_t data_size);
void enhanced_ast_retain_node(EnhancedASTNode *node);
void enhanced_ast_release_node(EnhancedASTNode *node);

// Legacy compatibility wrapper
void ast_free_node(ASTNode *node);

// =============================================================================
// ENHANCED PARSER FUNCTIONS
// =============================================================================

// Enhanced Parser creation and destruction
EnhancedParser *enhanced_parser_create(void);
void enhanced_parser_destroy(EnhancedParser *parser);

// Enhanced parsing
EnhancedASTNode *enhanced_parser_parse_program(EnhancedParser *parser, const char *input);

// Legacy compatibility wrapper (simplified for testing)
ASTNode *parser_parse(const char *input);

#endif // ASTHRA_FFI_STUBS_AST_PARSER_H