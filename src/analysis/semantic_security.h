/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Security Annotation Module Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Security annotation validation (constant-time, volatile memory)
 */

#ifndef ASTHRA_SEMANTIC_SECURITY_H
#define ASTHRA_SEMANTIC_SECURITY_H

#include "semantic_analyzer.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SECURITY ANNOTATION VALIDATION
// =============================================================================

/**
 * Check security annotations for validity
 */
bool semantic_check_security_annotations(SemanticAnalyzer *analyzer, ASTNodeList *annotations);

/**
 * Validate security annotation context
 */
bool semantic_validate_security_annotation_context(SemanticAnalyzer *analyzer, ASTNode *node, SecurityType security_type);

// =============================================================================
// CONSTANT-TIME VALIDATION
// =============================================================================

/**
 * Validate constant-time function constraints
 */
bool semantic_validate_constant_time_function(SemanticAnalyzer *analyzer, ASTNode *func_decl);

/**
 * Validate constant-time block
 */
bool semantic_validate_constant_time_block(SemanticAnalyzer *analyzer, ASTNode *block);

/**
 * Validate constant-time statement
 */
bool semantic_validate_constant_time_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);

/**
 * Validate constant-time function call
 */
bool semantic_validate_constant_time_call(SemanticAnalyzer *analyzer, ASTNode *call_expr);

/**
 * Validate constant-time expression
 */
bool semantic_validate_constant_time_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

// =============================================================================
// VOLATILE MEMORY VALIDATION
// =============================================================================

/**
 * Validate volatile memory access
 */
bool semantic_validate_volatile_memory_access(SemanticAnalyzer *analyzer, ASTNode *node);

/**
 * Validate volatile memory argument
 */
bool semantic_validate_volatile_memory_argument(SemanticAnalyzer *analyzer, ASTNode *arg);

/**
 * Check volatile assignment
 */
bool semantic_check_volatile_assignment(SemanticAnalyzer *analyzer, ASTNode *assignment);

/**
 * Check volatile field access
 */
bool semantic_check_volatile_field_access(SemanticAnalyzer *analyzer, ASTNode *field_access);

/**
 * Check volatile dereference
 */
bool semantic_check_volatile_dereference(SemanticAnalyzer *analyzer, ASTNode *deref);

// =============================================================================
// ANNOTATION HELPERS
// =============================================================================

/**
 * Check if declaration has volatile annotation
 */
bool semantic_has_volatile_annotation(ASTNode *declaration);

/**
 * Check if declaration has constant-time annotation
 */
bool semantic_has_constant_time_annotation(ASTNode *declaration);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_SECURITY_H 
