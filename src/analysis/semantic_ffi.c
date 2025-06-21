/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - FFI Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * FFI validation and extern function handling
 */

#include "semantic_ffi.h"
#include "semantic_core.h"
#include <stdlib.h>

// =============================================================================
// FFI TYPE VALIDATION
// =============================================================================

/**
 * Validate that a given TypeDescriptor is compatible with C ABI for FFI.
 * This is a simplified implementation. A full implementation would involve
 * detailed checks for struct packing, alignment, and complex type mappings.
 */
bool semantic_validate_ffi_type(SemanticAnalyzer *analyzer, TypeDescriptor *type) {
    if (!analyzer || !type) {
        return false;
    }

    switch (type->category) {
        case TYPE_BOOL:
        case TYPE_INTEGER:  // Covers all integer types (i8, i16, i32, i64, u8, u16, u32, u64)
        case TYPE_FLOAT:    // Covers floating point types (f32, f64)
        case TYPE_BUILTIN:  // Covers char, string, and other builtin types
            // Basic types are generally FFI compatible
            return true;

        case TYPE_POINTER:
            // Pointers are generally FFI compatible. Recursively check pointee type.
            return semantic_validate_ffi_type(analyzer, type->data.pointer.pointee_type);

        case TYPE_SLICE:
            // Slices are typically passed as a pointer + length pair in C. This is complex.
            // For now, we'll treat them as a pointer to elements, similar to arrays.
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNSUPPORTED_OPERATION,
                                 (SourceLocation){0}, // No specific location for type descriptor
                                 "Slices passed to FFI will decay to pointers. Ensure C function expects pointer type. Length information is lost.");
            return semantic_validate_ffi_type(analyzer, type->data.slice.element_type);
            
        case TYPE_STRUCT:
            // Structs require careful layout and packing for FFI. Complex.
            // For simplicity, for now, we only allow structs that are explicitly marked as FFI-compatible
            // or have a simple, flat structure (e.g., only primitive fields).
            // This needs significant enhancement for full FFI support.
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNSUPPORTED_OPERATION,
                                 (SourceLocation){0}, // No specific location for type descriptor
                                 "Structs in FFI require careful layout. Only simple structs are currently supported.");
            return true; // Placeholder: assume compatible for simple cases

        case TYPE_ENUM:
            // Enums are typically passed as their underlying integer type.
            // Assume compatible if underlying type is integer.
            return true; // Placeholder: assume compatible for now

        case TYPE_FUNCTION: // Function pointers (callbacks)
            semantic_report_error(analyzer, SEMANTIC_ERROR_UNSUPPORTED_OPERATION,
                                 (SourceLocation){0}, 
                                 "Function pointers (callbacks) in FFI require explicit type annotations and care.");
            return true; // Placeholder: assume compatible for now

        default:
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE,
                                 (SourceLocation){0}, 
                                 "Type '%s' is not compatible with C ABI for FFI", type->name);
            return false;
    }
}

bool semantic_check_ffi_annotations(SemanticAnalyzer *analyzer, ASTNodeList *annotations) {
    if (!analyzer || !annotations) return true;

    bool all_valid = true;
    for (size_t i = 0; i < annotations->count; i++) {
        ASTNode *annotation = annotations->nodes[i];
        if (!annotation || annotation->type != AST_FFI_ANNOTATION) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION,
                                 annotation->location,
                                 "Invalid FFI annotation structure");
            all_valid = false;
            continue;
        }

        FFITransferType transfer_type = annotation->data.ffi_annotation.transfer_type;
        switch (transfer_type) {
            case FFI_TRANSFER_FULL:
            case FFI_TRANSFER_NONE:
            case FFI_BORROWED:
                // These are transfer semantics, validation depends on usage context (e.g., param/return type)
                // For now, their presence is syntactically valid.
                break;
            default:
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_ANNOTATION,
                                     annotation->location,
                                     "Unsupported FFI transfer type: %d", transfer_type);
                all_valid = false;
                break;
        }
    }
    return all_valid;
}

