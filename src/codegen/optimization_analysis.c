#include "optimization_analysis.h"
#include "../parser/ast_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Asthra Programming Language - Smart Compiler Optimization Analysis Implementation
 * 
 * This module implements the core analysis engine that enables C-level performance
 * from AI-friendly value semantics through intelligent pattern recognition and
 * optimization decision making.
 */

// =============================================================================
// SIZE ANALYSIS IMPLEMENTATION
// =============================================================================

size_t get_basic_type_size(const char *type_name) {
    if (!type_name) return 0;
    
    // Basic type sizes in bytes
    if (strcmp(type_name, "i8") == 0) return 1;
    if (strcmp(type_name, "i16") == 0) return 2;
    if (strcmp(type_name, "i32") == 0) return 4;
    if (strcmp(type_name, "i64") == 0) return 8;
    if (strcmp(type_name, "u8") == 0) return 1;
    if (strcmp(type_name, "u16") == 0) return 2;
    if (strcmp(type_name, "u32") == 0) return 4;
    if (strcmp(type_name, "u64") == 0) return 8;
    if (strcmp(type_name, "i128") == 0) return 16;
    if (strcmp(type_name, "u128") == 0) return 16;
    if (strcmp(type_name, "f32") == 0) return 4;
    if (strcmp(type_name, "f64") == 0) return 8;
    if (strcmp(type_name, "bool") == 0) return 1;
    if (strcmp(type_name, "char") == 0) return 4; // UTF-32
    
    // Pointer types
    if (strstr(type_name, "*") || strcmp(type_name, "string") == 0) {
        return sizeof(void*); // 8 bytes on 64-bit systems
    }
    
    // Unknown type - estimate conservatively
    return 32; // Assume medium-sized struct
}

size_t estimate_type_size_bytes(ASTNode *type_node) {
    if (!type_node) return 32; // Default estimate
    
    switch (type_node->type) {
        case AST_BASE_TYPE:
            return get_basic_type_size(type_node->data.base_type.name);
            
        case AST_STRUCT_TYPE:
            // Estimate struct size - heuristic based on common patterns
            // Real implementation would analyze struct fields
            return 128; // Typical struct estimate
            
        case AST_SLICE_TYPE:
            // Slice is typically 16 bytes (ptr + len)
            return 16;
            
        case AST_PTR_TYPE:
            return sizeof(void*);
            
        case AST_RESULT_TYPE:
            // Result<T,E> typically 16-24 bytes + sizeof(T) + sizeof(E)
            return 32;
            
        default:
            return 32; // Conservative estimate
    }
}

SizeCategory estimate_type_size_category(ASTNode *type_node) {
    size_t size = estimate_type_size_bytes(type_node);
    
    if (size <= 32) {
        return SIZE_CATEGORY_SMALL;
    } else if (size <= 128) {
        return SIZE_CATEGORY_MEDIUM;
    } else if (size <= 1024) {
        return SIZE_CATEGORY_LARGE;
    } else {
        return SIZE_CATEGORY_HUGE;
    }
}

bool is_type_worth_optimizing(ASTNode *type_node) {
    return estimate_type_size_bytes(type_node) > 64;
}

// =============================================================================
// PATTERN DETECTION IMPLEMENTATION  
// =============================================================================

bool is_self_mutation_pattern(ASTNode *assignment) {
    if (!assignment || assignment->type != AST_ASSIGNMENT) {
        return false;
    }
    
    // Extract target and value from assignment
    ASTNode *target = assignment->data.assignment.target;
    ASTNode *value = assignment->data.assignment.value;
    
    if (!target || !value) return false;
    
    // Check if value is a function call
    if (value->type != AST_CALL_EXPR) return false;
    
    // Check if first argument to function is the same variable being assigned
    ASTNode *func_call = value;
    if (!func_call->data.call_expr.args || func_call->data.call_expr.args->count == 0) {
        return false;
    }
    
    ASTNode *first_arg = func_call->data.call_expr.args->nodes[0];
    
    // Simple case: target and first argument are both identifiers with same name
    if (target->type == AST_IDENTIFIER && first_arg->type == AST_IDENTIFIER) {
        return strcmp(target->data.identifier.name, first_arg->data.identifier.name) == 0;
    }
    
    return false; // More complex patterns would need deeper analysis
}

bool is_call_chain_pattern(ASTNode *assignment) {
    if (!assignment || assignment->type != AST_ASSIGNMENT) {
        return false;
    }
    
    ASTNode *value = assignment->data.assignment.value;
    
    // Check for nested function calls: f1(f2(f3(x)))
    int chain_depth = 0;
    ASTNode *current = value;
    
    while (current && current->type == AST_CALL_EXPR) {
        chain_depth++;
        
        // Move to first argument if it's also a function call
        if (current->data.call_expr.args && current->data.call_expr.args->count > 0) {
            ASTNode *first_arg = current->data.call_expr.args->nodes[0];
            if (first_arg->type == AST_CALL_EXPR) {
                current = first_arg;
            } else {
                break;
            }
        } else {
            break;
        }
    }
    
    // Consider it a chain if we have 2+ nested calls
    return chain_depth >= 2;
}

