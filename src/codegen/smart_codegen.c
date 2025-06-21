#include "smart_codegen.h"
#include "../parser/ast_types.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * Asthra Programming Language - Smart Code Generation Implementation
 * 
 * This module implements the breakthrough smart compiler optimization that enables
 * AI models to write simple value semantics while achieving C-level performance
 * through intelligent pattern recognition and optimization application.
 */

// Forward declarations for functions defined later in file
bool smart_codegen_generate_function_call(SmartCodegen *codegen, ASTNode *call_expr);
void generate_c_base_type(SmartCodegen *codegen, const char *type_name);
void generate_indent(SmartCodegen *codegen);
bool generate_basic_node(SmartCodegen *codegen, ASTNode *node);
bool generate_standard_assignment(SmartCodegen *codegen, ASTNode *assignment);

// =============================================================================
// SMART CODEGEN LIFECYCLE
// =============================================================================

SmartCodegen *smart_codegen_create(FILE *output) {
    if (!output) return NULL;
    
    SmartCodegen *codegen = calloc(1, sizeof(SmartCodegen));
    if (!codegen) return NULL;
    
    codegen->output = output;
    codegen->opt_context = NULL;
    codegen->indent_level = 0;
    codegen->debug_mode = false;
    codegen->generate_stats = true;
    
    // Initialize counters
    codegen->optimizations_applied = 0;
    codegen->copies_eliminated = 0;
    codegen->memory_traffic_saved = 0;
    
    return codegen;
}

void smart_codegen_destroy(SmartCodegen *codegen) {
    if (!codegen) return;
    
    if (codegen->opt_context) {
        destroy_optimization_context(codegen->opt_context);
    }
    
    free(codegen);
}

// =============================================================================
// CORE SMART CODE GENERATION
// =============================================================================

bool smart_codegen_generate_node(SmartCodegen *codegen, ASTNode *node) {
    if (!codegen || !node) return false;
    
    // Analyze optimization opportunities
    OptimizationContext context = analyze_optimization_opportunity(node);
    codegen->opt_context = &context;
    
    // Dispatch to specific generation functions based on node type
    switch (node->type) {
        case AST_ASSIGNMENT:
            return smart_codegen_generate_assignment(codegen, node);
            
        case AST_LET_STMT:
            return smart_codegen_generate_variable_decl(codegen, node);
            
        case AST_CALL_EXPR:
            return smart_codegen_generate_function_call(codegen, node);
            
        default:
            // Fall back to basic generation for unoptimized nodes
            return generate_basic_node(codegen, node);
    }
}

bool smart_codegen_generate_assignment(SmartCodegen *codegen, ASTNode *assignment) {
    if (!codegen || !assignment || assignment->type != AST_ASSIGNMENT) {
        return false;
    }
    
    OptimizationContext *context = codegen->opt_context;
    if (!context) {
        return false;
    }
    
    // Select optimization strategy
    CodegenStrategy strategy = select_codegen_strategy(context);
    
    // Apply optimizations based on detected patterns
    switch (context->detected_pattern) {
        case OPT_PATTERN_SELF_MUTATION:
            if (should_apply_optimization(strategy, context->confidence)) {
                return apply_self_mutation_optimization(codegen, assignment);
            }
            break;
            
        case OPT_PATTERN_CALL_CHAIN:
            if (should_apply_optimization(strategy, context->confidence)) {
                return apply_call_chain_optimization(codegen, assignment);
            }
            break;
            
        case OPT_PATTERN_SIMPLE_ASSIGNMENT:
            if (should_apply_optimization(strategy, context->confidence)) {
                return apply_copy_elimination(codegen, assignment);
            }
            break;
            
        default:
            break;
    }
    
    // Fall back to standard assignment generation
    return generate_standard_assignment(codegen, assignment);
}

