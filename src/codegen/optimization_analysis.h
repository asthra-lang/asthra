#ifndef ASTHRA_OPTIMIZATION_ANALYSIS_H
#define ASTHRA_OPTIMIZATION_ANALYSIS_H

#include "../parser/ast_node.h"
#include <stdbool.h>
#include <stdint.h>

/**
 * Asthra Programming Language - Smart Compiler Optimization Analysis
 * 
 * Phase 3: Code Generation & Runtime Optimization
 * 
 * This module implements the breakthrough "Explicit Mutability + Smart Compiler Optimization"
 * approach that enables AI models to write simple value semantics code while achieving
 * C-level performance through intelligent compiler optimization.
 * 
 * Key Innovation: Pattern recognition automatically converts simple immutable-by-default
 * code into efficient in-place operations, eliminating 90%+ copies while maintaining
 * AI-friendly syntax.
 */

// Forward declarations
typedef struct ASTNode ASTNode;

// Optimization confidence levels for decision making
typedef enum {
    OPT_CONFIDENCE_NONE = 0,      // No optimization possible
    OPT_CONFIDENCE_LOW = 25,      // Risky optimization - skip
    OPT_CONFIDENCE_MEDIUM = 60,   // Safe optimization - apply cautiously 
    OPT_CONFIDENCE_HIGH = 85,     // Very safe optimization - apply aggressively
    OPT_CONFIDENCE_CERTAIN = 100  // Guaranteed safe optimization - always apply
} OptimizationConfidence;

// Smart optimization patterns detected by analysis
typedef enum {
    OPT_PATTERN_NONE,                    // No optimization pattern
    OPT_PATTERN_SELF_MUTATION,           // f(x) -> x pattern (x = f(x))
    OPT_PATTERN_CALL_CHAIN,              // f1(f2(f3(x))) pattern
    OPT_PATTERN_SIMPLE_ASSIGNMENT,       // Direct assignment with large types
    OPT_PATTERN_RETURN_VALUE_OPT,        // Function return can use RVO
    OPT_PATTERN_TEMPORARY_ELIMINATION    // Eliminate temporary variables
} OptimizationPattern;

// Type size categories for optimization decisions
typedef enum {
    SIZE_CATEGORY_SMALL = 0,    // <= 32 bytes - don't optimize (faster to copy)
    SIZE_CATEGORY_MEDIUM = 1,   // 33-128 bytes - consider optimization
    SIZE_CATEGORY_LARGE = 2,    // 129-1024 bytes - strongly recommend optimization
    SIZE_CATEGORY_HUGE = 3      // > 1024 bytes - always optimize
} SizeCategory;

// Optimization context for analysis decisions
typedef struct {
    // Function context
    ASTNode *current_function;
    bool is_in_loop;
    int nested_call_depth;
    
    // Performance metrics
    size_t estimated_type_size;
    SizeCategory size_category;
    OptimizationConfidence confidence;
    OptimizationPattern detected_pattern;
    
    // Optimization flags
    bool can_apply_rvo;
    bool can_use_inplace;
    bool can_eliminate_copies;
    bool requires_pointer_ops;
    
    // Statistics
    int potential_copies_eliminated;
    int performance_improvement_factor;
} OptimizationContext;

// Function call chain analysis
typedef struct {
    ASTNode **functions;        // Array of function calls in chain
    size_t function_count;      // Number of functions in chain
    ASTNode *target_variable;   // Variable being operated on
    bool all_support_inplace;   // All functions have in-place versions
    bool is_self_modifying;     // Chain modifies its input variable
} CallChainAnalysis;

// =============================================================================
// CORE ANALYSIS FUNCTIONS
// =============================================================================

/**
 * Analyze a statement/expression for optimization opportunities
 * Returns optimization context with recommendations
 */
OptimizationContext analyze_optimization_opportunity(ASTNode *node);

/**
 * Estimate the size category of a type for optimization decisions
 * Uses heuristics for struct/array sizes
 */
SizeCategory estimate_type_size_category(ASTNode *type_node);

/**
 * Detect optimization patterns in assignment statements
 * Key patterns: self-mutation, call chains, RVO opportunities
 */
OptimizationPattern detect_optimization_pattern(ASTNode *assignment);

/**
 * Analyze function call chains for in-place optimization
 * Detects patterns like: x = f1(f2(f3(x)))
 */
CallChainAnalysis analyze_call_chain(ASTNode *expression);

/**
 * Determine if Return Value Optimization (RVO) can be applied
 * Checks for large return types and simple return patterns
 */
bool can_apply_return_value_optimization(ASTNode *function_decl);

/**
 * Calculate confidence level for applying specific optimization
 * Considers type safety, side effects, and performance benefits
 */
OptimizationConfidence calculate_optimization_confidence(OptimizationContext *context);

// =============================================================================
// PATTERN DETECTION FUNCTIONS  
// =============================================================================

/**
 * Detect self-mutation pattern: x = f(x)
 * This is the most common optimization opportunity
 */
bool is_self_mutation_pattern(ASTNode *assignment);

/**
 * Detect call chain pattern: x = f1(f2(f3(x)))
 * Can be optimized to: f3_inplace(&x); f2_inplace(&x); f1_inplace(&x);
 */
bool is_call_chain_pattern(ASTNode *assignment);

/**
 * Check if all functions in chain support in-place operations
 * Required for safe call chain optimization
 */
bool functions_support_inplace(CallChainAnalysis *chain);

/**
 * Detect temporary variable elimination opportunities
 * Find intermediate variables that can be eliminated
 */
bool can_eliminate_temporary_variables(ASTNode *block);

// =============================================================================
// SIZE ANALYSIS FUNCTIONS
// =============================================================================

/**
 * Estimate actual size in bytes for a type
 * Uses struct layout and array size information
 */
size_t estimate_type_size_bytes(ASTNode *type_node);

/**
 * Check if type is worth optimizing (>64 bytes threshold)
 * Small types are faster to copy than optimize
 */
bool is_type_worth_optimizing(ASTNode *type_node);

/**
 * Get C type size for basic types (i32, f64, etc.)
 */
size_t get_basic_type_size(const char *type_name);

// =============================================================================
// PERFORMANCE CALCULATION FUNCTIONS
// =============================================================================

/**
 * Calculate potential performance improvement
 * Returns factor improvement (e.g., 2048x for GameState example)
 */
int calculate_performance_improvement_factor(OptimizationContext *context);

/**
 * Estimate memory traffic reduction
 * Calculates bytes saved by optimization
 */
size_t calculate_memory_traffic_reduction(OptimizationContext *context);

/**
 * Calculate optimization cost vs benefit
 * Helps decide if optimization is worthwhile
 */
bool is_optimization_beneficial(OptimizationContext *context);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Create and initialize optimization context
 */
OptimizationContext *create_optimization_context(void);

/**
 * Cleanup optimization context
 */
void destroy_optimization_context(OptimizationContext *context);

/**
 * Print optimization analysis results for debugging
 */
void print_optimization_analysis(OptimizationContext *context);

/**
 * Convert optimization pattern to string for debugging
 */
const char *optimization_pattern_to_string(OptimizationPattern pattern);

/**
 * Convert confidence level to string for debugging  
 */
const char *optimization_confidence_to_string(OptimizationConfidence confidence);

#endif // ASTHRA_OPTIMIZATION_ANALYSIS_H 
