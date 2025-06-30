/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Analyzer Core
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main semantic analyzer structure and core operation declarations
 */

#ifndef ASTHRA_SEMANTIC_ANALYZER_CORE_H
#define ASTHRA_SEMANTIC_ANALYZER_CORE_H

#include "../parser/ast.h"
#include "../parser/ast_types.h"
#include "semantic_errors_defs.h"
#include "semantic_symbols_defs.h"
#include "semantic_types_defs.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// C17 static assertions for semantic analysis assumptions
_Static_assert(sizeof(size_t) >= sizeof(uint32_t),
               "size_t must be at least 32-bit for symbol table");
_Static_assert(__STDC_VERSION__ >= 201710L,
               "C17 standard required for semantic analysis modernization");

// =============================================================================
// SEMANTIC ANALYZER STRUCTURE
// =============================================================================

typedef struct ImportedModule {
    char *path;
    SourceLocation location;  // Location of the import declaration
} ImportedModule;

typedef struct SemanticAnalyzer {
    SymbolTable *global_scope;
    SymbolTable *current_scope;
    TypeDescriptor **builtin_types;
    size_t builtin_type_count;

    // Predeclared identifiers (log, range, etc.)
    PredeclaredIdentifier *predeclared_identifiers;
    size_t predeclared_count;

    // Import tracking
    ImportedModule *imported_modules;
    size_t imported_count;
    size_t imported_capacity;

    // Error handling
    SemanticError *errors;
    SemanticError *last_error;
    size_t error_count;
    size_t max_errors;

    // Statistics
    SemanticStatistics stats;

    // Context tracking
    bool in_unsafe_context;        // Track if we're currently inside an unsafe block
    SymbolEntry *current_function; // Track the function currently being analyzed
    size_t loop_depth;             // Track nesting depth of loops for break/continue validation
    bool in_unreachable_code;      // Track if we're analyzing unreachable code
    TypeDescriptor *expected_type; // Track expected type for context-based type inference

    // Configuration
    struct {
        bool strict_mode;
        bool allow_unsafe;
        bool check_ownership;
        bool validate_ffi;
        bool enable_warnings;
        bool test_mode; // More permissive mode for test contexts
    } config;
} SemanticAnalyzer;

// =============================================================================
// CORE SEMANTIC ANALYSIS FUNCTIONS
// =============================================================================

// Analyzer lifecycle
SemanticAnalyzer *semantic_analyzer_create(void);
void semantic_analyzer_destroy(SemanticAnalyzer *analyzer);
void semantic_analyzer_reset(SemanticAnalyzer *analyzer);

// Configuration
void semantic_analyzer_set_test_mode(SemanticAnalyzer *analyzer, bool enable);

// Main analysis entry points
bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program);
bool semantic_analyze_declaration(SemanticAnalyzer *analyzer, ASTNode *decl);
bool semantic_analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool semantic_analyze_expression(SemanticAnalyzer *analyzer, ASTNode *expr);

