/**
 * Asthra Programming Language Compiler
 * Code Generation Core Interface
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_CORE_H
#define ASTHRA_CODE_GENERATOR_CORE_H

#include "code_generator_types.h"
#include "code_generator_instructions.h"
#include "code_generator_registers.h"
#include "code_generator_labels.h"
#include "code_generator_symbols.h"
#include "code_generator_abi.h"
#include "code_generator_optimization.h"
#include "generic_instantiation.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MAIN CODE GENERATOR STRUCTURE
// =============================================================================

struct CodeGenerator {
    // Target configuration
    TargetArchitecture target_arch;
    CallingConvention calling_conv;
    bool generate_debug_info;
    bool optimize_code;
    
    // Core components
    InstructionBuffer *instruction_buffer;
    RegisterAllocator *register_allocator;
    LabelManager *label_manager;
    SemanticAnalyzer *semantic_analyzer;  // Reference to semantic analysis results
    LocalSymbolTable *symbol_table;  // Local variable symbol table
    
    // Phase 4: Generic struct monomorphization registry
    GenericRegistry *generic_registry;
    
    // Code generation state
    size_t current_function_stack_size;
    size_t current_function_param_size;
    char *current_function_name;
    
    // Loop context for break/continue statements
    struct {
        char *exit_label;      // Label to jump to for break
        char *continue_label;  // Label to jump to for continue
        size_t depth;         // Nesting depth
    } loop_context;
    
    // Statistics
    CodeGenStatistics stats;
    
    // Configuration
    struct {
        bool emit_comments;
        bool position_independent_code;
        bool stack_protection;
        bool bounds_checking;
        size_t optimization_level;
    } config;
};

// =============================================================================
// CODE GENERATOR INTERFACE
// =============================================================================

// Creation and destruction
CodeGenerator *code_generator_create(TargetArchitecture arch, CallingConvention conv);
void code_generator_destroy(CodeGenerator *generator);
void code_generator_reset(CodeGenerator *generator);

// Semantic analyzer integration
void code_generator_set_semantic_analyzer(CodeGenerator *generator, SemanticAnalyzer *analyzer);

// Main code generation entry points
bool code_generate_program(CodeGenerator *generator, ASTNode *program);
bool code_generate_function(CodeGenerator *generator, ASTNode *function_decl);
bool code_generate_statement(CodeGenerator *generator, ASTNode *stmt);
bool code_generate_expression(CodeGenerator *generator, ASTNode *expr, Register target_reg);

// Associated function call code generation
bool code_generate_associated_function_call(CodeGenerator *generator, ASTNode *call_expr, Register target_reg);

// Regular function call code generation
bool code_generate_function_call(CodeGenerator *generator, ASTNode *call_expr, Register target_reg);

// Enum variant construction code generation
bool code_generate_enum_variant_construction(CodeGenerator *generator, ASTNode *expr, Register target_reg);

// Enum declaration code generation (Phase 2)
bool code_generate_enum_declaration(CodeGenerator *generator, ASTNode *enum_decl);

// Package system code generation


// Method and impl block code generation
bool code_generate_impl_block(CodeGenerator *generator, ASTNode *impl_block);
bool code_generate_method(CodeGenerator *generator, ASTNode *method_decl, const char *struct_name);

// Assembly output with C17 restrict pointers
bool code_generator_emit_assembly(CodeGenerator *generator, 
                                 char * restrict output_buffer, 
                                 size_t buffer_size);
bool code_generator_write_assembly_file(CodeGenerator *generator, 
                                       const char * restrict filename);

// Architecture-specific assembly formatting
int code_generator_format_instruction(const CodeGenerator *generator, 
                                     const AssemblyInstruction *inst,
                                     char *buffer, size_t buffer_size);
static int format_x86_64_instruction(const AssemblyInstruction *inst, 
                                    char *buffer, size_t buffer_size);
static int format_aarch64_instruction(const AssemblyInstruction *inst, 
                                     char *buffer, size_t buffer_size);
static const char* format_x86_64_operand(const AssemblyOperand *operand);
static const char* format_aarch64_operand(const AssemblyOperand *operand);
static const char* get_x86_64_register_name(Register reg);
static const char* get_aarch64_register_name(Register reg);

// New function declarations
bool code_generate_lvalue_address(CodeGenerator *generator, ASTNode *lvalue, Register addr_reg);
int compute_field_offset(CodeGenerator *generator, ASTNode *base_obj, const char *field_name);

// If-let statement code generation
bool code_generate_if_let_statement(CodeGenerator *generator, ASTNode *stmt);

// Match statement code generation
bool code_generate_match_statement(CodeGenerator *generator, ASTNode *stmt);

bool code_generate_pattern_test(CodeGenerator *generator, ASTNode *pattern, Register value_reg, 
                               char *match_label, char *no_match_label);
bool code_generate_pattern_bindings(CodeGenerator *generator, ASTNode *pattern, Register value_reg);
bool code_generate_enum_pattern_test(CodeGenerator *generator, ASTNode *pattern, Register value_reg,
                                    char *match_label, char *no_match_label);
bool code_generate_struct_pattern_test(CodeGenerator *generator, ASTNode *pattern, Register value_reg,
                                      char *match_label, char *no_match_label);
bool code_generate_enum_pattern_bindings(CodeGenerator *generator, ASTNode *pattern, Register value_reg);
bool code_generate_struct_pattern_bindings(CodeGenerator *generator, ASTNode *pattern, Register value_reg);
uint32_t simple_string_hash(const char *str);

// Unsafe block code generation
bool code_generate_unsafe_block(CodeGenerator *generator, ASTNode *stmt);

// =============================================================================
// PHASE 4: GENERIC STRUCT CODE GENERATION
// =============================================================================

// Register a generic struct for monomorphization
bool code_generator_register_generic_struct(CodeGenerator *generator, 
                                           const char *struct_name,
                                           ASTNode *struct_decl,
                                           TypeDescriptor *struct_type);

// Generate code for a generic struct instantiation
bool code_generate_generic_struct_instantiation(CodeGenerator *generator,
                                               const char *struct_name,
                                               TypeDescriptor **type_args,
                                               size_t type_arg_count);

// Generate all instantiated struct definitions
bool code_generate_all_generic_instantiations(CodeGenerator *generator,
                                             char *output_buffer,
                                             size_t buffer_size);

// Generate code for struct literals with type arguments
bool code_generate_generic_struct_literal(CodeGenerator *generator,
                                         ASTNode *struct_literal,
                                         Register target_reg);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_CORE_H 
