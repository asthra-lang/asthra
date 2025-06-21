/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Internal Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_FFI_ASSEMBLY_INTERNAL_H
#define ASTHRA_FFI_ASSEMBLY_INTERNAL_H

#include "ffi_assembly_generator.h"
#include "code_generator.h"
#include <stdatomic.h>

#ifdef __cplusplus
extern "C" {
#endif

// Define missing type constants
#define TYPE_VOID    0
#define TYPE_FLOAT   1
#define TYPE_DOUBLE  2
#define TYPE_USIZE   15  // Add missing USIZE type
#define TYPE_POINTER 16  // Add missing POINTER type

// Include runtime header for canonical type constants
#include "../../runtime/core/asthra_runtime_core.h"

// Add missing register definitions
#define REG_AL REG_RAX  // Use RAX for AL register access

// =============================================================================
// CORE FUNCTIONS (ffi_assembly_core.c)
// =============================================================================

/**
 * Generates a unique label for assembly code
 */
char *generate_unique_label(FFIAssemblyGenerator *generator, const char *prefix);

/**
 * Emits an assembly instruction
 */
bool emit_instruction(FFIAssemblyGenerator *generator, InstructionType type, 
                      size_t operand_count, ...);

/**
 * Emits a comment in the assembly output
 */
void emit_comment(FFIAssemblyGenerator *generator, const char *comment);

/**
 * Emits a label in the assembly output
 */
void emit_label(FFIAssemblyGenerator *generator, const char *label);

/**
 * Generates function declaration code
 */
bool ffi_generate_function_declaration(FFIAssemblyGenerator *generator, ASTNode *decl);

/**
 * Generates extern declaration code
 */
bool ffi_generate_extern_declaration(FFIAssemblyGenerator *generator, ASTNode *decl);

/**
 * Generates struct declaration code
 */
bool ffi_generate_struct_declaration(FFIAssemblyGenerator *generator, ASTNode *decl);

/**
 * Validates the generated assembly for correctness
 */
bool ffi_validate_generated_assembly(FFIAssemblyGenerator *generator);

/**
 * Retrieves generation statistics
 */
void ffi_get_generation_statistics(FFIAssemblyGenerator *generator,
                                  size_t *ffi_calls, size_t *pattern_matches,
                                  size_t *string_ops, size_t *slice_ops,
                                  size_t *security_ops, size_t *spawn_stmts);

/**
 * Prints the generated assembly in NASM format
 */
bool ffi_print_nasm_assembly(FFIAssemblyGenerator *generator,
                            char * restrict output_buffer, size_t buffer_size);

// =============================================================================
// FFI CALL FUNCTIONS (ffi_assembly_calls_core.c, ffi_assembly_marshaling*.c)
// =============================================================================

/**
 * Determines marshaling type for a parameter
 */
FFIMarshalingType ffi_determine_marshaling_type(FFIAssemblyGenerator *generator, ASTNode *arg);

/**
 * Determines marshaling type for a return value
 */
FFIMarshalingType ffi_determine_return_marshaling_type(FFIAssemblyGenerator *generator, ASTNode *call);

/**
 * Marshals a slice parameter for FFI
 */
bool ffi_marshal_slice_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);

/**
 * Marshals a string parameter for FFI
 */
bool ffi_marshal_string_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);

/**
 * Marshals a Result parameter for FFI
 */
bool ffi_marshal_result_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);

/**
 * Marshals an Option parameter for FFI
 */
bool ffi_marshal_option_parameter(FFIAssemblyGenerator *generator, ASTNode *param, Register target_reg);

/**
 * Handles return value unmarshaling
 */
bool ffi_generate_return_unmarshaling(FFIAssemblyGenerator *generator,
                                    FFIMarshalingType marshal_type,
                                    FFIOwnershipTransferType transfer_type,
                                    Register source_reg, Register target_reg);

/**
 * Generates ownership transfer code
 */
bool ffi_generate_ownership_transfer(FFIAssemblyGenerator *generator,
                                   Register source_reg, Register target_reg,
                                   FFIOwnershipTransferType transfer_type);

// =============================================================================
// PHASE 4: FFI ANNOTATION HANDLING FUNCTIONS (ffi_assembly_annotations.c)
// =============================================================================

/**
 * Extract FFI annotation from AST node
 */
bool ffi_extract_annotation(FFIAssemblyGenerator *generator, ASTNode *node, FFIOwnershipTransferType *annotation_type);

/**
 * Generate ownership transfer code based on FFI annotation
 */
bool ffi_generate_annotation_based_transfer(FFIAssemblyGenerator *generator,
                                           Register source_reg, Register target_reg,
                                           FFIOwnershipTransferType transfer_type);

/**
 * Generate return value handling with FFI annotation
 */
bool ffi_generate_annotated_return(FFIAssemblyGenerator *generator,
                                  ASTNode *return_expr, Register result_reg);

