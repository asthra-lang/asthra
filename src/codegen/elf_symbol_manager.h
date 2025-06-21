/**
 * Asthra Programming Language Compiler
 * ELF Symbol Manager
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * High-level symbol management API for ELF generation tests
 */

#ifndef ASTHRA_ELF_SYMBOL_MANAGER_H
#define ASTHRA_ELF_SYMBOL_MANAGER_H

#include "elf_writer.h"
#include "elf_writer_core.h"
#include "elf_writer_symbols.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// SYMBOL MANAGER TYPE
// =============================================================================

/**
 * Symbol manager structure for high-level symbol operations
 */
typedef struct ELFSymbolManager {
    ELFWriter* writer;
    size_t symbol_count;
    size_t symbol_capacity;
    ELFSymbol** symbols;
    bool is_finalized;
} ELFSymbolManager;

// =============================================================================
// SYMBOL MANAGER LIFECYCLE
// =============================================================================

/**
 * Create a new symbol manager
 */
ELFSymbolManager* elf_symbol_manager_create(void);

/**
 * Destroy symbol manager and free resources
 */
void elf_symbol_manager_destroy(ELFSymbolManager* manager);

// =============================================================================
// SYMBOL OPERATIONS
// =============================================================================

/**
 * Add symbol to manager
 */
bool elf_symbol_manager_add_symbol(ELFSymbolManager* manager, const ELFSymbol* symbol);

/**
 * Lookup symbol by name
 */
ELFSymbol* elf_symbol_manager_lookup_symbol(ELFSymbolManager* manager, const char* name);

/**
 * Get symbol index by name
 */
size_t elf_symbol_manager_get_symbol_index(ELFSymbolManager* manager, const char* name);

/**
 * Count global symbols
 */
size_t elf_symbol_manager_count_global_symbols(ELFSymbolManager* manager);

// =============================================================================
// TABLE GENERATION
// =============================================================================

/**
 * Generate symbol table
 */
bool elf_symbol_manager_generate_table(ELFSymbolManager* manager, 
                                       ELFSymbol*** symbol_table, 
                                       size_t* symbol_count);

/**
 * Generate dynamic symbol table
 */
bool elf_symbol_manager_generate_dynamic_table(ELFSymbolManager* manager,
                                               ELFSymbol*** dynsym_table,
                                               size_t* dynsym_count);

/**
 * Generate sorted symbol table
 */
bool elf_symbol_manager_generate_sorted_table(ELFSymbolManager* manager,
                                              ELFSymbol*** symbol_table,
                                              size_t* symbol_count);

// =============================================================================
// VALIDATION
// =============================================================================

/**
 * Validate symbol bindings
 */
bool elf_symbol_manager_validate_bindings(ELFSymbolManager* manager);

/**
 * Validate symbol table consistency
 */
bool elf_symbol_manager_validate_table(ELFSymbolManager* manager);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ELF_SYMBOL_MANAGER_H 
