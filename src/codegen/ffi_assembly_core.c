/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Core Functionality
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_assembly_generator.h"
#include "ffi_assembly_internal.h"
#include "ffi_assembly_const.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>

// TODO: Instruction buffer optimization will be implemented in future version

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

char *generate_unique_label(FFIAssemblyGenerator *generator, const char *prefix) {
    (void)generator; // Mark as unused to suppress warning
    static atomic_uint_fast32_t label_counter = 0;
    uint32_t id = atomic_fetch_add(&label_counter, 1);
    
    size_t len = strlen(prefix) + 32;
    char *label = malloc(len);
    if (!label) return NULL;
    
    snprintf(label, len, "%s_%u", prefix, id);
    return label;
}

bool emit_instruction(FFIAssemblyGenerator *generator, InstructionType type, 
                           size_t operand_count, ...) {
    va_list args;
    va_start(args, operand_count);
    
    // Allocate instruction with flexible array member - same as create_instruction
    size_t size = sizeof(AssemblyInstruction) + operand_count * sizeof(AssemblyOperand);
    AssemblyInstruction *inst = malloc(size);
    if (!inst) {
        va_end(args);
        return false;
    }
    
    inst->type = type;
    inst->operand_count = operand_count;
    inst->comment = NULL;
    
    // Process variadic arguments for operands
    for (size_t i = 0; i < operand_count; i++) {
        inst->operands[i] = va_arg(args, AssemblyOperand);
    }
    
    va_end(args);
    
    bool result = instruction_buffer_add(generator->base_generator->instruction_buffer, inst);
    return result;
}

void emit_comment(FFIAssemblyGenerator *generator, const char *comment) {
    if (!generator || !comment) return;
    
    // Create comment instruction using existing create_instruction
    AssemblyInstruction *inst = create_instruction(INST_MOV, 0); // Use MOV as placeholder
    if (!inst) return;
    
    // Override with comment
    inst->comment = strdup(comment);
    
    instruction_buffer_add(generator->base_generator->instruction_buffer, inst);
}

void emit_label(FFIAssemblyGenerator *generator, const char *label) {
    if (!generator || !label) return;
    
    // Create a label entry in the label manager
    if (generator->base_generator->label_manager) {
        label_manager_define_label(generator->base_generator->label_manager, 
                                 label, 
                                 generator->base_generator->instruction_buffer->count);
    }
}

// =============================================================================
// CORE FFI ASSEMBLY GENERATOR FUNCTIONS
// =============================================================================

FFIAssemblyGenerator *ffi_assembly_generator_create(TargetArchitecture arch, 
                                                   CallingConvention conv) {
    FFIAssemblyGenerator *generator = calloc(1, sizeof(FFIAssemblyGenerator));
    if (!generator) return NULL;
    
    // Create base code generator
    generator->base_generator = code_generator_create(arch, conv);
    if (!generator->base_generator) {
        free(generator);
        return NULL;
    }
    
    // Initialize configuration with defaults
    generator->config.enable_bounds_checking = true;
    generator->config.enable_security_features = true;
    generator->config.enable_concurrency = true;
    generator->config.optimize_string_operations = true;
    generator->config.optimize_pattern_matching = true;
    generator->config.max_variadic_args = 32;
    generator->config.pic_mode = false;
    
    // Initialize runtime function names
    generator->runtime_functions.gc_alloc = strdup("asthra_gc_alloc");
    generator->runtime_functions.gc_free = strdup("asthra_gc_free");
    generator->runtime_functions.slice_bounds_check = strdup("asthra_slice_bounds_check");
    generator->runtime_functions.string_concat = strdup("asthra_string_concat");
    generator->runtime_functions.string_interpolate = strdup("asthra_string_interpolate");
    generator->runtime_functions.result_create_ok = strdup("asthra_result_create_ok");
    generator->runtime_functions.result_create_err = strdup("asthra_result_create_err");
    generator->runtime_functions.spawn_task = strdup("asthra_spawn_task");
    generator->runtime_functions.secure_zero = strdup("asthra_secure_zero");
    
    // Initialize atomic statistics
    atomic_init(&generator->ffi_calls_generated, 0);
    atomic_init(&generator->pattern_matches_generated, 0);
    atomic_init(&generator->string_operations_generated, 0);
    atomic_init(&generator->slice_operations_generated, 0);
    atomic_init(&generator->security_operations_generated, 0);
    atomic_init(&generator->spawn_statements_generated, 0);
    
    return generator;
}