bool semantic_validate_extern_function(SemanticAnalyzer *analyzer, ASTNode *extern_decl) {
    if (!analyzer || !extern_decl || extern_decl->type != AST_EXTERN_DECL) {
        return false;
    }

    const char *func_name = extern_decl->data.extern_decl.name;
    ASTNodeList *params = extern_decl->data.extern_decl.params;
    ASTNode *return_type_node = extern_decl->data.extern_decl.return_type;
    ASTNodeList *annotations = extern_decl->data.extern_decl.annotations;

    // 1. Validate FFI annotations on the extern declaration
    if (!semantic_check_ffi_annotations(analyzer, annotations)) {
        return false;
    }

    // 2. Validate return type compatibility with C ABI
    if (return_type_node) {
        TypeDescriptor *return_type_desc = analyze_type_node(analyzer, return_type_node);
        if (!return_type_desc) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, return_type_node->location,
                                 "Invalid return type for extern function '%s'", func_name);
            return false;
        }
        if (!semantic_validate_ffi_type(analyzer, return_type_desc)) {
            semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, return_type_node->location,
                                 "Return type '%s' of extern function '%s' is not C ABI compatible",
                                 return_type_desc->name, func_name);
            type_descriptor_release(return_type_desc);
            return false;
        }
        type_descriptor_release(return_type_desc);
    } else {
        // Default to void/none return type if not specified, which is C ABI compatible
        // No error, but ensure it maps correctly to C void.
    }

    // 3. Validate parameter types compatibility with C ABI and FFI annotations on parameters
    if (params) {
        for (size_t i = 0; i < params->count; i++) {
            ASTNode *param_decl = params->nodes[i];
            if (param_decl->type != AST_PARAM_DECL) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INTERNAL, param_decl->location,
                                     "Expected parameter declaration in extern function parameter list");
                return false;
            }
            
            TypeDescriptor *param_type_desc = analyze_type_node(analyzer, param_decl->data.param_decl.type);
            if (!param_type_desc) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, param_decl->location,
                                     "Invalid type for parameter '%s' in extern function '%s'",
                                     param_decl->data.param_decl.name, func_name);
                return false;
            }

            if (!semantic_validate_ffi_type(analyzer, param_type_desc)) {
                semantic_report_error(analyzer, SEMANTIC_ERROR_INVALID_TYPE, param_decl->location,
                                     "Parameter '%s' type '%s' of extern function '%s' is not C ABI compatible",
                                     param_decl->data.param_decl.name, param_type_desc->name, func_name);
                type_descriptor_release(param_type_desc);
                return false;
            }
            type_descriptor_release(param_type_desc);

            // Validate FFI annotations on the parameter itself
            if (!semantic_check_ffi_annotations(analyzer, param_decl->data.param_decl.annotations)) {
                return false;
            }
        }
    }

    // 4. Register the extern function in the symbol table (if not already done)
    // This should ideally be done when parsing declarations, but ensure FFI-specific flags are set.
    // For this task, we assume the symbol is already in the table and we're just validating.

    return true;
}

// =============================================================================
// FFI CONSTANT-TIME SAFETY (STUB IMPLEMENTATIONS)
// =============================================================================

bool semantic_validate_ffi_constant_time_safety(SemanticAnalyzer *analyzer, ASTNode *extern_decl) {
    (void)analyzer;
    (void)extern_decl;
    // TODO: Implement FFI constant-time safety validation
    return true;
}

bool semantic_validate_ffi_parameter_constant_time_safety(SemanticAnalyzer *analyzer, ASTNode *param) {
    (void)analyzer;
    (void)param;
    // TODO: Implement FFI parameter constant-time safety validation
    return true;
}

// =============================================================================
// FFI VOLATILE MEMORY SAFETY (STUB IMPLEMENTATIONS)
// =============================================================================

bool semantic_validate_ffi_volatile_memory_safety(SemanticAnalyzer *analyzer, ASTNode *call_expr) {
    (void)analyzer;
    (void)call_expr;
    // TODO: Implement FFI volatile memory safety validation
    return true;
} 
