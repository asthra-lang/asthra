/**
 * Asthra Programming Language Compiler
 * Global Symbol Table for Const Declarations - Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Phase 3: Code Generation Implementation
 * Interface for managing global symbols for const declarations
 */

#ifndef ASTHRA_GLOBAL_SYMBOLS_H
#define ASTHRA_GLOBAL_SYMBOLS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct FFIAssemblyGenerator FFIAssemblyGenerator;
typedef struct GlobalSymbolTable GlobalSymbolTable;
typedef struct GlobalSymbolEntry GlobalSymbolEntry;

// =============================================================================
// TYPES AND CONSTANTS
// =============================================================================

#define INITIAL_GLOBAL_SYMBOL_CAPACITY 64
#define LOAD_FACTOR_THRESHOLD 0.75

/**
 * Symbol visibility types
 */
typedef enum {
    GLOBAL_SYMBOL_PRIVATE,      // Private to current module
    GLOBAL_SYMBOL_PUBLIC        // Public, visible to other modules
} GlobalSymbolVisibility;

/**
 * Const value types for symbol classification
 */
typedef enum {
    CONST_VALUE_INTEGER,        // Integer constant
    CONST_VALUE_FLOAT,          // Float constant
    CONST_VALUE_STRING,         // String constant
    CONST_VALUE_BOOLEAN,        // Boolean constant
    CONST_VALUE_CHAR,           // Character constant
    CONST_VALUE_COMPUTED        // Computed/expression constant
} ConstValueType;

/**
 * Global symbol entry
 */
struct GlobalSymbolEntry {
    char* name;                          // Symbol name
    char* type;                          // Symbol type (C type string or "define")
    GlobalSymbolVisibility visibility;   // Symbol visibility
    ConstValueType value_type;           // Value type classification
    struct GlobalSymbolEntry* next;     // Hash table chaining
};

/**
 * Global symbol table
 */
struct GlobalSymbolTable {
    GlobalSymbolEntry** entries;    // Hash table entries
    size_t capacity;                // Current capacity
    size_t size;                    // Number of symbols
};

/**
 * Symbol table iterator
 */
typedef struct {
    GlobalSymbolTable* table;       // Table being iterated
    size_t bucket_index;           // Current bucket index
    GlobalSymbolEntry* current_entry; // Current entry
    bool finished;                 // Iterator finished flag
} GlobalSymbolIterator;

// =============================================================================
// GLOBAL SYMBOL TABLE API
// =============================================================================

/**
 * Create a new global symbol table
 * @return New symbol table instance, or NULL on failure
 */
GlobalSymbolTable* global_symbol_table_create(void);

/**
 * Destroy a global symbol table and free all memory
 * @param table The symbol table to destroy
 */
void global_symbol_table_destroy(GlobalSymbolTable* table);

/**
 * Add a symbol to the global symbol table
 * @param table The symbol table
 * @param name Symbol name
 * @param type Symbol type
 * @param visibility Symbol visibility
 * @param value_type Const value type
 * @return true if symbol added successfully, false otherwise
 */
bool global_symbol_table_add(GlobalSymbolTable* table, const char* name, const char* type, 
                             GlobalSymbolVisibility visibility, ConstValueType value_type);

/**
 * Look up a symbol in the global symbol table
 * @param table The symbol table
 * @param name Symbol name to look up
 * @return Symbol entry if found, NULL otherwise
 */
GlobalSymbolEntry* global_symbol_table_lookup(GlobalSymbolTable* table, const char* name);

/**
 * Get all public symbols from the global symbol table
 * @param table The symbol table
 * @param count Output parameter for number of public symbols
 * @return Array of public symbol entries, caller must free
 */
GlobalSymbolEntry** global_symbol_table_get_public_symbols(GlobalSymbolTable* table, size_t* count);

/**
 * Create an iterator for the global symbol table
 * @param table The symbol table to iterate
 * @return Iterator instance
 */
GlobalSymbolIterator global_symbol_table_iterator_create(GlobalSymbolTable* table);

/**
 * Get the next symbol from the iterator
 * @param iterator The iterator
 * @return Next symbol entry, or NULL if finished
 */
GlobalSymbolEntry* global_symbol_table_iterator_next(GlobalSymbolIterator* iterator);

/**
 * Check if the iterator has finished
 * @param iterator The iterator
 * @return true if iterator is finished, false otherwise
 */
bool global_symbol_table_iterator_finished(GlobalSymbolIterator* iterator);

// =============================================================================
// FFI ASSEMBLY GENERATOR INTEGRATION
// =============================================================================

/**
 * Add a const symbol to the global symbol table
 * @param generator The FFI assembly generator (can be NULL for testing)
 * @param symbol_name The name of the constant symbol
 * @param symbol_type The type of the constant symbol
 * @param is_public Whether the symbol is publicly visible
 * @return true if symbol added successfully, false otherwise
 */
bool ffi_add_global_const_symbol(FFIAssemblyGenerator* generator, const char* symbol_name, 
                                 const char* symbol_type, bool is_public);

/**
 * Check if a const symbol exists in the global symbol table
 * @param generator The FFI assembly generator (can be NULL for testing)
 * @param symbol_name The name of the symbol to check
 * @return true if symbol exists, false otherwise
 */
bool ffi_const_symbol_exists(FFIAssemblyGenerator* generator, const char* symbol_name);

/**
 * Check if a const symbol is public
 * @param generator The FFI assembly generator (can be NULL for testing)
 * @param symbol_name The name of the symbol to check
 * @return true if symbol exists and is public, false otherwise
 */
bool ffi_const_symbol_is_public(FFIAssemblyGenerator* generator, const char* symbol_name);

/**
 * Cleanup global symbol table (for testing)
 */
void global_symbols_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_GLOBAL_SYMBOLS_H 