bool smart_codegen_generate_variable_decl(SmartCodegen *codegen, ASTNode *let_stmt) {
    if (!codegen || !let_stmt || let_stmt->type != AST_LET_STMT) {
        return false;
    }
    
    generate_indent(codegen);
    
    // Get variable declaration data
    char *var_name = let_stmt->data.let_stmt.name;
    ASTNode *var_type = let_stmt->data.let_stmt.type;
    ASTNode *initializer = let_stmt->data.let_stmt.initializer;
    bool is_mutable = let_stmt->data.let_stmt.is_mutable;
    
    if (!var_name) return false;
    
    // Generate optimized C type with const/mutable semantics
    generate_c_type_declaration(codegen, var_type, is_mutable);
    fprintf(codegen->output, " %s", var_name);
    
    if (initializer) {
        fprintf(codegen->output, " = ");
        smart_codegen_generate_node(codegen, initializer);
    }
    
    fprintf(codegen->output, ";\n");
    
    // Generate optimization comment if applicable
    if (codegen->debug_mode && !is_mutable) {
        generate_debug_comment(codegen, "Immutable variable - compiler can optimize");
    }
    
    return true;
}

// =============================================================================
// OPTIMIZATION APPLICATION IMPLEMENTATIONS
// =============================================================================

bool apply_self_mutation_optimization(SmartCodegen *codegen, ASTNode *assignment) {
    if (!codegen || !assignment) return false;
    
    ASTNode *target = assignment->data.assignment.target;
    ASTNode *value = assignment->data.assignment.value;
    
    if (!target || !value || value->type != AST_CALL_EXPR) {
        return false;
    }
    
    // Extract function call information
    ASTNode *func_call = value;
    ASTNode *function = func_call->data.call_expr.function;
    
    if (!function || function->type != AST_IDENTIFIER) {
        return false;
    }
    
    char *func_name = function->data.identifier.name;
    char *inplace_name = generate_inplace_function_name(func_name);
    
    if (!function_supports_inplace(func_name)) {
        free(inplace_name);
        return generate_standard_assignment(codegen, assignment);
    }
    
    // Generate optimized in-place call
    generate_indent(codegen);
    
    fprintf(codegen->output, "%s(", inplace_name);
    
    // First argument is address of target variable
    generate_pointer_operation(codegen, target, true);
    
    // Add remaining arguments from original call
    if (func_call->data.call_expr.args && func_call->data.call_expr.args->count > 1) {
        for (size_t i = 1; i < func_call->data.call_expr.args->count; i++) {
            fprintf(codegen->output, ", ");
            smart_codegen_generate_node(codegen, func_call->data.call_expr.args->nodes[i]);
        }
    }
    
    fprintf(codegen->output, ");\n");
    
    // Record optimization statistics
    if (codegen->opt_context) {
        size_t bytes_saved = calculate_memory_traffic_reduction(codegen->opt_context);
        record_optimization_applied(codegen, OPT_PATTERN_SELF_MUTATION, bytes_saved);
    }
    
    free(inplace_name);
    return true;
}

bool apply_call_chain_optimization(SmartCodegen *codegen, ASTNode *assignment) {
    if (!codegen || !assignment) return false;
    
    ASTNode *target = assignment->data.assignment.target;
    ASTNode *value = assignment->data.assignment.value;
    
    // Analyze the call chain
    CallChainAnalysis chain = analyze_call_chain(value);
    
    if (chain.function_count < 2 || !functions_support_inplace(&chain)) {
        return generate_standard_assignment(codegen, assignment);
    }
    
    if (codegen->debug_mode) {
        generate_indent(codegen);
        fprintf(codegen->output, "// OPTIMIZATION: Call chain with %zu functions converted to in-place operations\n", 
                chain.function_count);
    }
    
    // Generate in-place calls in reverse order (innermost first)
    for (int i = chain.function_count - 1; i >= 0; i--) {
        generate_indent(codegen);
        
        ASTNode *func_call = chain.functions[i];
        ASTNode *function = func_call->data.call_expr.function;
        
        if (function && function->type == AST_IDENTIFIER) {
            char *func_name = function->data.identifier.name;
            char *inplace_name = generate_inplace_function_name(func_name);
            
            fprintf(codegen->output, "%s(", inplace_name);
            
            // First argument is always address of target
            generate_pointer_operation(codegen, target, true);
            
            // Add any additional arguments
            if (func_call->data.call_expr.args && func_call->data.call_expr.args->count > 1) {
                for (size_t j = 1; j < func_call->data.call_expr.args->count; j++) {
                    fprintf(codegen->output, ", ");
                    smart_codegen_generate_node(codegen, func_call->data.call_expr.args->nodes[j]);
                }
            }
            
            fprintf(codegen->output, ");\n");
            free(inplace_name);
        }
    }
    
    // Record optimization statistics
    if (codegen->opt_context) {
        size_t bytes_saved = calculate_memory_traffic_reduction(codegen->opt_context);
        record_optimization_applied(codegen, OPT_PATTERN_CALL_CHAIN, bytes_saved);
    }
    
    // Cleanup chain analysis
    free(chain.functions);
    
    return true;
}

