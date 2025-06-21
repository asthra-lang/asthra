#ifndef ASTHRA_SMART_CODEGEN_H
#define ASTHRA_SMART_CODEGEN_H

#include "optimization_analysis.h"
#include "../parser/ast_node.h"
#include <stdio.h>
#include <stdbool.h>

/**
 * Asthra Programming Language - Smart Code Generation Engine
 * 
 * Phase 3: Code Generation & Runtime Optimization
 * 
 * This module implements the smart code generation system that automatically
 * applies optimization patterns detected by the analysis engine to generate
 * efficient C code from simple AI-friendly value semantics.
 */

// Smart code generation context
typedef struct {
    FILE *output;                     // Output file for generated C code
    OptimizationContext *opt_context; // Current optimization context
    int indent_level;                 // Current indentation level
    bool debug_mode;                  // Generate debug comments
    bool generate_stats;              // Generate performance statistics
    
    // Generation counters
    int optimizations_applied;
    int copies_eliminated;
    size_t memory_traffic_saved;
} SmartCodegen;

// Code generation strategies
typedef enum {
    CODEGEN_STRATEGY_NAIVE,          // Simple value semantics (no optimization)
    CODEGEN_STRATEGY_OPTIMIZED,      // Apply detected optimizations
    CODEGEN_STRATEGY_AGGRESSIVE      // Apply all possible optimizations
} CodegenStrategy;

// =============================================================================
// SMART CODE GENERATION FUNCTIONS
// =============================================================================

/**
 * Create smart code generation context
 */
SmartCodegen *smart_codegen_create(FILE *output);

/**
 * Destroy smart code generation context
 */
void smart_codegen_destroy(SmartCodegen *codegen);

/**
 * Generate optimized C code for an AST node
 * Automatically detects and applies optimization patterns
 */
bool smart_codegen_generate_node(SmartCodegen *codegen, ASTNode *node);

/**
 * Generate optimized assignment statement
 * Applies self-mutation and call-chain optimizations
 */
bool smart_codegen_generate_assignment(SmartCodegen *codegen, ASTNode *assignment);

/**
 * Generate optimized function call
 * Applies RVO and in-place optimizations
 */
bool smart_codegen_generate_function_call(SmartCodegen *codegen, ASTNode *call_expr);

/**
 * Generate optimized variable declaration
 * Handles const/mutable semantics
 */
bool smart_codegen_generate_variable_decl(SmartCodegen *codegen, ASTNode *let_stmt);

// =============================================================================
// OPTIMIZATION APPLICATION FUNCTIONS
// =============================================================================

/**
 * Apply self-mutation optimization: x = f(x) -> f_inplace(&x)
 */
bool apply_self_mutation_optimization(SmartCodegen *codegen, ASTNode *assignment);

/**
 * Apply call-chain optimization: x = f1(f2(f3(x))) -> f3_inplace(&x); f2_inplace(&x); f1_inplace(&x);
 */
bool apply_call_chain_optimization(SmartCodegen *codegen, ASTNode *assignment);

/**
 * Apply Return Value Optimization (RVO)
 * Generate function that constructs directly into return slot
 */
bool apply_rvo_optimization(SmartCodegen *codegen, ASTNode *function_decl);

/**
 * Apply copy elimination for large type assignments
 */
bool apply_copy_elimination(SmartCodegen *codegen, ASTNode *assignment);

// =============================================================================
// C CODE GENERATION HELPERS
// =============================================================================

/**
 * Generate optimized C type declaration with const/mutable semantics
 */
void generate_c_type_declaration(SmartCodegen *codegen, ASTNode *type_node, bool is_mutable);

/**
 * Generate in-place function call
 * Converts f(x) to f_inplace(&x)
 */
void generate_inplace_function_call(SmartCodegen *codegen, ASTNode *func_call, ASTNode *target_var);

/**
 * Generate RVO function signature
 * Adds hidden return slot parameter
 */
void generate_rvo_function_signature(SmartCodegen *codegen, ASTNode *function_decl);

/**
 * Generate pointer operation for mutable access
 * Handles address-of operations for in-place modifications
 */
void generate_pointer_operation(SmartCodegen *codegen, ASTNode *var, bool take_address);

// =============================================================================
// PERFORMANCE TRACKING FUNCTIONS
// =============================================================================

/**
 * Record optimization application in statistics
 */
void record_optimization_applied(SmartCodegen *codegen, OptimizationPattern pattern, size_t bytes_saved);

/**
 * Generate performance statistics comment
 */
void generate_performance_comment(SmartCodegen *codegen, OptimizationContext *context);

/**
 * Print final optimization statistics
 */
void print_optimization_statistics(SmartCodegen *codegen);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Generate indentation for pretty-printed C code
 */
void generate_indent(SmartCodegen *codegen);

/**
 * Increment indentation level
 */
void indent_increase(SmartCodegen *codegen);

/**
 * Decrement indentation level
 */
void indent_decrease(SmartCodegen *codegen);

/**
 * Generate debug comment with optimization information
 */
void generate_debug_comment(SmartCodegen *codegen, const char *message);

/**
 * Check if function supports in-place operation
 * Returns true if function_name_inplace exists
 */
bool function_supports_inplace(const char *function_name);

/**
 * Generate in-place function name
 * Converts "update_physics" to "update_physics_inplace"
 */
char *generate_inplace_function_name(const char *original_name);

/**
 * Estimate C code size for complexity analysis
 */
size_t estimate_generated_code_size(ASTNode *node);

// =============================================================================
// STRATEGY SELECTION FUNCTIONS
// =============================================================================

/**
 * Select optimal code generation strategy based on analysis
 */
CodegenStrategy select_codegen_strategy(OptimizationContext *context);

/**
 * Should apply optimization based on strategy and confidence
 */
bool should_apply_optimization(CodegenStrategy strategy, OptimizationConfidence confidence);

/**
 * Configure smart codegen based on optimization level
 */
void configure_smart_codegen(SmartCodegen *codegen, int optimization_level);

#endif // ASTHRA_SMART_CODEGEN_H 