/**
 * Generate parameter marshaling with FFI annotation
 */
bool ffi_generate_annotated_parameter(FFIAssemblyGenerator *generator,
                                     ASTNode *param_node, Register value_reg,
                                     Register target_reg);

/**
 * Generate annotated function call
 */
bool ffi_generate_annotated_function_call(FFIAssemblyGenerator *generator, ASTNode *call_expr);

/**
 * Validate FFI annotation usage during code generation
 */
bool ffi_validate_annotation_usage(FFIAssemblyGenerator *generator, ASTNode *node);

/**
 * Generates extern call code
 */
bool ffi_generate_extern_call(FFIAssemblyGenerator *generator, ASTNode *call_expr);

/**
 * Generates variadic call code
 */
bool ffi_generate_variadic_call(FFIAssemblyGenerator *generator, 
                               ASTNode *call_expr, size_t fixed_args);

// =============================================================================
// PATTERN MATCHING FUNCTIONS (ffi_assembly_pattern.c)
// =============================================================================

/**
 * Determines pattern matching strategy based on context
 */
PatternMatchStrategy ffi_determine_pattern_strategy(FFIAssemblyGenerator *generator, 
                                                 PatternMatchContext *context);

/**
 * Generates linear pattern matching code
 */
bool ffi_generate_linear_pattern_match(FFIAssemblyGenerator *generator, 
                                    PatternMatchContext *context);

/**
 * Generates jump table pattern matching code
 */
bool ffi_generate_pattern_jump_table(FFIAssemblyGenerator *generator,
                                    PatternMatchContext *context);

/**
 * Generates if-let statement pattern matching
 */
bool ffi_generate_if_let_statement(FFIAssemblyGenerator *generator, ASTNode *if_let_stmt);

/**
 * Generates Result<T,E> construction
 */
bool ffi_generate_result_construction(FFIAssemblyGenerator *generator,
                                    bool is_ok, Register value_reg,
                                    TypeInfo *result_type, Register result_reg);

// =============================================================================
// STRING OPERATION FUNCTIONS (ffi_assembly_string.c)
// =============================================================================

/**
 * Generates deterministic string operation code
 */
bool ffi_generate_deterministic_string_op(FFIAssemblyGenerator *generator,
                                         StringOperationType op_type,
                                         Register *operand_regs, size_t operand_count,
                                         Register result_reg);

/**
 * Generates postfix expression code
 */
bool ffi_generate_postfix_expression(FFIAssemblyGenerator *generator,
                                    ASTNode *postfix_expr, Register result_reg);

// =============================================================================
// SLICE OPERATION FUNCTIONS (ffi_assembly_slice.c)
// =============================================================================

/**
 * Generates slice creation code
 */
bool ffi_generate_slice_creation(FFIAssemblyGenerator *generator,
                                ASTNode *array_literal, Register result_reg);

// =============================================================================
// SECURITY FUNCTIONS (ffi_assembly_security.c)
// =============================================================================

/**
 * Generates secure operation code
 */
bool ffi_generate_secure_operation(FFIAssemblyGenerator *generator, 
                                ASTNode *operation, 
                                SecurityContext *context);

/**
 * Generates FFI security boundary code
 */
bool ffi_generate_ffi_security_boundary(FFIAssemblyGenerator *generator,
                                       ASTNode *extern_call, bool entering_ffi);

/**
 * Validates security annotations at FFI boundary
 */
bool ffi_validate_security_annotations_at_boundary(FFIAssemblyGenerator *generator,
                                                  ASTNode *extern_decl);

/**
 * Optimizes zero-cost abstractions
 */
bool ffi_optimize_zero_cost_abstractions(FFIAssemblyGenerator *generator);

/**
 * Generates GC barriers for unsafe blocks
 */
bool ffi_generate_gc_barriers(FFIAssemblyGenerator *generator,
                             bool entering_unsafe, bool exiting_unsafe);

// =============================================================================
// CONCURRENCY FUNCTIONS (ffi_assembly_concurrency.c)
// =============================================================================

/**
 * Generates channel send operation code
 */
bool ffi_generate_channel_send(FFIAssemblyGenerator *generator, 
                              Register channel_reg, Register value_reg);

/**
 * Generates channel receive operation code
 */
bool ffi_generate_channel_recv(FFIAssemblyGenerator *generator, 
                              Register channel_reg, Register result_reg, 
                              ASTNode *timeout_expr);

// =============================================================================
// EXPRESSION HANDLING FUNCTIONS
// =============================================================================

/**
 * Generates expression code
 */
bool ffi_generate_expression(FFIAssemblyGenerator *generator, ASTNode *expr, Register result_reg);

/**
 * Alternative implementation for expression generation to avoid conflicts
 */
bool ffi_generate_expression_impl(FFIAssemblyGenerator *generator, ASTNode *expr, Register result_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_FFI_ASSEMBLY_INTERNAL_H 