bool apply_copy_elimination(SmartCodegen *codegen, ASTNode *assignment) {
    if (!codegen || !assignment) return false;
    
    // For large type assignments, use memcpy instead of value copy
    ASTNode *target = assignment->data.assignment.target;
    ASTNode *value = assignment->data.assignment.value;
    
    if (codegen->debug_mode) {
        generate_indent(codegen);
        fprintf(codegen->output, "// OPTIMIZATION: Copy elimination for large type\n");
    }
    
    generate_indent(codegen);
    fprintf(codegen->output, "memcpy(&");
    smart_codegen_generate_node(codegen, target);
    fprintf(codegen->output, ", &");
    smart_codegen_generate_node(codegen, value);
    fprintf(codegen->output, ", sizeof(");
    smart_codegen_generate_node(codegen, target);
    fprintf(codegen->output, "));\n");
    
    // Record optimization
    if (codegen->opt_context) {
        size_t bytes_saved = calculate_memory_traffic_reduction(codegen->opt_context);
        record_optimization_applied(codegen, OPT_PATTERN_SIMPLE_ASSIGNMENT, bytes_saved);
    }
    
    return true;
}

// =============================================================================
// C CODE GENERATION HELPERS
// =============================================================================

void generate_c_type_declaration(SmartCodegen *codegen, ASTNode *type_node, bool is_mutable) {
    if (!codegen || !type_node) {
        fprintf(codegen->output, "void");
        return;
    }
    
    // Add const qualifier for immutable variables
    if (!is_mutable) {
        fprintf(codegen->output, "const ");
    }
    
    // Generate base C type
    switch (type_node->type) {
        case AST_BASE_TYPE:
            generate_c_base_type(codegen, type_node->data.base_type.name);
            break;
            
        case AST_STRUCT_TYPE:
            fprintf(codegen->output, "struct %s", type_node->data.struct_type.name);
            break;
            
        case AST_PTR_TYPE:
            generate_c_type_declaration(codegen, type_node->data.ptr_type.pointee_type, is_mutable);
            fprintf(codegen->output, " *");
            break;
            
        default:
            fprintf(codegen->output, "void");
            break;
    }
}

