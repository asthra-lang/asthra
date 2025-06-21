/**
 * Asthra Programming Language - Immutable-by-Default Creators
 * 
 * Creation and cleanup functions for immutability infrastructure
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "immutable_infrastructure.h"

// =============================================================================
// CREATION FUNCTIONS
// =============================================================================

/**
 * Create immutable semantic analyzer
 */
ImmutableSemanticAnalyzer* immutable_analyzer_create(ImmutabilityConfig config) {
    ImmutableSemanticAnalyzer* analyzer = calloc(1, sizeof(ImmutableSemanticAnalyzer));
    if (!analyzer) return NULL;
    
    // Create base semantic analyzer
    analyzer->base_analyzer = semantic_analyzer_create();
    if (!analyzer->base_analyzer) {
        free(analyzer);
        return NULL;
    }
    
    // Initialize immutability configuration
    analyzer->config = config;
    analyzer->max_violations = config.max_mutation_violations;
    analyzer->violations = calloc(analyzer->max_violations, sizeof(MutabilityViolation));
    
    // Initialize symbol tracking
    analyzer->symbol_mutability.immutable_symbols = calloc(1000, sizeof(char*));
    analyzer->symbol_mutability.mutable_symbols = calloc(1000, sizeof(char*));
    
    return analyzer;
}

/**
 * Create immutable code generator
 */
ImmutableCodeGenerator* immutable_generator_create(const char* output_path) {
    ImmutableCodeGenerator* generator = calloc(1, sizeof(ImmutableCodeGenerator));
    if (!generator) return NULL;
    
    // Create base code generator with default architecture
    generator->base_generator = code_generator_create(TARGET_ARCH_X86_64, CALLING_CONV_SYSTEM_V_AMD64);
    if (!generator->base_generator) {
        free(generator);
        return NULL;
    }
    
    // Open output file if provided
    if (output_path) {
        generator->output = fopen(output_path, "w");
        if (!generator->output) {
            code_generator_destroy(generator->base_generator);
            free(generator);
            return NULL;
        }
    }
    
    generator->generate_cow_support = true;
    generator->generate_immutable_checks = true;
    
    return generator;
}

// =============================================================================
// CLEANUP FUNCTIONS
// =============================================================================

/**
 * Cleanup immutable analyzer
 */
void immutable_analyzer_destroy(ImmutableSemanticAnalyzer* analyzer) {
    if (!analyzer) return;
    
    if (analyzer->base_analyzer) {
        semantic_analyzer_destroy(analyzer->base_analyzer);
    }
    
    // Cleanup violations
    if (analyzer->violations) {
        for (size_t i = 0; i < analyzer->violation_count; i++) {
            free((void*)analyzer->violations[i].variable_name);
            free((void*)analyzer->violations[i].violation_reason);
        }
        free(analyzer->violations);
    }
    
    // Cleanup symbol tracking
    if (analyzer->symbol_mutability.immutable_symbols) {
        for (size_t i = 0; i < analyzer->symbol_mutability.immutable_count; i++) {
            free((void*)analyzer->symbol_mutability.immutable_symbols[i]);
        }
        free(analyzer->symbol_mutability.immutable_symbols);
    }
    
    if (analyzer->symbol_mutability.mutable_symbols) {
        for (size_t i = 0; i < analyzer->symbol_mutability.mutable_count; i++) {
            free((void*)analyzer->symbol_mutability.mutable_symbols[i]);
        }
        free(analyzer->symbol_mutability.mutable_symbols);
    }
    
    free(analyzer);
}

/**
 * Cleanup immutable generator
 */
void immutable_generator_destroy(ImmutableCodeGenerator* generator) {
    if (!generator) return;
    
    if (generator->output) {
        fclose(generator->output);
    }
    
    if (generator->base_generator) {
        code_generator_destroy(generator->base_generator);
    }
    
    free(generator);
}