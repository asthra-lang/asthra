/**
 * Asthra Programming Language Compiler - Annotation Grammar Productions
 * Annotation parsing for ownership, FFI, and security attributes
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_GRAMMAR_ANNOTATIONS_H
#define ASTHRA_GRAMMAR_ANNOTATIONS_H

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// ANNOTATION GRAMMAR PRODUCTIONS
// =============================================================================

// Core annotation parsing
ASTNode *parse_annotation(Parser *parser);
ASTNode *parse_bracketed_annotation(Parser *parser);
// parse_semantic_annotation removed - @ annotations no longer supported

// Ownership and memory management annotations
ASTNode *parse_ownership_annotation(Parser *parser);

// FFI and interoperability annotations
ASTNode *parse_ffi_annotation(Parser *parser);
ASTNode *parse_safe_ffi_annotation(Parser *parser);

// Security annotations
ASTNode *parse_security_annotation(Parser *parser);
ASTNode *parse_human_review_annotation(Parser *parser);

// Annotation list parsing
ASTNodeList *parse_annotation_list(Parser *parser);

// Helper functions
bool is_annotation_start(Parser *parser);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GRAMMAR_ANNOTATIONS_H 
