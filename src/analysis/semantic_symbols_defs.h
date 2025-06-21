/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Symbol Table Definitions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Symbol table structures, symbol entries, and related definitions
 */

#ifndef ASTHRA_SEMANTIC_SYMBOLS_DEFS_H
#define ASTHRA_SEMANTIC_SYMBOLS_DEFS_H

#include "semantic_types_defs.h"
#include "../parser/ast.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declaration for const value support
typedef struct ConstValue ConstValue;

// =============================================================================
// SYMBOL TABLE DEFINITIONS WITH C17 ATOMIC OPERATIONS
// =============================================================================

// Symbol kinds
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_FUNCTION,
    SYMBOL_TYPE,
    SYMBOL_PARAMETER,
    SYMBOL_FIELD,
    SYMBOL_METHOD,         // NEW: for method symbols
    SYMBOL_ENUM_VARIANT,   // NEW: for enum variant symbols
    SYMBOL_TYPE_PARAMETER, // Phase 3: Generic Structs - for type parameters like T in Vec<T>
    SYMBOL_CONST,          // Phase 2: Const declarations - for compile-time constants
    SYMBOL_COUNT  // For validation
} SymbolKind;

// Module alias structure for import aliases
typedef struct {
    char *alias_name;
    char *module_path;
    SymbolTable *module_symbols;
} ModuleAlias;

// Symbol entry with atomic operations support
typedef struct SymbolEntry {
    char *name;
    SymbolKind kind;
    TypeDescriptor *type;
    ASTNode *declaration;
    uint32_t scope_id;
    
    // Symbol flags
    struct {
        bool is_used : 1;
        bool is_exported : 1;
        bool is_mutable : 1;
        bool is_initialized : 1;
        bool is_predeclared : 1;
        uint8_t reserved : 3;
    } flags;
    
    // Visibility and method-specific information (NEW)
    VisibilityType visibility;     // pub or private
    bool is_instance_method;       // true if method has self parameter
    
    // Phase 3: Generic Structs - support for generic type tracking
    bool is_generic;               // true if this is a generic type (struct/enum)
    size_t type_param_count;       // number of type parameters for generic types
    ASTNode *generic_decl;         // original AST node for generic instantiation
    
    // Phase 2: Const declarations - compile-time constant value
    ConstValue *const_value;       // For SYMBOL_CONST, stores the evaluated const value
    
    struct SymbolEntry *next;  // For hash table chaining
} SymbolEntry;

// Thread-safe symbol table with atomic operations
struct SymbolTable {
    SymbolEntry **buckets;
    size_t bucket_count;
    atomic_size_t entry_count;
    atomic_uint_fast32_t scope_counter;
    uint32_t current_scope;
    
    // Thread safety
    pthread_rwlock_t rwlock;
    
    // Parent scope for nested scopes
    SymbolTable *parent;
    
    // Import aliases
    ModuleAlias *aliases;
    size_t alias_count;
    size_t alias_capacity;
};

// Predeclared identifiers structure
typedef struct {
    const char *name;
    const char *signature;      // Function signature string
    SymbolKind kind;
    TypeDescriptor *type;
    bool is_predeclared;
} PredeclaredIdentifier;

// =============================================================================
// SYMBOL TABLE ITERATION
// =============================================================================

// Function pointer type for symbol table iteration
// Returns true to continue iteration, false to stop
typedef bool (*SymbolIteratorFunc)(const char *name, SymbolEntry *entry, void *user_data);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_SYMBOLS_DEFS_H 
