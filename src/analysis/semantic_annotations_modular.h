/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Modular Umbrella Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Umbrella header for the modular semantic annotations system
 * Provides backward compatibility and unified access to all annotation modules
 * 
 * This header replaces the monolithic semantic_annotations.h and provides
 * access to all split modules while maintaining the same public API.
 */

#ifndef ASTHRA_SEMANTIC_ANNOTATIONS_MODULAR_H
#define ASTHRA_SEMANTIC_ANNOTATIONS_MODULAR_H

// =============================================================================
// MODULAR ANNOTATION SYSTEM INCLUDES
// =============================================================================

// Core registry and type definitions
#include "semantic_annotations_registry.h"

// Error reporting and warnings
#include "semantic_annotations_errors.h"

// Core validation functions
#include "semantic_annotations_validation.h"

// Conflict detection and duplicate checking
#include "semantic_annotations_conflicts.h"

// Parameter validation
#include "semantic_annotations_parameters.h"

// Inheritance and dependency validation
#include "semantic_annotations_inheritance.h"

// Concurrency annotation handling
#include "semantic_annotations_concurrency.h"

// FFI annotation validation
#include "semantic_annotations_ffi.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BACKWARD COMPATIBILITY NOTES
// =============================================================================

/*
 * This modular system provides the same public API as the original
 * semantic_annotations.h file, but with improved organization:
 * 
 * 1. semantic_annotations_registry.{h,c} - Annotation definitions and lookup
 * 2. semantic_annotations_errors.{h,c} - Error reporting and warnings
 * 3. semantic_annotations_validation.{h,c} - Core validation logic
 * 4. semantic_annotations_conflicts.{h,c} - Conflict and duplicate detection
 * 5. semantic_annotations_parameters.{h,c} - Parameter validation
 * 6. semantic_annotations_inheritance.{h,c} - Inheritance and dependencies
 * 7. semantic_annotations_concurrency.{h,c} - Concurrency annotation handling
 * 8. semantic_annotations_ffi.{h,c} - FFI annotation validation
 * 
 * All original functions are available through their respective modules.
 * Include this header to get access to the complete annotation system.
 */

// =============================================================================
// UNIFIED API DOCUMENTATION
// =============================================================================

/*
 * MAIN ENTRY POINTS:
 * 
 * Registry Functions:
 * - find_semantic_tag_definition()
 * - find_parameter_schema()
 * - ast_node_type_to_context()
 * - get_node_annotations()
 * 
 * Validation Functions:
 * - analyze_semantic_tag()
 * - validate_annotation_context()
 * - has_non_deterministic_annotation()
 * - analyze_declaration_annotations()
 * - analyze_statement_annotations()
 * - analyze_expression_annotations()
 * 
 * Conflict Detection:
 * - check_annotation_conflicts()
 * - check_duplicate_annotations()
 * 
 * Parameter Validation:
 * - validate_annotation_parameters()
 * 
 * Inheritance and Dependencies:
 * - analyze_annotation_inheritance()
 * - validate_annotation_dependencies()
 * - check_annotation_scope_resolution()
 * 
 * Concurrency Features:
 * - validate_non_deterministic_annotation()
 * - validate_tier2_concurrency_annotation()
 * - analyze_tier1_concurrency_feature()
 * 
 * FFI Validation:
 * - validate_ffi_transfer_annotation_context()
 * - validate_function_ffi_annotations()
 * - validate_extern_ffi_annotations()
 * - is_ffi_transfer_annotation()
 * 
 * Error Reporting:
 * - report_unknown_annotation_error()
 * - report_parameter_validation_error()
 * - report_context_validation_error()
 * - report_missing_non_deterministic_annotation()
 * - issue_annotation_warning()
 */

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANNOTATIONS_MODULAR_H 
