/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - FFI Annotations Validation Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * FFI annotation validation for SafeFFIAnnotation implementation
 * Phase 3: Semantic Analysis Enhancement for FFI Annotation Ambiguity Fix
 *
 * Implements context-specific validation rules:
 * 1. Return Types: Only #[transfer_full] or #[transfer_none] allowed
 * 2. Parameters: Only #[borrowed], #[transfer_full], or #[transfer_none] allowed
 * 3. Mutual Exclusivity: At most one annotation per declaration
 * 4. Context Validation: Ensure annotations match C interop patterns
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_FFI_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_FFI_H

#include "../parser/ast.h"
#include "../parser/ast_types.h"
#include "semantic_annotations_registry.h"
#include "semantic_core.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// FFI ANNOTATION VALIDATION FUNCTIONS
// =============================================================================

/**
 * Validate FFI transfer annotation in context
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The name of the FFI annotation (transfer_full, transfer_none, borrowed)
 * @param target_node The node being annotated (function, parameter, etc.)
 * @param context The specific context (CONTEXT_PARAMETER or CONTEXT_RETURN_TYPE)
 * @return true if annotation is valid in this context, false otherwise
 */
bool validate_ffi_transfer_annotation_context(SemanticAnalyzer *analyzer,
                                              const char *annotation_name, ASTNode *target_node,
                                              AnnotationContext context);

/**
 * Validate return type FFI annotation
 * Only #[transfer_full] or #[transfer_none] are allowed on return types
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The annotation name to validate
 * @param function_node The function or extern declaration node
 * @return true if annotation is valid for return type, false otherwise
 */
bool validate_return_type_ffi_annotation(SemanticAnalyzer *analyzer, const char *annotation_name,
                                         ASTNode *function_node);

/**
 * Validate parameter FFI annotation
 * All FFI transfer annotations are allowed on parameters
 * @param analyzer The semantic analyzer instance
 * @param annotation_name The annotation name to validate
 * @param param_node The parameter declaration node
 * @return true if annotation is valid for parameter, false otherwise
 */
bool validate_parameter_ffi_annotation(SemanticAnalyzer *analyzer, const char *annotation_name,
                                       ASTNode *param_node);

/**
 * Check if an annotation is an FFI transfer annotation
 * @param annotation_name The annotation name to check
 * @return true if it's transfer_full, transfer_none, or borrowed
 */
bool is_ffi_transfer_annotation(const char *annotation_name);

/**
 * Validate that no conflicting FFI annotations exist
 * This should not happen with SafeFFIAnnotation grammar, but validates for safety
 * @param analyzer The semantic analyzer instance
 * @param node The node with annotations to check
 * @param annotations List of annotations to validate
 * @return true if no conflicts found, false if conflicts detected
 */
bool validate_ffi_annotation_mutual_exclusivity(SemanticAnalyzer *analyzer, ASTNode *node,
                                                ASTNodeList *annotations);

/**
 * Get FFI annotation context for a node
 * @param node The AST node to determine context for
 * @return CONTEXT_PARAMETER, CONTEXT_RETURN_TYPE, or CONTEXT_ANY
 */
AnnotationContext get_ffi_annotation_context(ASTNode *node);

/**
 * Validate FFI annotation on function declaration
 * Handles both function return type annotations and parameter annotations
 * @param analyzer The semantic analyzer instance
 * @param func_node The function declaration node
 * @return true if all FFI annotations are valid, false otherwise
 */
bool validate_function_ffi_annotations(SemanticAnalyzer *analyzer, ASTNode *func_node);

/**
 * Validate FFI annotation on extern declaration
 * Handles both extern return type annotations and parameter annotations
 * @param analyzer The semantic analyzer instance
 * @param extern_node The extern declaration node
 * @return true if all FFI annotations are valid, false otherwise
 */
bool validate_extern_ffi_annotations(SemanticAnalyzer *analyzer, ASTNode *extern_node);

// =============================================================================
// FFI ANNOTATION ERROR REPORTING
// =============================================================================

/**
 * Report invalid FFI annotation context error
 * @param analyzer The semantic analyzer instance
 * @param node The node with the invalid annotation
 * @param annotation_name The invalid annotation name
 * @param context The context where it was used
 */
void report_invalid_ffi_annotation_context_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                                 const char *annotation_name,
                                                 AnnotationContext context);

/**
 * Report FFI annotation conflict error
 * @param analyzer The semantic analyzer instance
 * @param node The node with conflicting annotations
 * @param first_annotation First conflicting annotation
 * @param second_annotation Second conflicting annotation
 */
void report_ffi_annotation_conflict_error(SemanticAnalyzer *analyzer, ASTNode *node,
                                          const char *first_annotation,
                                          const char *second_annotation);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_FFI_H