CallChainAnalysis analyze_call_chain(ASTNode *expression) {
    CallChainAnalysis analysis = {0};
    
    if (!expression || expression->type != AST_CALL_EXPR) {
        return analysis;
    }
    
    // Count function calls in chain
    int count = 0;
    ASTNode *current = expression;
    
    // First pass: count functions
    while (current && current->type == AST_CALL_EXPR) {
        count++;
        if (current->data.call_expr.args && current->data.call_expr.args->count > 0) {
            current = current->data.call_expr.args->nodes[0];
        } else {
            break;
        }
    }
    
    if (count < 2) return analysis; // Not a chain
    
    // Allocate space for functions
    analysis.functions = malloc(sizeof(ASTNode*) * (size_t)count);
    analysis.function_count = (size_t)count;
    
    // Second pass: collect functions
    current = expression;
    for (int i = 0; i < count && current; i++) {
        analysis.functions[i] = current;
        if (current->data.call_expr.args && current->data.call_expr.args->count > 0) {
            current = current->data.call_expr.args->nodes[0];
        }
    }
    
    // Set target variable (final argument in chain)
    analysis.target_variable = current;
    analysis.is_self_modifying = true; // Assume self-modifying for now
    analysis.all_support_inplace = true; // Optimistic - would check function signatures
    
    return analysis;
}

bool functions_support_inplace(CallChainAnalysis *chain) {
    if (!chain || !chain->functions) return false;
    
    // For now, assume all functions support in-place operations
    // Real implementation would check function signatures for _inplace variants
    return chain->all_support_inplace;
}

// =============================================================================
// OPTIMIZATION PATTERN DETECTION
// =============================================================================

OptimizationPattern detect_optimization_pattern(ASTNode *assignment) {
    if (!assignment) return OPT_PATTERN_NONE;
    
    if (is_self_mutation_pattern(assignment)) {
        return OPT_PATTERN_SELF_MUTATION;
    }
    
    if (is_call_chain_pattern(assignment)) {
        return OPT_PATTERN_CALL_CHAIN;
    }
    
    // Check for simple assignment with large types
    if (assignment->type == AST_ASSIGNMENT) {
        ASTNode *target = assignment->data.assignment.target;
        // For now, assume assignments to identifiers could be optimizable
        // Real implementation would use semantic analysis results
        if (target && target->type == AST_IDENTIFIER) {
            return OPT_PATTERN_SIMPLE_ASSIGNMENT;
        }
    }
    
    return OPT_PATTERN_NONE;
}

bool can_apply_return_value_optimization(ASTNode *function_decl) {
    if (!function_decl || function_decl->type != AST_FUNCTION_DECL) {
        return false;
    }
    
    ASTNode *return_type = function_decl->data.function_decl.return_type;
    
    // RVO beneficial for large return types
    return return_type && is_type_worth_optimizing(return_type);
}

// =============================================================================
// CORE ANALYSIS IMPLEMENTATION
// =============================================================================

OptimizationContext analyze_optimization_opportunity(ASTNode *node) {
    OptimizationContext context = {0};
    
    if (!node) return context;
    
    context.current_function = NULL; // Would set based on traversal context
    context.is_in_loop = false;      // Would detect based on parent nodes
    context.nested_call_depth = 0;
    
    // Detect optimization pattern
    context.detected_pattern = detect_optimization_pattern(node);
    
    // Estimate type size for the operation
    if (node->type == AST_ASSIGNMENT && node->data.assignment.target) {
        // Use conservative estimates for now - real implementation would use semantic analysis
        context.estimated_type_size = 64; // Conservative estimate for optimization decisions
        context.size_category = SIZE_CATEGORY_MEDIUM;
    }
    
    // Set optimization flags based on pattern and size
    switch (context.detected_pattern) {
        case OPT_PATTERN_SELF_MUTATION:
            context.can_use_inplace = true;
            context.can_eliminate_copies = true;
            context.potential_copies_eliminated = 2; // Input copy + return copy
            break;
            
        case OPT_PATTERN_CALL_CHAIN:
            context.can_use_inplace = true;
            context.can_eliminate_copies = true;
            // Multiple function calls can eliminate many copies
            context.potential_copies_eliminated = 4; // Estimate based on chain length
            break;
            
        case OPT_PATTERN_SIMPLE_ASSIGNMENT:
            if (context.size_category >= SIZE_CATEGORY_LARGE) {
                context.can_eliminate_copies = true;
                context.potential_copies_eliminated = 1;
            }
            break;
            
        default:
            break;
    }
    
    // Calculate confidence based on pattern and context
    context.confidence = calculate_optimization_confidence(&context);
    
    // Calculate performance improvement
    context.performance_improvement_factor = calculate_performance_improvement_factor(&context);
    
    return context;
}

