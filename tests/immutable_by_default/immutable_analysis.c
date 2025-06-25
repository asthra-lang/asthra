/**
 * Asthra Programming Language - Immutable-by-Default Analysis
 *
 * Mutability analysis and violation detection functions
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "immutable_infrastructure.h"

// =============================================================================
// TIME UTILITY
// =============================================================================

double get_time_ms(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec * 1000.0 + (double)ts.tv_nsec / 1000000.0;
}

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

static void add_mutability_violation(ImmutableSemanticAnalyzer *analyzer, const char *variable_name,
                                     const char *reason, int line, int column) {
    if (!analyzer || analyzer->violation_count >= analyzer->max_violations)
        return;

    MutabilityViolation *violation = &analyzer->violations[analyzer->violation_count];
    violation->variable_name = strdup(variable_name);
    violation->violation_reason = strdup(reason);
    violation->line = line;
    violation->column = column;

    analyzer->violation_count++;
}

// =============================================================================
// IMMUTABILITY ANALYSIS FUNCTIONS
// =============================================================================

/**
 * Analyze mutability of variable declaration
 */
bool immutable_analyze_variable_declaration(ImmutableSemanticAnalyzer *analyzer,
                                            EnhancedASTNode *var_decl, const char *variable_name) {
    if (!analyzer || !var_decl || !variable_name)
        return false;

    // Check for explicit mutability annotations
    bool is_explicitly_mutable = false;
    bool has_mut_keyword = false; // Would check AST for 'mut' keyword

    if (has_mut_keyword) {
        is_explicitly_mutable = true;
        var_decl->mutability_info.kind = MUTABILITY_MUTABLE;
        var_decl->mutability_info.mutability_source = "explicit mut";

        // Track mutable symbol
        if (analyzer->symbol_mutability.mutable_count < 1000) { // Arbitrary limit
            analyzer->symbol_mutability
                .mutable_symbols[analyzer->symbol_mutability.mutable_count++] =
                strdup(variable_name);
        }
    } else {
        // Default to immutable
        var_decl->mutability_info.kind = MUTABILITY_IMMUTABLE;
        var_decl->mutability_info.mutability_source = "default immutable";
        var_decl->mutability_info.is_deep_immutable = true;

        // Track immutable symbol
        if (analyzer->symbol_mutability.immutable_count < 1000) { // Arbitrary limit
            analyzer->symbol_mutability
                .immutable_symbols[analyzer->symbol_mutability.immutable_count++] =
                strdup(variable_name);
        }
    }

    return true;
}

/**
 * Analyze mutation attempt on variable
 */
bool immutable_analyze_mutation(ImmutableSemanticAnalyzer *analyzer, EnhancedASTNode *mutation_site,
                                const char *variable_name) {
    if (!analyzer || !mutation_site || !variable_name)
        return false;

    // Check if variable is declared as mutable
    bool is_mutable = false;
    for (size_t i = 0; i < analyzer->symbol_mutability.mutable_count; i++) {
        if (strcmp(analyzer->symbol_mutability.mutable_symbols[i], variable_name) == 0) {
            is_mutable = true;
            break;
        }
    }

    if (!is_mutable) {
        // Check if it's in immutable symbols
        for (size_t i = 0; i < analyzer->symbol_mutability.immutable_count; i++) {
            if (strcmp(analyzer->symbol_mutability.immutable_symbols[i], variable_name) == 0) {
                // Mutation of immutable variable detected
                add_mutability_violation(analyzer, variable_name,
                                         "Attempted mutation of immutable variable", 0,
                                         0); // Line/column would come from AST
                return false;
            }
        }
    }

    // Mark as mutation site for code generation
    mutation_site->is_mutation_site = true;

    return is_mutable;
}

/**
 * Analyze borrowing and reference mutability
 */
bool immutable_analyze_borrow(ImmutableSemanticAnalyzer *analyzer, EnhancedASTNode *borrow_site,
                              const char *variable_name, bool is_mutable_borrow) {
    if (!analyzer || !borrow_site || !variable_name)
        return false;

    // Check if mutable borrow of immutable data
    if (is_mutable_borrow) {
        bool is_mutable = false;
        for (size_t i = 0; i < analyzer->symbol_mutability.mutable_count; i++) {
            if (strcmp(analyzer->symbol_mutability.mutable_symbols[i], variable_name) == 0) {
                is_mutable = true;
                break;
            }
        }

        if (!is_mutable) {
            add_mutability_violation(analyzer, variable_name, "Mutable borrow of immutable data", 0,
                                     0);
            return false;
        }
    }

    return true;
}

/**
 * Complete immutability analysis of AST
 */
ImmutabilityAnalysisResult immutable_analyze_program(ImmutableSemanticAnalyzer *analyzer,
                                                     EnhancedASTNode *program_ast) {
    ImmutabilityAnalysisResult result = {0};
    if (!analyzer || !program_ast) {
        result.analysis_successful = false;
        return result;
    }

    double start_time = get_time_ms();

    // Initialize result structure
    result.violations = analyzer->violations;
    result.max_violations = analyzer->max_violations;

    // Perform semantic analysis with base analyzer first
    if (analyzer->base_analyzer) {
        ASTNode *base_ast = (ASTNode *)program_ast; // Cast for compatibility
        bool base_success = semantic_analyze_program(analyzer->base_analyzer, base_ast);
        if (!base_success) {
            result.analysis_successful = false;
            result.analysis_time_ms = get_time_ms() - start_time;
            return result;
        }
    }

    // Perform immutability-specific analysis
    // This would walk the AST and analyze each node for mutability constraints
    // For this implementation, we'll simulate by analyzing a few common patterns

    // Simulate variable declaration analysis
    immutable_analyze_variable_declaration(analyzer, program_ast, "example_var");

    // Simulate mutation analysis
    bool mutation_valid = immutable_analyze_mutation(analyzer, program_ast, "example_var");
    if (!mutation_valid) {
        result.analysis_successful = false;
    } else {
        result.analysis_successful = true;
    }

    result.violation_count = analyzer->violation_count;
    result.analysis_time_ms = get_time_ms() - start_time;

    return result;
}