void ffi_assembly_generator_destroy(FFIAssemblyGenerator *generator) {
    if (!generator) return;
    
    // Destroy base generator
    if (generator->base_generator) {
        code_generator_destroy(generator->base_generator);
    }
    
    // Free runtime function names
    free(generator->runtime_functions.gc_alloc);
    free(generator->runtime_functions.gc_free);
    free(generator->runtime_functions.slice_bounds_check);
    free(generator->runtime_functions.string_concat);
    free(generator->runtime_functions.string_interpolate);
    free(generator->runtime_functions.result_create_ok);
    free(generator->runtime_functions.result_create_err);
    free(generator->runtime_functions.spawn_task);
    free(generator->runtime_functions.secure_zero);
    
    // Free contexts if allocated
    free(generator->current_ffi_call);
    free(generator->current_pattern_match);
    free(generator->current_string_op);
    free(generator->current_slice_op);
    free(generator->current_security_op);
    free(generator->current_concurrency_op);
    
    free(generator);
}

bool ffi_generate_program(FFIAssemblyGenerator *generator, ASTNode *program) {
    if (!generator || !program || program->type != AST_PROGRAM) {
        return false;
    }
    
    emit_comment(generator, "Generated by Asthra FFI Assembly Generator");
    emit_comment(generator, "Target: System V AMD64 ABI with C17 compatibility");
    
    // Generate assembly directives
    if (generator->config.pic_mode) {
        emit_comment(generator, "Position Independent Code enabled");
    }
    
    // Process all top-level declarations
    ASTNodeList *declarations = program->data.program.declarations;
    for (size_t i = 0; i < declarations->count; i++) {
        ASTNode *decl = declarations->nodes[i];
        
        switch (decl->type) {
            case AST_FUNCTION_DECL:
                if (!ffi_generate_function_declaration(generator, decl)) {
                    return false;
                }
                break;
                
            case AST_EXTERN_DECL:
                if (!ffi_generate_extern_declaration(generator, decl)) {
                    return false;
                }
                break;
                
            case AST_STRUCT_DECL:
                if (!ffi_generate_struct_declaration(generator, decl)) {
                    return false;
                }
                break;
                
            case AST_CONST_DECL:
                if (!ffi_generate_const_decl(generator, decl)) {
                    return false;
                }
                break;
                
            default:
                // Skip other declaration types
                break;
        }
    }
    
    return true;
}

// =============================================================================
// DECLARATIONS GENERATION
// =============================================================================

bool ffi_generate_extern_declaration(FFIAssemblyGenerator *generator, ASTNode *decl) {
    if (!generator || !decl || decl->type != AST_EXTERN_DECL) return false;
    
    emit_comment(generator, "External function declaration with FFI annotation support");
    
    // Extract extern declaration information
    const char *function_name = decl->data.extern_decl.name;
    const char *extern_name = decl->data.extern_decl.extern_name;
    
    if (!function_name) return false;
    
    // Phase 4: FFI Annotation Ambiguity Fix - Validate annotation usage
    if (!ffi_validate_annotation_usage(generator, decl)) {
        emit_comment(generator, "ERROR: Invalid FFI annotation usage in extern declaration");
        return false;
    }
    
    // Extract FFI annotation for return type handling
    FFIOwnershipTransferType return_annotation;
    bool has_return_annotation = ffi_extract_annotation(generator, decl, &return_annotation);
    
    // Generate external symbol reference with annotation information
    char comment_buffer[512];
    if (extern_name && has_return_annotation) {
        const char *annotation_name = (return_annotation == FFI_OWNERSHIP_MOVE) ? "transfer_full" :
                                    (return_annotation == FFI_OWNERSHIP_COPY) ? "transfer_none" :
                                    (return_annotation == FFI_OWNERSHIP_BORROW) ? "borrowed" : "none";
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "External function: %s from %s (return: %s)", function_name, extern_name, annotation_name);
    } else if (extern_name) {
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "External function: %s from %s (no FFI annotation)", function_name, extern_name);
    } else if (has_return_annotation) {
        const char *annotation_name = (return_annotation == FFI_OWNERSHIP_MOVE) ? "transfer_full" :
                                    (return_annotation == FFI_OWNERSHIP_COPY) ? "transfer_none" :
                                    (return_annotation == FFI_OWNERSHIP_BORROW) ? "borrowed" : "none";
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "External function: %s (return: %s)", function_name, annotation_name);
    } else {
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "External function: %s (no FFI annotation)", function_name);
    }
    emit_comment(generator, comment_buffer);
    
    // For extern declarations, we don't generate code, just metadata
    // The actual linking is handled by the linker
    
    return true;
}

