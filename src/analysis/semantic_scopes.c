/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Scope Management Module Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Scope creation, management, and navigation
 */

#include "semantic_scopes.h"
#include "semantic_symbols.h"

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
    #define ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
    #define ATOMIC_FETCH_SUB(ptr, val) atomic_fetch_sub(ptr, val)
    #define ATOMIC_LOAD(ptr) atomic_load(ptr)
    #define ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired) \
        atomic_compare_exchange_weak(ptr, expected, desired)
#else
    #define ATOMIC_FETCH_ADD(ptr, val) __sync_fetch_and_add(ptr, val)
    #define ATOMIC_FETCH_SUB(ptr, val) __sync_fetch_and_sub(ptr, val)
    #define ATOMIC_LOAD(ptr) (*(ptr))
    #define ATOMIC_COMPARE_EXCHANGE(ptr, expected, desired) \
        __sync_bool_compare_and_swap(ptr, *(expected), desired)
#endif

// =============================================================================
// SCOPE MANAGEMENT
// =============================================================================

void semantic_enter_scope(SemanticAnalyzer *analyzer) {
    if (!analyzer) return;
    
    SymbolTable *new_scope = symbol_table_create_child(analyzer->current_scope);
    if (new_scope) {
        analyzer->current_scope = new_scope;
        uint_fast32_t depth = ATOMIC_FETCH_ADD(&analyzer->stats.current_scope_depth, 1);
        
        // Update max depth if needed
        uint_fast32_t max_depth = ATOMIC_LOAD(&analyzer->stats.max_scope_depth);
        while (depth > max_depth) {
            if (ATOMIC_COMPARE_EXCHANGE(&analyzer->stats.max_scope_depth, &max_depth, depth)) {
                break;
            }
        }
    }
}

void semantic_exit_scope(SemanticAnalyzer *analyzer) {
    if (!analyzer || !analyzer->current_scope || 
        analyzer->current_scope == analyzer->global_scope) {
        return;
    }
    
    SymbolTable *old_scope = analyzer->current_scope;
    analyzer->current_scope = old_scope->parent;
    
    // Don't destroy the scope immediately - it might be referenced
    // In a real implementation, we'd use a more sophisticated cleanup strategy
    
    ATOMIC_FETCH_SUB(&analyzer->stats.current_scope_depth, 1);
}

uint32_t semantic_get_current_scope_id(SemanticAnalyzer *analyzer) {
    if (!analyzer || !analyzer->current_scope) return 0;
    return analyzer->current_scope->current_scope;
} 
