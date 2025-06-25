/**
 * Asthra Programming Language - Immutable-by-Default Infrastructure
 *
 * Common definitions and structures for immutability testing
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef IMMUTABLE_INFRASTRUCTURE_H
#define IMMUTABLE_INFRASTRUCTURE_H

#include "../framework/test_framework.h"
#include "code_generator_core.h"
#include "parser_string_interface.h"
#include "semantic_analyzer_core.h"
#include "semantic_core.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// =============================================================================
// MUTABILITY DEFINITIONS
// =============================================================================

// Mutability annotations and contexts
typedef enum {
    MUTABILITY_IMMUTABLE, // Default - cannot be modified
    MUTABILITY_MUTABLE,   // Explicitly marked as mutable
    MUTABILITY_CONST,     // Compile-time constant
    MUTABILITY_SHARED,    // Shared immutable reference
    MUTABILITY_UNIQUE     // Unique mutable reference
} MutabilityKind;

typedef struct {
    MutabilityKind kind;
    bool is_deep_immutable;          // Immutability extends to referenced data
    bool allows_interior_mutability; // Cell/RefCell pattern
    const char *mutability_source;   // Where mutability was declared
    int source_line;
    int source_column;
} MutabilityInfo;

typedef struct {
    bool enforce_immutability;
    bool allow_interior_mutability;
    bool enable_copy_on_write;
    bool strict_mode; // No escape hatches
    size_t max_mutation_violations;
} ImmutabilityConfig;

typedef struct {
    const char *variable_name;
    MutabilityInfo mutability;
    const char *violation_reason;
    int line;
    int column;
} MutabilityViolation;

typedef struct {
    MutabilityViolation *violations;
    size_t violation_count;
    size_t max_violations;
    bool analysis_successful;
    double analysis_time_ms;
} ImmutabilityAnalysisResult;

// =============================================================================
// ENHANCED STRUCTURES
// =============================================================================

// Enhanced structures with immutability support
typedef struct EnhancedASTNode {
    int type;
    void *data;
    struct EnhancedASTNode *next_sibling;
    struct EnhancedASTNode *first_child;

    // Immutability metadata
    MutabilityInfo mutability_info;
    bool is_mutation_site;
    bool requires_copy_on_write;
    atomic_int reference_count; // For COW optimization
} EnhancedASTNode;

typedef struct ImmutableParser {
    const char *source;
    size_t position;
    bool has_error;
    char error_message[512];

    // Immutability parsing state
    bool in_mutable_context;
    bool parsing_mut_declaration;
    size_t immutable_bindings_count;
    size_t mutable_bindings_count;
} ImmutableParser;

typedef struct ImmutableSemanticAnalyzer {
    bool has_error;
    char error_message[512];

    // Real semantic analyzer integration
    SemanticAnalyzer *base_analyzer;

    // Immutability analysis state
    ImmutabilityConfig config;
    MutabilityViolation *violations;
    size_t violation_count;
    size_t max_violations;

    // Symbol table integration for mutability tracking
    struct {
        const char **immutable_symbols;
        const char **mutable_symbols;
        size_t immutable_count;
        size_t mutable_count;
    } symbol_mutability;
} ImmutableSemanticAnalyzer;

typedef struct ImmutableCodeGenerator {
    FILE *output;
    bool has_error;
    char error_message[512];

    // Real code generator integration
    CodeGenerator *base_generator;

    // Immutability code generation
    bool generate_cow_support;
    bool generate_immutable_checks;
    size_t cow_operations_generated;
    size_t immutable_structures_created;
} ImmutableCodeGenerator;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Time utility
double get_time_ms(void);

// Analysis functions
bool immutable_analyze_variable_declaration(ImmutableSemanticAnalyzer *analyzer,
                                            EnhancedASTNode *var_decl, const char *variable_name);
bool immutable_analyze_mutation(ImmutableSemanticAnalyzer *analyzer, EnhancedASTNode *mutation_site,
                                const char *variable_name);
bool immutable_analyze_borrow(ImmutableSemanticAnalyzer *analyzer, EnhancedASTNode *borrow_site,
                              const char *variable_name, bool is_mutable_borrow);
ImmutabilityAnalysisResult immutable_analyze_program(ImmutableSemanticAnalyzer *analyzer,
                                                     EnhancedASTNode *program_ast);

// Code generation functions
bool immutable_generate_cow_support(ImmutableCodeGenerator *generator, EnhancedASTNode *cow_node);
bool immutable_generate_structure(ImmutableCodeGenerator *generator, EnhancedASTNode *struct_node,
                                  const char *struct_name);
bool immutable_generate_program(ImmutableCodeGenerator *generator, EnhancedASTNode *program_ast);

// Creation and cleanup functions
ImmutableSemanticAnalyzer *immutable_analyzer_create(ImmutabilityConfig config);
ImmutableCodeGenerator *immutable_generator_create(const char *output_path);
void immutable_analyzer_destroy(ImmutableSemanticAnalyzer *analyzer);
void immutable_generator_destroy(ImmutableCodeGenerator *generator);

// Test functions
bool test_immutable_variable_declaration(void);
bool test_mutation_violation_detection(void);
bool test_cow_code_generation(void);
bool test_immutability_performance(void);
bool test_memory_usage_optimization(void);
bool run_immutable_by_default_tests(void);

#endif // IMMUTABLE_INFRASTRUCTURE_H