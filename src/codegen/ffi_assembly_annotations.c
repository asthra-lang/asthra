/**
 * Asthra Programming Language Compiler
 * FFI Assembly Generator - Annotation Handling
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 4: FFI Annotation Ambiguity Fix - Code Generation Updates
 * 
 * Implementation of FFI annotation extraction and handling for code generation.
 * Supports the SafeFFIAnnotation grammar changes that enforce mutual exclusivity.
 */

#include "ffi_assembly_generator.h"
#include "ffi_assembly_internal.h"
#include "../analysis/semantic_annotations_registry.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Helper function for type checking
static bool is_void_type(const TypeInfo *type_info) {
    return type_info && 
           type_info->category == TYPE_INFO_PRIMITIVE &&
           type_info->data.primitive.kind == PRIMITIVE_INFO_VOID;
}

// =============================================================================
// FFI ANNOTATION EXTRACTION
// =============================================================================

bool ffi_extract_annotation(FFIAssemblyGenerator *generator, ASTNode *node, FFIOwnershipTransferType *annotation_type) {
    if (!generator || !node || !annotation_type) {
        return false;
    }
    
    // Initialize to default (no annotation)
    *annotation_type = FFI_OWNERSHIP_NONE;
    
    // Get annotations list for this node
    ASTNodeList *annotations = get_node_annotations(node);
    if (!annotations) {
        return false; // No annotations found
    }
    
    // Search for FFI transfer annotations
    // Note: SafeFFIAnnotation grammar ensures at most one FFI annotation per node
    for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
        ASTNode *annotation = ast_node_list_get(annotations, i);
        if (!annotation || annotation->type != AST_SEMANTIC_TAG) {
            continue;
        }
        
        const char *annotation_name = annotation->data.semantic_tag.name;
        if (!annotation_name) {
            continue;
        }
        
        // Map annotation names to ownership transfer types
        if (strcmp(annotation_name, "transfer_full") == 0) {
            *annotation_type = FFI_OWNERSHIP_MOVE;
            emit_comment(generator, "FFI annotation: transfer_full -> move ownership");
            return true;
        } else if (strcmp(annotation_name, "transfer_none") == 0) {
            *annotation_type = FFI_OWNERSHIP_COPY;
            emit_comment(generator, "FFI annotation: transfer_none -> copy value");
            return true;
        } else if (strcmp(annotation_name, "borrowed") == 0) {
            *annotation_type = FFI_OWNERSHIP_BORROW;
            emit_comment(generator, "FFI annotation: borrowed -> temporary borrow");
            return true;
        }
    }
    
    return false; // No FFI annotation found
}

// =============================================================================
// ANNOTATION-BASED TRANSFER CODE GENERATION
// =============================================================================

bool ffi_generate_annotation_based_transfer(FFIAssemblyGenerator *generator,
                                           Register source_reg, Register target_reg,
                                           FFIOwnershipTransferType transfer_type) {
    if (!generator) {
        return false;
    }
    
    switch (transfer_type) {
        case FFI_OWNERSHIP_MOVE:
            // #[transfer_full] - Move ownership (caller releases, callee owns)
            emit_comment(generator, "Transfer full ownership (move)");
            
            // Simple register move for primitive types
            if (source_reg != target_reg) {
                emit_instruction(generator, INST_MOV, 2,
                               create_register_operand(target_reg),
                               create_register_operand(source_reg));
            }
            
            // For complex types, might need additional cleanup
            // For now, treat as simple move
            return true;
            
        case FFI_OWNERSHIP_COPY:
            // #[transfer_none] - Copy value (caller retains ownership)
            emit_comment(generator, "Transfer none (copy value)");
            
            // For primitive types, simple copy
            if (source_reg != target_reg) {
                emit_instruction(generator, INST_MOV, 2,
                               create_register_operand(target_reg),
                               create_register_operand(source_reg));
            }
            
            // For complex types, might need deep copy
            // For now, treat as simple copy
            return true;
            
        case FFI_OWNERSHIP_BORROW:
            // #[borrowed] - Temporary reference (caller retains ownership)
            emit_comment(generator, "Borrow reference (temporary)");
            
            // For borrowed references, typically pass pointer
            if (source_reg != target_reg) {
                emit_instruction(generator, INST_MOV, 2,
                               create_register_operand(target_reg),
                               create_register_operand(source_reg));
            }
            
            return true;
            
        case FFI_OWNERSHIP_NONE:
        default:
            // No annotation - use default behavior
            emit_comment(generator, "No FFI annotation - default transfer");
            
            if (source_reg != target_reg) {
                emit_instruction(generator, INST_MOV, 2,
                               create_register_operand(target_reg),
                               create_register_operand(source_reg));
            }
            
            return true;
    }
}

// =============================================================================
// ANNOTATED RETURN VALUE HANDLING
// =============================================================================

bool ffi_generate_annotated_return(FFIAssemblyGenerator *generator,
                                  ASTNode *return_expr, Register result_reg) {
    if (!generator || !return_expr) {
        return false;
    }
    
    // Extract FFI annotation from return expression context
    // Note: In practice, return annotations are on the function declaration
    FFIOwnershipTransferType annotation_type = FFI_OWNERSHIP_NONE;
    
    // For return values, we need to look at the function context
    // This would require parent node traversal in a real implementation
    // For now, we'll use a simplified approach
    
    emit_comment(generator, "Generate annotated return value");
    
    // Generate the expression value
    if (!code_generate_expression(generator->base_generator, return_expr, result_reg)) {
        return false;
    }
    
    // Apply annotation-based transfer semantics
    // Return values typically use RAX register in System V AMD64 ABI
    Register return_reg = ASTHRA_REG_RAX;
    
    return ffi_generate_annotation_based_transfer(generator, result_reg, return_reg, annotation_type);
}