void generate_c_base_type(SmartCodegen *codegen, const char *type_name) {
    if (!type_name) {
        fprintf(codegen->output, "void");
        return;
    }
    
    // Map Asthra types to C types
    if (strcmp(type_name, "i8") == 0) fprintf(codegen->output, "int8_t");
    else if (strcmp(type_name, "i16") == 0) fprintf(codegen->output, "int16_t");
    else if (strcmp(type_name, "i32") == 0) fprintf(codegen->output, "int32_t");
    else if (strcmp(type_name, "i64") == 0) fprintf(codegen->output, "int64_t");
    else if (strcmp(type_name, "u8") == 0) fprintf(codegen->output, "uint8_t");
    else if (strcmp(type_name, "u16") == 0) fprintf(codegen->output, "uint16_t");
    else if (strcmp(type_name, "u32") == 0) fprintf(codegen->output, "uint32_t");
    else if (strcmp(type_name, "u64") == 0) fprintf(codegen->output, "uint64_t");
    else if (strcmp(type_name, "f32") == 0) fprintf(codegen->output, "float");
    else if (strcmp(type_name, "f64") == 0) fprintf(codegen->output, "double");
    else if (strcmp(type_name, "bool") == 0) fprintf(codegen->output, "bool");
    else if (strcmp(type_name, "char") == 0) fprintf(codegen->output, "char");
    else if (strcmp(type_name, "string") == 0) fprintf(codegen->output, "char *");
    else {
        // Unknown type - output as-is
        fprintf(codegen->output, "%s", type_name);
    }
}

void generate_pointer_operation(SmartCodegen *codegen, ASTNode *var, bool take_address) {
    if (!codegen || !var) return;
    
    if (take_address) {
        fprintf(codegen->output, "&");
    }
    
    smart_codegen_generate_node(codegen, var);
}

// =============================================================================
// UTILITY FUNCTIONS IMPLEMENTATION
// =============================================================================

void generate_indent(SmartCodegen *codegen) {
    if (!codegen) return;
    
    for (int i = 0; i < codegen->indent_level; i++) {
        fprintf(codegen->output, "    ");
    }
}

void indent_increase(SmartCodegen *codegen) {
    if (codegen) {
        codegen->indent_level++;
    }
}

void indent_decrease(SmartCodegen *codegen) {
    if (codegen && codegen->indent_level > 0) {
        codegen->indent_level--;
    }
}

void generate_debug_comment(SmartCodegen *codegen, const char *message) {
    if (!codegen || !message || !codegen->debug_mode) return;
    
    generate_indent(codegen);
    fprintf(codegen->output, "// %s\n", message);
}

bool function_supports_inplace(const char *function_name) {
    if (!function_name) return false;
    
    // For demonstration, assume common functions support in-place operations
    const char *inplace_functions[] = {
        "update_physics", "update_players", "update_world",
        "apply_game_rules", "apply_ai_behaviors", "process_data",
        "transform_state", "calculate_score", "normalize_data",
        NULL
    };
    
    for (int i = 0; inplace_functions[i]; i++) {
        if (strcmp(function_name, inplace_functions[i]) == 0) {
            return true;
        }
    }
    
    return false; // Conservative default
}

char *generate_inplace_function_name(const char *original_name) {
    if (!original_name) return NULL;
    
    size_t len = strlen(original_name) + strlen("_inplace") + 1;
    char *inplace_name = malloc(len);
    if (!inplace_name) return NULL;
    
    snprintf(inplace_name, len, "%s_inplace", original_name);
    return inplace_name;
}

// =============================================================================
// STRATEGY AND PERFORMANCE FUNCTIONS
// =============================================================================

CodegenStrategy select_codegen_strategy(OptimizationContext *context) {
    if (!context) return CODEGEN_STRATEGY_NAIVE;
    
    // Select strategy based on confidence and potential benefits
    if (context->confidence >= OPT_CONFIDENCE_HIGH && 
        context->performance_improvement_factor >= 4) {
        return CODEGEN_STRATEGY_AGGRESSIVE;
    } else if (context->confidence >= OPT_CONFIDENCE_MEDIUM &&
               context->performance_improvement_factor >= 2) {
        return CODEGEN_STRATEGY_OPTIMIZED;
    } else {
        return CODEGEN_STRATEGY_NAIVE;
    }
}

bool should_apply_optimization(CodegenStrategy strategy, OptimizationConfidence confidence) {
    switch (strategy) {
        case CODEGEN_STRATEGY_NAIVE:
            return false; // Never apply optimizations
            
        case CODEGEN_STRATEGY_OPTIMIZED:
            return confidence >= OPT_CONFIDENCE_MEDIUM;
            
        case CODEGEN_STRATEGY_AGGRESSIVE:
            return confidence >= OPT_CONFIDENCE_LOW;
            
        default:
            return false;
    }
}