OptimizationConfidence calculate_optimization_confidence(OptimizationContext *context) {
    if (!context) return OPT_CONFIDENCE_NONE;
    
    // Base confidence on pattern type
    switch (context->detected_pattern) {
        case OPT_PATTERN_NONE:
            return OPT_CONFIDENCE_NONE;
            
        case OPT_PATTERN_SELF_MUTATION:
            // Self-mutation is very safe to optimize
            if (context->size_category >= SIZE_CATEGORY_LARGE) {
                return OPT_CONFIDENCE_CERTAIN;
            } else if (context->size_category >= SIZE_CATEGORY_MEDIUM) {
                return OPT_CONFIDENCE_HIGH;
            } else {
                return OPT_CONFIDENCE_LOW; // Not worth optimizing small types
            }
            
        case OPT_PATTERN_CALL_CHAIN:
            // Call chains are generally safe if all functions support in-place
            return context->can_use_inplace ? OPT_CONFIDENCE_HIGH : OPT_CONFIDENCE_LOW;
            
        case OPT_PATTERN_SIMPLE_ASSIGNMENT:
            // Simple assignments are safe for large types
            return context->size_category >= SIZE_CATEGORY_LARGE ? 
                   OPT_CONFIDENCE_HIGH : OPT_CONFIDENCE_MEDIUM;
            
        default:
            return OPT_CONFIDENCE_MEDIUM;
    }
}

// =============================================================================
// PERFORMANCE CALCULATION IMPLEMENTATION
// =============================================================================

int calculate_performance_improvement_factor(OptimizationContext *context) {
    if (!context || context->potential_copies_eliminated <= 0) {
        return 1; // No improvement
    }
    
    // Calculate improvement based on eliminated copies and type size
    size_t bytes_per_copy = context->estimated_type_size;
    int copies_eliminated = context->potential_copies_eliminated;
    
    // Performance improvement is roughly proportional to:
    // (number of copies eliminated) * (bytes per copy) / (fixed overhead)
    int base_improvement = copies_eliminated * (int)(bytes_per_copy / 32);
    
    // Cap at realistic maximum
    return base_improvement > 2048 ? 2048 : (base_improvement < 1 ? 1 : base_improvement);
}

size_t calculate_memory_traffic_reduction(OptimizationContext *context) {
    if (!context) return 0;
    
    return context->estimated_type_size * (size_t)context->potential_copies_eliminated;
}

bool is_optimization_beneficial(OptimizationContext *context) {
    if (!context) return false;
    
    // Optimization is beneficial if:
    // 1. We have high confidence it's safe
    // 2. Performance improvement is significant (>2x)
    // 3. Type is worth optimizing (>64 bytes)
    
    return context->confidence >= OPT_CONFIDENCE_MEDIUM &&
           context->performance_improvement_factor >= 2 &&
           context->estimated_type_size > 64;
}

// =============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =============================================================================

OptimizationContext *create_optimization_context(void) {
    OptimizationContext *context = calloc(1, sizeof(OptimizationContext));
    return context;
}

void destroy_optimization_context(OptimizationContext *context) {
    free(context);
}

const char *optimization_pattern_to_string(OptimizationPattern pattern) {
    switch (pattern) {
        case OPT_PATTERN_NONE: return "None";
        case OPT_PATTERN_SELF_MUTATION: return "Self-Mutation";
        case OPT_PATTERN_CALL_CHAIN: return "Call-Chain";
        case OPT_PATTERN_SIMPLE_ASSIGNMENT: return "Simple-Assignment";
        case OPT_PATTERN_RETURN_VALUE_OPT: return "Return-Value-Optimization";
        case OPT_PATTERN_TEMPORARY_ELIMINATION: return "Temporary-Elimination";
        default: return "Unknown";
    }
}

const char *optimization_confidence_to_string(OptimizationConfidence confidence) {
    if (confidence >= OPT_CONFIDENCE_CERTAIN) return "Certain";
    if (confidence >= OPT_CONFIDENCE_HIGH) return "High";
    if (confidence >= OPT_CONFIDENCE_MEDIUM) return "Medium";
    if (confidence >= OPT_CONFIDENCE_LOW) return "Low";
    return "None";
}

void print_optimization_analysis(OptimizationContext *context) {
    if (!context) {
        printf("No optimization context available\n");
        return;
    }
    
    printf("=== Optimization Analysis ===\n");
    printf("Pattern: %s\n", optimization_pattern_to_string(context->detected_pattern));
    printf("Confidence: %s (%d%%)\n", 
           optimization_confidence_to_string(context->confidence), 
           context->confidence);
    printf("Type Size: %zu bytes\n", context->estimated_type_size);
    printf("Copies Eliminated: %d\n", context->potential_copies_eliminated);
    printf("Performance Improvement: %dx\n", context->performance_improvement_factor);
    printf("Can Use In-Place: %s\n", context->can_use_inplace ? "Yes" : "No");
    printf("Can Eliminate Copies: %s\n", context->can_eliminate_copies ? "Yes" : "No");
    printf("Beneficial: %s\n", is_optimization_beneficial(context) ? "Yes" : "No");
    printf("============================\n\n");
} 