bool ffi_generate_function_declaration(FFIAssemblyGenerator *generator, ASTNode *decl) {
    if (!generator || !decl || decl->type != AST_FUNCTION_DECL) return false;
    
    // Phase 4: FFI Annotation Ambiguity Fix - Validate annotation usage
    if (!ffi_validate_annotation_usage(generator, decl)) {
        emit_comment(generator, "ERROR: Invalid FFI annotation usage in function declaration");
        return false;
    }
    
    // Extract FFI annotation for return type handling
    FFIOwnershipTransferType return_annotation;
    bool has_return_annotation = ffi_extract_annotation(generator, decl, &return_annotation);
    
    if (has_return_annotation) {
        const char *function_name = decl->data.function_decl.name;
        const char *annotation_name = (return_annotation == FFI_OWNERSHIP_MOVE) ? "transfer_full" :
                                    (return_annotation == FFI_OWNERSHIP_COPY) ? "transfer_none" :
                                    (return_annotation == FFI_OWNERSHIP_BORROW) ? "borrowed" : "none";
        
        char comment_buffer[256];
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "Function %s with FFI return annotation: %s", 
                function_name ? function_name : "(anonymous)", annotation_name);
        emit_comment(generator, comment_buffer);
    }
    
    // Generate function using base code generator with FFI annotation support
    return code_generate_function(generator->base_generator, decl);
}

bool ffi_generate_struct_declaration(FFIAssemblyGenerator *generator, ASTNode *decl) {
    if (!generator || !decl || decl->type != AST_STRUCT_DECL) return false;
    
    emit_comment(generator, "Struct declaration");
    
    // For struct declarations, we don't generate assembly code
    // The struct layout is handled during semantic analysis
    // However, we can emit metadata for debugging
    
    const char *struct_name = decl->data.struct_decl.name;
    if (struct_name) {
        char comment_buffer[256];
        snprintf(comment_buffer, sizeof(comment_buffer), 
                "Struct: %s", struct_name);
        emit_comment(generator, comment_buffer);
    }
    
    return true;
}





/**
 * Generate C-compatible struct layouts
 * This function generates struct layout information for C-compatibility
 */
bool ffi_generate_c_struct_layout(FFIAssemblyGenerator *generator,
                                 ASTNode *struct_decl, bool packed) {
    if (!generator || !struct_decl || struct_decl->type != AST_STRUCT_DECL) {
        return false;
    }
    
    const char *struct_name = struct_decl->data.struct_decl.name;
    
    // Emit struct layout information comment
    char comment_buffer[256];
    snprintf(comment_buffer, sizeof(comment_buffer), 
            "C-compatible struct layout: %s%s", 
            struct_name ? struct_name : "(anonymous)",
            packed ? " (packed)" : "");
    emit_comment(generator, comment_buffer);
    
    // For actual layout, we'd calculate field offsets based on types
    // and alignment requirements, but for this implementation, we'll
    // just emit a placeholder
    
    if (packed) {
        emit_comment(generator, "  Packed struct - no alignment padding");
    } else {
        emit_comment(generator, "  Standard alignment rules applied");
    }
    
    // In a real implementation, we would:
    // 1. Iterate through all fields
    // 2. Calculate proper alignment for each field
    // 3. Calculate field offsets
    // 4. Generate metadata for the struct layout
    
    return true;
}

// =============================================================================
// UTILITY AND VALIDATION FUNCTIONS
// =============================================================================

bool ffi_validate_generated_assembly(FFIAssemblyGenerator *generator) {
    if (!generator) return false;
    
    // Validate the generated assembly for correctness
    return code_generator_validate_instructions(generator->base_generator);
}

void ffi_get_generation_statistics(FFIAssemblyGenerator *generator,
                                  size_t *ffi_calls, size_t *pattern_matches,
                                  size_t *string_ops, size_t *slice_ops,
                                  size_t *security_ops, size_t *spawn_stmts) {
    if (!generator) return;
    
    if (ffi_calls) *ffi_calls = atomic_load(&generator->ffi_calls_generated);
    if (pattern_matches) *pattern_matches = atomic_load(&generator->pattern_matches_generated);
    if (string_ops) *string_ops = atomic_load(&generator->string_operations_generated);
    if (slice_ops) *slice_ops = atomic_load(&generator->slice_operations_generated);
    if (security_ops) *security_ops = atomic_load(&generator->security_operations_generated);
    if (spawn_stmts) *spawn_stmts = atomic_load(&generator->spawn_statements_generated);
}

bool ffi_print_nasm_assembly(FFIAssemblyGenerator *generator,
                            char * restrict output_buffer, size_t buffer_size) {
    if (!generator || !output_buffer) return false;
    
    // Generate NASM-compatible assembly output
    return code_generator_emit_assembly(generator->base_generator, 
                                      output_buffer, buffer_size);
} 
