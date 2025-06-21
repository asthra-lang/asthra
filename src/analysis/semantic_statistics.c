/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Statistics and Profiling Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Statistics collection and profiling for semantic analysis
 */

#include "semantic_statistics.h"
#include <stdio.h>

// C17 feature detection and compatibility
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
    #define ASTHRA_HAS_C17 1
#else
    #define ASTHRA_HAS_C17 0
#endif

#if ASTHRA_HAS_C17 && !defined(__STDC_NO_ATOMICS__)
    #define ASTHRA_HAS_ATOMICS 1
#else
    #define ASTHRA_HAS_ATOMICS 0
#endif

// Atomic operation wrappers with fallbacks
#if ASTHRA_HAS_ATOMICS
    #define ATOMIC_LOAD(ptr) atomic_load(ptr)
    #define ATOMIC_STORE(ptr, val) atomic_store(ptr, val)
#else
    #define ATOMIC_LOAD(ptr) (*(ptr))
    #define ATOMIC_STORE(ptr, val) (*(ptr) = (val))
#endif

// =============================================================================
// STATISTICS MANAGEMENT
// =============================================================================

SemanticStatistics semantic_get_statistics(const SemanticAnalyzer *analyzer) {
    if (!analyzer) {
        return (SemanticStatistics){
            .nodes_analyzed = 0,
            .types_checked = 0,
            .symbols_resolved = 0,
            .errors_found = 0,
            .warnings_issued = 0,
            .max_scope_depth = 0,
            .current_scope_depth = 0
        };
    }
    
    return analyzer->stats;
}

void semantic_reset_statistics(SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    ATOMIC_STORE(&analyzer->stats.nodes_analyzed, 0);
    ATOMIC_STORE(&analyzer->stats.types_checked, 0);
    ATOMIC_STORE(&analyzer->stats.symbols_resolved, 0);
    ATOMIC_STORE(&analyzer->stats.errors_found, 0);
    ATOMIC_STORE(&analyzer->stats.warnings_issued, 0);
    ATOMIC_STORE(&analyzer->stats.max_scope_depth, 0);
    ATOMIC_STORE(&analyzer->stats.current_scope_depth, 0);
}

void semantic_print_statistics(const SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    printf("Semantic Analysis Statistics:\n");
    printf("  Nodes analyzed: %llu\n", (unsigned long long)ATOMIC_LOAD(&analyzer->stats.nodes_analyzed));
    printf("  Types checked: %llu\n", (unsigned long long)ATOMIC_LOAD(&analyzer->stats.types_checked));
    printf("  Symbols resolved: %llu\n", (unsigned long long)ATOMIC_LOAD(&analyzer->stats.symbols_resolved));
    printf("  Errors found: %llu\n", (unsigned long long)ATOMIC_LOAD(&analyzer->stats.errors_found));
    printf("  Warnings issued: %llu\n", (unsigned long long)ATOMIC_LOAD(&analyzer->stats.warnings_issued));
    printf("  Max scope depth: %u\n", (unsigned int)ATOMIC_LOAD(&analyzer->stats.max_scope_depth));
} 
