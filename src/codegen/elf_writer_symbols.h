/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Symbol Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ELF_WRITER_SYMBOLS_H
#define ASTHRA_ELF_WRITER_SYMBOLS_H

#include "elf_writer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for context structures
typedef struct FFICallContext FFICallContext;
typedef struct PatternMatchContext PatternMatchContext;
typedef struct StringOperationContext StringOperationContext;
typedef struct SliceOperationContext SliceOperationContext;
typedef struct ConcurrencyContext ConcurrencyContext;

// =============================================================================
// SYMBOL MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Add symbol to symbol table with Asthra metadata
 */
ELFSymbol *elf_add_symbol(ELFWriter *writer, const char *name, 
                         uint64_t value, uint64_t size, 
                         uint8_t type, uint8_t binding, uint16_t section);

/**
 * Add FFI function symbol with enhanced metadata
 */
ELFSymbol *elf_add_ffi_symbol(ELFWriter *writer, const char *name,
                             uint64_t address, FFICallContext *ffi_context);

/**
 * Add pattern matching symbol
 */
ELFSymbol *elf_add_pattern_match_symbol(ELFWriter *writer, const char *name,
                                       uint64_t address, PatternMatchContext *context);

/**
 * Add string operation symbol
 */
ELFSymbol *elf_add_string_op_symbol(ELFWriter *writer, const char *name,
                                   uint64_t address, StringOperationContext *context);

/**
 * Add slice operation symbol
 */
ELFSymbol *elf_add_slice_op_symbol(ELFWriter *writer, const char *name,
                                  uint64_t address, SliceOperationContext *context);

/**
 * Add concurrency spawn point symbol
 */
ELFSymbol *elf_add_spawn_symbol(ELFWriter *writer, const char *name,
                               uint64_t address, ConcurrencyContext *context);

/**
 * Add GC root symbol
 */
ELFSymbol *elf_add_gc_root_symbol(ELFWriter *writer, const char *name,
                                 uint64_t address, size_t size, uint32_t ownership_type);

/**
 * Find symbol by name
 */
ELFSymbol *elf_find_symbol(ELFWriter *writer, const char *name);

/**
 * Find symbol by address
 */
ELFSymbol *elf_find_symbol_by_address(ELFWriter *writer, uint64_t address);

/**
 * Update symbol value/address
 */
bool elf_update_symbol_value(ELFWriter *writer, const char *name, uint64_t new_value);

/**
 * Mark symbol as exported/global
 */
bool elf_mark_symbol_global(ELFWriter *writer, const char *name);

/**
 * Mark symbol as local/private
 */
bool elf_mark_symbol_local(ELFWriter *writer, const char *name);

/**
 * Get symbol count by type
 */
size_t elf_get_symbol_count_by_type(ELFWriter *writer, uint8_t symbol_type);

/**
 * Get all FFI function symbols
 */
ELFSymbol **elf_get_ffi_symbols(ELFWriter *writer, size_t *count);

/**
 * Get all pattern matching symbols
 */
ELFSymbol **elf_get_pattern_match_symbols(ELFWriter *writer, size_t *count);

/**
 * Get all string operation symbols
 */
ELFSymbol **elf_get_string_op_symbols(ELFWriter *writer, size_t *count);

/**
 * Get all slice operation symbols
 */
ELFSymbol **elf_get_slice_op_symbols(ELFWriter *writer, size_t *count);

/**
 * Get all spawn point symbols
 */
ELFSymbol **elf_get_spawn_symbols(ELFWriter *writer, size_t *count);

/**
 * Get all GC root symbols
 */
ELFSymbol **elf_get_gc_root_symbols(ELFWriter *writer, size_t *count);

/**
 * Validate symbol table consistency
 */
bool elf_validate_symbol_table(ELFWriter *writer);

/**
 * Sort symbols by address
 */
void elf_sort_symbols_by_address(ELFWriter *writer);

/**
 * Sort symbols by name
 */
void elf_sort_symbols_by_name(ELFWriter *writer);

/**
 * Print symbol table for debugging
 */
void elf_print_symbol_table(ELFWriter *writer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ELF_WRITER_SYMBOLS_H 
