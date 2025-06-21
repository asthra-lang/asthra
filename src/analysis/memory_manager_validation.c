/**
 * Asthra Memory Manager - Ownership Validation Implementation
 * Semantic Analysis and Ownership Validation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_manager.h"
#include "../parser/ast.h"

// =============================================================================
// OWNERSHIP VALIDATION IMPLEMENTATION
// =============================================================================

OwnershipValidationResult validate_ownership_annotation(const ASTNode *node,
                                                       OwnershipValidation *validation) {
    if (!node || !validation) {
        return OWNERSHIP_INVALID_ANNOTATION;
    }
    
    validation->node = node;
    validation->location = node->location;
    
    // Check for ownership annotations
    ASTNodeList *annotations = NULL;
    switch (node->type) {
        case AST_STRUCT_DECL:
            annotations = node->data.struct_decl.annotations;
            break;
        case AST_FUNCTION_DECL:
            annotations = node->data.function_decl.annotations;
            break;
        case AST_EXTERN_DECL:
            annotations = node->data.extern_decl.annotations;
            break;
        case AST_LET_STMT:
            // Variable declarations can have ownership annotations
            // This would need to be implemented in the parser
            break;
        default:
            // No ownership annotations expected
            return OWNERSHIP_VALID;
    }
    
    if (!annotations) {
        // No annotations is valid (defaults apply)
        return OWNERSHIP_VALID;
    }
    
    // Validate ownership annotations
    bool has_ownership = false;
    OwnershipType ownership_type = OWNERSHIP_GC; // Default
    
    for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
        ASTNode *annotation = ast_node_list_get(annotations, i);
        if (!annotation) continue;
        
        if (annotation->type == AST_OWNERSHIP_TAG) {
            if (has_ownership) {
                snprintf(validation->error_message, sizeof(validation->error_message),
                        "Multiple ownership annotations found");
                return OWNERSHIP_INVALID_ANNOTATION;
            }
            has_ownership = true;
            ownership_type = annotation->data.ownership_tag.ownership;
        }
    }
    
    // Validate ownership type consistency
    if (node->type == AST_EXTERN_DECL && ownership_type == OWNERSHIP_GC) {
        snprintf(validation->error_message, sizeof(validation->error_message),
                "External functions cannot use GC ownership");
        return OWNERSHIP_INVALID_ANNOTATION;
    }
    
    return OWNERSHIP_VALID;
}

OwnershipValidationResult validate_ownership_transfer(const ASTNode *call_node,
                                                     const ASTNode *function_decl,
                                                     OwnershipValidation *validation) {
    if (!call_node || !function_decl || !validation) {
        return OWNERSHIP_INVALID_ANNOTATION;
    }
    
    validation->node = call_node;
    validation->location = call_node->location;
    validation->operation = "function_call";
    
    // Check FFI transfer annotations on function parameters
    if (function_decl->type == AST_EXTERN_DECL) {
        ASTNodeList *params = function_decl->data.extern_decl.params;
        ASTNodeList *args = call_node->data.call_expr.args;
        
        if (params && args) {
            size_t param_count = ast_node_list_size(params);
            size_t arg_count = ast_node_list_size(args);
            
            if (param_count != arg_count) {
                snprintf(validation->error_message, sizeof(validation->error_message),
                        "Argument count mismatch: expected %zu, got %zu",
                        param_count, arg_count);
                return OWNERSHIP_TRANSFER_VIOLATION;
            }
            
            // Validate each parameter's ownership transfer
            for (size_t i = 0; i < param_count; i++) {
                ASTNode *param = ast_node_list_get(params, i);
                ASTNode *arg = ast_node_list_get(args, i);
                
                if (!param || !arg) continue;
                
                // Check for transfer annotations on parameter
                ASTNodeList *param_annotations = param->data.param_decl.annotations;
                if (param_annotations) {
                    for (size_t j = 0; j < ast_node_list_size(param_annotations); j++) {
                        ASTNode *annotation = ast_node_list_get(param_annotations, j);
                        if (annotation && annotation->type == AST_FFI_ANNOTATION) {
                            FFITransferType transfer = annotation->data.ffi_annotation.transfer_type;
                            
                            // Validate transfer semantics
                            if (transfer == FFI_TRANSFER_FULL) {
                                // Full ownership transfer - argument must be owned
                                // This would require more sophisticated analysis
                            } else if (transfer == FFI_BORROWED) {
                                // Borrowed reference - argument must remain valid
                                // This would require lifetime analysis
                            }
                        }
                    }
                }
            }
        }
    }
    
    return OWNERSHIP_VALID;
}

OwnershipValidationResult validate_ffi_ownership(const ASTNode *extern_node,
                                                const ASTNode *call_node,
                                                OwnershipValidation *validation) {
    if (!extern_node || !validation) {
        return OWNERSHIP_INVALID_ANNOTATION;
    }
    
    validation->node = extern_node;
    validation->location = extern_node->location;
    validation->operation = "ffi_boundary";
    
    // Validate that extern functions have appropriate ownership annotations
    ASTNodeList *annotations = extern_node->data.extern_decl.annotations;
    if (annotations) {
        for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
            ASTNode *annotation = ast_node_list_get(annotations, i);
            if (annotation && annotation->type == AST_OWNERSHIP_TAG) {
                OwnershipType ownership = annotation->data.ownership_tag.ownership;
                
                if (ownership == OWNERSHIP_GC) {
                    snprintf(validation->error_message, sizeof(validation->error_message),
                            "External functions cannot use GC ownership");
                    return OWNERSHIP_FFI_BOUNDARY_ERROR;
                }
            }
        }
    }
    
    // Validate parameter and return type ownership
    ASTNodeList *params = extern_node->data.extern_decl.params;
    if (params) {
        for (size_t i = 0; i < ast_node_list_size(params); i++) {
            ASTNode *param = ast_node_list_get(params, i);
            if (!param) continue;
            
            // Check parameter type for ownership consistency
            ASTNode *param_type = param->data.param_decl.type;
            if (param_type && param_type->type == AST_PTR_TYPE) {
                // Pointer parameters should have transfer annotations
                ASTNodeList *param_annotations = param->data.param_decl.annotations;
                bool has_transfer = false;
                
                if (param_annotations) {
                    for (size_t j = 0; j < ast_node_list_size(param_annotations); j++) {
                        ASTNode *annotation = ast_node_list_get(param_annotations, j);
                        if (annotation && annotation->type == AST_FFI_ANNOTATION) {
                            has_transfer = true;
                            break;
                        }
                    }
                }
                
                if (!has_transfer) {
                    snprintf(validation->error_message, sizeof(validation->error_message),
                            "Pointer parameter %zu lacks transfer annotation", i);
                    return OWNERSHIP_FFI_BOUNDARY_ERROR;
                }
            }
        }
    }
    
    return OWNERSHIP_VALID;
}

OwnershipValidationResult validate_lifetime_annotations(const ASTNode *node,
                                                       OwnershipValidation *validation) {
    if (!node || !validation) {
        return OWNERSHIP_INVALID_ANNOTATION;
    }
    
    validation->node = node;
    validation->location = node->location;
    validation->operation = "lifetime_validation";
    
    // Check for borrowed references that outlive their source
    // This is a simplified implementation - full lifetime analysis would be more complex
    
    if (node->type == AST_CALL_EXPR) {
        ASTNode *function = node->data.call_expr.function;
        if (function && function->type == AST_IDENTIFIER) {
            // Check if this is a call to an extern function with borrowed parameters
            // This would require symbol table lookup to get the function declaration
        }
    }
    
    return OWNERSHIP_VALID;
}

OwnershipValidationResult check_ownership_violations(const ASTNode *expr,
                                                    OwnershipValidation *validation) {
    if (!expr || !validation) {
        return OWNERSHIP_INVALID_ANNOTATION;
    }
    
    validation->node = expr;
    validation->location = expr->location;
    validation->operation = "ownership_check";
    
    // Check for common ownership violations
    switch (expr->type) {
        case AST_ASSIGNMENT:
            // Check that assignment target is mutable
            if (!expr->data.assignment.target->flags.is_mutable) {
                snprintf(validation->error_message, sizeof(validation->error_message),
                        "Cannot assign to immutable value");
                return OWNERSHIP_TRANSFER_VIOLATION;
            }
            break;
            
        case AST_UNARY_EXPR:
            if (expr->data.unary_expr.operator == UNOP_DEREF) {
                // Check that dereferenced pointer is valid
                // This would require more sophisticated analysis
            }
            break;
            
        case AST_CALL_EXPR:
            // Check function call ownership transfer
            // This would require function signature analysis
            break;
            
        default:
            break;
    }
    
    return OWNERSHIP_VALID;
}

// =============================================================================
// SEMANTIC ANALYSIS INTEGRATION
// =============================================================================

bool extract_ownership_context(const ASTNode *node, OwnershipContext *context) {
    if (!node || !context) return false;
    
    // Initialize with defaults
    context->ownership_type = OWNERSHIP_GC;
    context->memory_zone = ASTHRA_ZONE_GC;
    context->is_mutable = false;
    context->is_borrowed = false;
    context->requires_cleanup = false;
    context->source_location = NULL;
    
    // Extract ownership from annotations
    ASTNodeList *annotations = NULL;
    switch (node->type) {
        case AST_STRUCT_DECL:
            annotations = node->data.struct_decl.annotations;
            break;
        case AST_FUNCTION_DECL:
            annotations = node->data.function_decl.annotations;
            break;
        case AST_EXTERN_DECL:
            annotations = node->data.extern_decl.annotations;
            context->ownership_type = OWNERSHIP_C; // Default for extern
            context->memory_zone = ASTHRA_ZONE_MANUAL;
            context->requires_cleanup = true;
            break;
        default:
            return true; // Use defaults
    }
    
    if (annotations) {
        for (size_t i = 0; i < ast_node_list_size(annotations); i++) {
            ASTNode *annotation = ast_node_list_get(annotations, i);
            if (!annotation) continue;
            
            if (annotation->type == AST_OWNERSHIP_TAG) {
                context->ownership_type = annotation->data.ownership_tag.ownership;
                context->memory_zone = ownership_to_memory_zone(context->ownership_type);
                context->requires_cleanup = (context->ownership_type == OWNERSHIP_C);
            } else if (annotation->type == AST_FFI_ANNOTATION) {
                if (annotation->data.ffi_annotation.transfer_type == FFI_BORROWED) {
                    context->is_borrowed = true;
                }
            }
        }
    }
    
    return true;
}

AsthraMemoryZone ownership_to_memory_zone(OwnershipType ownership) {
    switch (ownership) {
        case OWNERSHIP_GC:
            return ASTHRA_ZONE_GC;
        case OWNERSHIP_C:
            return ASTHRA_ZONE_MANUAL;
        case OWNERSHIP_PINNED:
            return ASTHRA_ZONE_PINNED;
        default:
            return ASTHRA_ZONE_GC;
    }
}

OwnershipType memory_zone_to_ownership(AsthraMemoryZone zone) {
    switch (zone) {
        case ASTHRA_ZONE_GC:
            return OWNERSHIP_GC;
        case ASTHRA_ZONE_MANUAL:
            return OWNERSHIP_C;
        case ASTHRA_ZONE_PINNED:
            return OWNERSHIP_PINNED;
        case ASTHRA_ZONE_STACK:
            return OWNERSHIP_GC; // Stack allocations are GC-like
        default:
            return OWNERSHIP_GC;
    }
}

bool validate_struct_ownership(const ASTNode *struct_decl, OwnershipValidation *validation) {
    if (!struct_decl || struct_decl->type != AST_STRUCT_DECL || !validation) {
        return false;
    }
    
    validation->node = struct_decl;
    validation->location = struct_decl->location;
    validation->operation = "struct_ownership_validation";
    
    // Extract struct ownership
    OwnershipContext struct_context;
    if (!extract_ownership_context(struct_decl, &struct_context)) {
        return false;
    }
    
    // Validate field ownership consistency
    ASTNodeList *fields = struct_decl->data.struct_decl.fields;
    if (fields) {
        for (size_t i = 0; i < ast_node_list_size(fields); i++) {
            ASTNode *field = ast_node_list_get(fields, i);
            if (!field) continue;
            
            // Check field type for ownership consistency
            ASTNode *field_type = field->data.param_decl.type;
            if (field_type && field_type->type == AST_PTR_TYPE) {
                // Pointer fields in GC structs should be carefully considered
                if (struct_context.ownership_type == OWNERSHIP_GC) {
                    // This might require additional validation
                }
            }
        }
    }
    
    return true;
}

bool validate_variable_ownership(const ASTNode *var_decl, OwnershipValidation *validation) {
    if (!var_decl || var_decl->type != AST_LET_STMT || !validation) {
        return false;
    }
    
    validation->node = var_decl;
    validation->location = var_decl->location;
    validation->operation = "variable_ownership_validation";
    
    // Check if variable has ownership annotations
    // This would need to be implemented in the parser to support
    // ownership annotations on variable declarations
    
    return true;
}

bool validate_parameter_ownership(const ASTNode *param_list, OwnershipValidation *validation) {
    if (!param_list || !validation) {
        return false;
    }
    
    validation->operation = "parameter_ownership_validation";
    
    // This would validate ownership annotations on function parameters
    // Implementation depends on the specific parameter list structure
    
    return true;
}

const char *ownership_validation_error_message(OwnershipValidationResult result) {
    switch (result) {
        case OWNERSHIP_VALID:
            return "No ownership errors";
        case OWNERSHIP_INVALID_ANNOTATION:
            return "Invalid ownership annotation";
        case OWNERSHIP_ZONE_MISMATCH:
            return "Memory zone mismatch";
        case OWNERSHIP_TRANSFER_VIOLATION:
            return "Ownership transfer violation";
        case OWNERSHIP_LIFETIME_VIOLATION:
            return "Lifetime violation";
        case OWNERSHIP_FFI_BOUNDARY_ERROR:
            return "FFI boundary ownership error";
        case OWNERSHIP_CIRCULAR_REFERENCE:
            return "Circular reference detected";
        case OWNERSHIP_DOUBLE_FREE:
            return "Double free detected";
        case OWNERSHIP_USE_AFTER_FREE:
            return "Use after free detected";
        default:
            return "Unknown ownership error";
    }
}

void format_ownership_context(const OwnershipContext *context, 
                             char *buffer, 
                             size_t buffer_size) {
    if (!context || !buffer || buffer_size == 0) return;
    
    const char *ownership_names[] = {"GC", "C", "Pinned"};
    const char *zone_names[] = {"GC", "Manual", "Pinned", "Stack"};
    
    snprintf(buffer, buffer_size,
             "Ownership: %s, Zone: %s, Mutable: %s, Borrowed: %s, Cleanup: %s",
             ownership_names[context->ownership_type],
             zone_names[context->memory_zone],
             context->is_mutable ? "yes" : "no",
             context->is_borrowed ? "yes" : "no",
             context->requires_cleanup ? "yes" : "no");
} 