void record_optimization_applied(SmartCodegen *codegen, OptimizationPattern pattern, size_t bytes_saved) {
    if (!codegen) return;
    
    codegen->optimizations_applied++;
    codegen->memory_traffic_saved += bytes_saved;
    
    // Estimate copies eliminated based on pattern
    switch (pattern) {
        case OPT_PATTERN_SELF_MUTATION:
            codegen->copies_eliminated += 2;
            break;
        case OPT_PATTERN_CALL_CHAIN:
            codegen->copies_eliminated += 4;
            break;
        case OPT_PATTERN_SIMPLE_ASSIGNMENT:
            codegen->copies_eliminated += 1;
            break;
        default:
            break;
    }
}

void generate_performance_comment(SmartCodegen *codegen, OptimizationContext *context) {
    if (!codegen || !context) return;
    
    generate_indent(codegen);
    fprintf(codegen->output, "// SMART OPTIMIZATION: %s (Confidence: %s, Improvement: %dx)\n",
            optimization_pattern_to_string(context->detected_pattern),
            optimization_confidence_to_string(context->confidence),
            context->performance_improvement_factor);
}

void print_optimization_statistics(SmartCodegen *codegen) {
    if (!codegen) return;
    
    printf("\n=== Smart Code Generation Statistics ===\n");
    printf("Optimizations Applied: %d\n", codegen->optimizations_applied);
    printf("Copies Eliminated: %d\n", codegen->copies_eliminated);
    printf("Memory Traffic Saved: %zu bytes\n", codegen->memory_traffic_saved);
    printf("========================================\n\n");
}

// =============================================================================
// FALLBACK FUNCTIONS FOR UNOPTIMIZED CASES
// =============================================================================

bool generate_basic_node(SmartCodegen *codegen, ASTNode *node) {
    if (!codegen || !node) return false;
    
    // Basic node generation without optimizations
    switch (node->type) {
        case AST_IDENTIFIER:
            if (node->data.identifier.name) {
                fprintf(codegen->output, "%s", node->data.identifier.name);
            }
            break;
            
        case AST_INTEGER_LITERAL:
            fprintf(codegen->output, "%lld", (long long)node->data.integer_literal.value);
            break;
            
        case AST_STRING_LITERAL:
            if (node->data.string_literal.value) {
                fprintf(codegen->output, "\"%s\"", node->data.string_literal.value);
            }
            break;
            
        default:
            // Unsupported node type
            return false;
    }
    
    return true;
}

bool generate_standard_assignment(SmartCodegen *codegen, ASTNode *assignment) {
    if (!codegen || !assignment) return false;
    
    generate_indent(codegen);
    smart_codegen_generate_node(codegen, assignment->data.assignment.target);
    fprintf(codegen->output, " = ");
    smart_codegen_generate_node(codegen, assignment->data.assignment.value);
    fprintf(codegen->output, ";\n");
    
    return true;
}

bool smart_codegen_generate_function_call(SmartCodegen *codegen, ASTNode *call_expr) {
    if (!codegen || !call_expr || call_expr->type != AST_CALL_EXPR) {
        return false;
    }
    
    // Generate basic function call - can be enhanced with optimizations later
    ASTNode *function = call_expr->data.call_expr.function;
    if (function && function->type == AST_IDENTIFIER) {
        fprintf(codegen->output, "%s(", function->data.identifier.name);
        
        // Generate arguments
        if (call_expr->data.call_expr.args && call_expr->data.call_expr.args->count > 0) {
            for (size_t i = 0; i < call_expr->data.call_expr.args->count; i++) {
                if (i > 0) {
                    fprintf(codegen->output, ", ");
                }
                smart_codegen_generate_node(codegen, call_expr->data.call_expr.args->nodes[i]);
            }
        }
        
        fprintf(codegen->output, ")");
    }
    
    return true;
} 