// Type checking
TypeDescriptor *semantic_get_expression_type(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_check_type_compatibility(SemanticAnalyzer *analyzer, TypeDescriptor *expected,
                                       TypeDescriptor *actual);
bool semantic_can_cast(SemanticAnalyzer *analyzer, TypeDescriptor *from, TypeDescriptor *to);

// Symbol resolution
SymbolEntry *semantic_resolve_identifier(SemanticAnalyzer *analyzer, const char *name);
bool semantic_declare_symbol(SemanticAnalyzer *analyzer, const char *name, SymbolKind kind,
                             TypeDescriptor *type, ASTNode *declaration);

// Scope management
void semantic_enter_scope(SemanticAnalyzer *analyzer);
void semantic_exit_scope(SemanticAnalyzer *analyzer);
uint32_t semantic_get_current_scope_id(SemanticAnalyzer *analyzer);

// =============================================================================
// TYPE SYSTEM FUNCTIONS
// =============================================================================

// Type descriptor management
TypeDescriptor *type_descriptor_create_primitive(int primitive_kind);
TypeDescriptor *type_descriptor_create_struct(const char *struct_name, size_t field_count);
TypeDescriptor *type_descriptor_create_slice(TypeDescriptor *element_type);
TypeDescriptor *type_descriptor_create_result(TypeDescriptor *ok_type, TypeDescriptor *err_type);
TypeDescriptor *type_descriptor_create_pointer(TypeDescriptor *pointee_type);
TypeDescriptor *type_descriptor_create_function(void);
TypeDescriptor *type_descriptor_create_function_with_params(TypeDescriptor *return_type,
                                                            size_t param_count);
void type_descriptor_retain(TypeDescriptor *type);
void type_descriptor_release(TypeDescriptor *type);
bool type_descriptor_equals(const TypeDescriptor *type1, const TypeDescriptor *type2);
size_t type_descriptor_hash(const TypeDescriptor *type);

// Built-in types
void semantic_init_builtin_types(SemanticAnalyzer *analyzer);
TypeDescriptor *semantic_get_builtin_type(SemanticAnalyzer *analyzer, const char *name);

// Predeclared identifiers
void semantic_init_predeclared_identifiers(SemanticAnalyzer *analyzer);

// =============================================================================
// SYMBOL TABLE FUNCTIONS WITH THREAD SAFETY
// =============================================================================

// Symbol table lifecycle
SymbolTable *symbol_table_create(size_t initial_capacity);
void symbol_table_destroy(SymbolTable *table);
SymbolTable *symbol_table_create_child(SymbolTable *parent);

// Thread-safe symbol operations
bool symbol_table_insert_safe(SymbolTable *table, const char *name, SymbolEntry *entry);
SymbolEntry *symbol_table_lookup_safe(SymbolTable *table, const char *name);
bool symbol_table_remove_safe(SymbolTable *table, const char *name);

// Symbol entry management
SymbolEntry *symbol_entry_create(const char *name, SymbolKind kind, TypeDescriptor *type,
                                 ASTNode *declaration);
void symbol_entry_destroy(SymbolEntry *entry);

// =============================================================================
// MODULE ALIAS MANAGEMENT
// =============================================================================

// Alias management functions
bool symbol_table_add_alias(SymbolTable *table, const char *alias_name, const char *module_path,
                            SymbolTable *module_symbols);
SymbolTable *symbol_table_resolve_alias(SymbolTable *table, const char *alias_name);
void symbol_table_clear_aliases(SymbolTable *table);

// =============================================================================
// ERROR HANDLING AND REPORTING
// =============================================================================

// Error management
void semantic_report_error(SemanticAnalyzer *analyzer, SemanticErrorCode code,
                           SourceLocation location, const char *format, ...);
void semantic_report_warning(SemanticAnalyzer *analyzer, SourceLocation location,
                             const char *format, ...);

// Error retrieval
const SemanticError *semantic_get_errors(const SemanticAnalyzer *analyzer);
size_t semantic_get_error_count(const SemanticAnalyzer *analyzer);
void semantic_clear_errors(SemanticAnalyzer *analyzer);

// Error utilities
const char *semantic_error_code_name(SemanticErrorCode code);
const char *semantic_error_code_description(SemanticErrorCode code);

// =============================================================================
// VALIDATION AND ANALYSIS HELPERS
// =============================================================================

// Ownership analysis
bool semantic_check_ownership_transfer(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_check_borrow_validity(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_validate_lifetime(SemanticAnalyzer *analyzer, ASTNode *expr);

// FFI validation
bool semantic_validate_ffi_type(SemanticAnalyzer *analyzer, TypeDescriptor *type);
bool semantic_check_ffi_annotations(SemanticAnalyzer *analyzer, ASTNodeList *annotations);
bool semantic_validate_extern_function(SemanticAnalyzer *analyzer, ASTNode *extern_decl);

// Security annotation validation
bool semantic_check_security_annotations(SemanticAnalyzer *analyzer, ASTNodeList *annotations);
bool semantic_validate_security_annotation_context(SemanticAnalyzer *analyzer, ASTNode *node,
                                                   SecurityType security_type);
bool semantic_validate_constant_time_function(SemanticAnalyzer *analyzer, ASTNode *func_decl);
bool semantic_validate_constant_time_block(SemanticAnalyzer *analyzer, ASTNode *block);
bool semantic_validate_constant_time_statement(SemanticAnalyzer *analyzer, ASTNode *stmt);
bool semantic_validate_constant_time_call(SemanticAnalyzer *analyzer, ASTNode *call_expr);
bool semantic_validate_constant_time_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_validate_ffi_constant_time_safety(SemanticAnalyzer *analyzer, ASTNode *extern_decl);
bool semantic_validate_ffi_parameter_constant_time_safety(SemanticAnalyzer *analyzer,
                                                          ASTNode *param);
bool semantic_validate_volatile_memory_access(SemanticAnalyzer *analyzer, ASTNode *node);
bool semantic_validate_ffi_volatile_memory_safety(SemanticAnalyzer *analyzer, ASTNode *call_expr);
bool semantic_validate_volatile_memory_argument(SemanticAnalyzer *analyzer, ASTNode *arg);
bool semantic_check_volatile_assignment(SemanticAnalyzer *analyzer, ASTNode *assignment);
bool semantic_check_volatile_field_access(SemanticAnalyzer *analyzer, ASTNode *field_access);
bool semantic_check_volatile_dereference(SemanticAnalyzer *analyzer, ASTNode *deref);
bool semantic_has_volatile_annotation(ASTNode *declaration);
bool semantic_has_constant_time_annotation(ASTNode *declaration);

// Pattern matching analysis
bool semantic_check_pattern_exhaustiveness(SemanticAnalyzer *analyzer, ASTNode *match_stmt);
bool semantic_validate_pattern_types(SemanticAnalyzer *analyzer, ASTNode *pattern,
                                     TypeDescriptor *expected);

// Expression analysis
bool semantic_is_lvalue(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_is_constant_expression(SemanticAnalyzer *analyzer, ASTNode *expr);
bool semantic_has_side_effects(SemanticAnalyzer *analyzer, ASTNode *expr);

// Internal helper functions
TypeDescriptor *analyze_type_node(SemanticAnalyzer *analyzer, ASTNode *type_node);

// =============================================================================
// STATISTICS AND PROFILING
// =============================================================================

// Statistics access
SemanticStatistics semantic_get_statistics(const SemanticAnalyzer *analyzer);
void semantic_reset_statistics(SemanticAnalyzer *analyzer);
void semantic_print_statistics(const SemanticAnalyzer *analyzer);

// Performance profiling
void semantic_start_profiling(SemanticAnalyzer *analyzer);
void semantic_stop_profiling(SemanticAnalyzer *analyzer);
double semantic_get_analysis_time(const SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANALYZER_CORE_H