// =============================================================================
// ANNOTATED PARAMETER HANDLING
// =============================================================================

bool ffi_generate_annotated_parameter(FFIAssemblyGenerator *generator,
                                     ASTNode *param_node, Register value_reg,
                                     Register target_reg) {
    if (!generator || !param_node) {
        return false;
    }
    
    // Extract FFI annotation from parameter
    FFIOwnershipTransferType annotation_type;
    bool has_annotation = ffi_extract_annotation(generator, param_node, &annotation_type);
    
    if (!has_annotation) {
        // No FFI annotation - use default parameter passing
        emit_comment(generator, "Parameter without FFI annotation");
        annotation_type = FFI_OWNERSHIP_NONE;
    }
    
    // Generate parameter marshaling based on annotation
    return ffi_generate_annotation_based_transfer(generator, value_reg, target_reg, annotation_type);
}

// =============================================================================
// ENHANCED FUNCTION AND EXTERN GENERATION
// =============================================================================

bool ffi_generate_annotated_function_call(FFIAssemblyGenerator *generator, ASTNode *call_expr) {
    if (!generator || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }
    
    emit_comment(generator, "FFI function call with annotation support");
    
    // Extract function and arguments
    ASTNode *function = call_expr->data.call_expr.function;
    ASTNodeList *args = call_expr->data.call_expr.args;
    
    if (!function || function->type != AST_IDENTIFIER) {
        return false;
    }
    
    const char *function_name = function->data.identifier.name;
    size_t arg_count = args ? args->count : 0;
    
    // Generate parameters with annotation support
    Register param_registers[] = {ASTHRA_REG_RDI, ASTHRA_REG_RSI, ASTHRA_REG_RDX, ASTHRA_REG_RCX, ASTHRA_REG_R8, ASTHRA_REG_R9};
    size_t max_register_params = sizeof(param_registers) / sizeof(param_registers[0]);
    
    for (size_t i = 0; i < arg_count && i < max_register_params; i++) {
        ASTNode *arg = args->nodes[i];
        
        // Generate argument value
        Register arg_reg = register_allocate(generator->base_generator->register_allocator, true);
        if (arg_reg == ASTHRA_REG_NONE) {
            return false;
        }
        
        if (!code_generate_expression(generator->base_generator, arg, arg_reg)) {
            register_free(generator->base_generator->register_allocator, arg_reg);
            return false;
        }
        
        // Note: In a complete implementation, we would need parameter declaration
        // information to extract FFI annotations. For now, use default transfer.
        FFIOwnershipTransferType transfer_type = FFI_OWNERSHIP_NONE;
        
        // Apply annotation-based transfer to target register
        if (!ffi_generate_annotation_based_transfer(generator, arg_reg, param_registers[i], transfer_type)) {
            register_free(generator->base_generator->register_allocator, arg_reg);
            return false;
        }
        
        register_free(generator->base_generator->register_allocator, arg_reg);
    }
    
    // Generate the actual function call
    emit_instruction(generator, INST_CALL, 1,
                    create_label_operand(function_name));
    
    // Handle return value with annotation support
    if (call_expr->type_info && !is_void_type(call_expr->type_info)) {
        // Return value is in RAX - extract annotation from function declaration
        // For now, use default handling
        emit_comment(generator, "Return value received in RAX");
    }
    
    return true;
}

// =============================================================================
// VALIDATION AND ERROR HANDLING
// =============================================================================

bool ffi_validate_annotation_usage(FFIAssemblyGenerator *generator, ASTNode *node) {
    if (!generator || !node) {
        return false;
    }
    
    // This function validates that FFI annotations are used correctly
    // during code generation phase as an additional safety check
    
    ASTNodeList *annotations = get_node_annotations(node);
    if (!annotations) {
        return true; // No annotations to validate
    }
    
    size_t ffi_annotation_count = 0;
    const char *found_ffi_annotation = NULL;
    
    // Count FFI annotations (should be at most 1 due to SafeFFIAnnotation grammar)
    for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
        ASTNode *annotation = ast_node_list_get(annotations, i);
        if (!annotation || annotation->type != AST_SEMANTIC_TAG) {
            continue;
        }
        
        const char *annotation_name = annotation->data.semantic_tag.name;
        if (!annotation_name) {
            continue;
        }
        
        if (strcmp(annotation_name, "transfer_full") == 0 ||
            strcmp(annotation_name, "transfer_none") == 0 ||
            strcmp(annotation_name, "borrowed") == 0) {
            ffi_annotation_count++;
            found_ffi_annotation = annotation_name;
        }
    }
    
    // SafeFFIAnnotation grammar should prevent this, but validate for safety
    if (ffi_annotation_count > 1) {
        emit_comment(generator, "ERROR: Multiple FFI annotations detected (grammar violation)");
        return false;
    }
    
    if (ffi_annotation_count == 1) {
        char comment_buffer[256];
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "Validated single FFI annotation: %s", found_ffi_annotation);
        emit_comment(generator, comment_buffer);
    }
    
    return true;
} 
