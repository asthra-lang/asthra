/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Relocation Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ELF_WRITER_RELOCATIONS_H
#define ASTHRA_ELF_WRITER_RELOCATIONS_H

#include "elf_writer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RELOCATION MANAGEMENT FUNCTIONS
// =============================================================================

/**
 * Add relocation entry with Asthra-specific metadata
 */
ELFRelocation *elf_add_relocation(ELFWriter *writer, uint64_t offset,
                                 uint32_t type, uint32_t symbol, int64_t addend);

/**
 * Add FFI call relocation
 */
bool elf_add_ffi_call_relocation(ELFWriter *writer, uint64_t call_site,
                                const char *function_name);

/**
 * Add pattern match jump relocation
 */
bool elf_add_pattern_match_relocation(ELFWriter *writer, uint64_t jump_site,
                                     const char *target_label);

/**
 * Add string operation runtime call relocation
 */
bool elf_add_string_op_relocation(ELFWriter *writer, uint64_t call_site,
                                 const char *runtime_function);

/**
 * Add slice bounds check relocation
 */
bool elf_add_slice_bounds_relocation(ELFWriter *writer, uint64_t check_site,
                                    const char *bounds_check_function);

/**
 * Add spawn call relocation
 */
bool elf_add_spawn_relocation(ELFWriter *writer, uint64_t spawn_site,
                             const char *scheduler_function);

/**
 * Add relative call relocation (for local function calls)
 */
bool elf_add_relative_call_relocation(ELFWriter *writer, uint64_t call_site,
                                     const char *function_name);

/**
 * Add absolute address relocation (for data references)
 */
bool elf_add_absolute_relocation(ELFWriter *writer, uint64_t reference_site,
                                const char *symbol_name);

/**
 * Add PC-relative data relocation
 */
bool elf_add_pc_relative_relocation(ELFWriter *writer, uint64_t reference_site,
                                   const char *symbol_name);

/**
 * Add GOT (Global Offset Table) relocation
 */
bool elf_add_got_relocation(ELFWriter *writer, uint64_t reference_site,
                           const char *symbol_name);

/**
 * Add PLT (Procedure Linkage Table) relocation
 */
bool elf_add_plt_relocation(ELFWriter *writer, uint64_t call_site,
                           const char *function_name);

/**
 * Find relocation by offset
 */
ELFRelocation *elf_find_relocation_by_offset(ELFWriter *writer, uint64_t offset);

/**
 * Find relocations by symbol
 */
ELFRelocation **elf_find_relocations_by_symbol(ELFWriter *writer, uint32_t symbol_index,
                                              size_t *count);

/**
 * Update relocation addend
 */
bool elf_update_relocation_addend(ELFWriter *writer, uint64_t offset, int64_t new_addend);

/**
 * Get relocations by type
 */
ELFRelocation **elf_get_relocations_by_type(ELFWriter *writer, uint32_t type,
                                           size_t *count);

/**
 * Get all FFI call relocations
 */
ELFRelocation **elf_get_ffi_call_relocations(ELFWriter *writer, size_t *count);

/**
 * Get all pattern match relocations
 */
ELFRelocation **elf_get_pattern_match_relocations(ELFWriter *writer, size_t *count);

/**
 * Get all string operation relocations
 */
ELFRelocation **elf_get_string_op_relocations(ELFWriter *writer, size_t *count);

/**
 * Get all slice bounds check relocations
 */
ELFRelocation **elf_get_slice_bounds_relocations(ELFWriter *writer, size_t *count);

/**
 * Get all spawn call relocations
 */
ELFRelocation **elf_get_spawn_relocations(ELFWriter *writer, size_t *count);

/**
 * Validate relocation table consistency
 */
bool elf_validate_relocation_table(ELFWriter *writer);

/**
 * Sort relocations by offset
 */
void elf_sort_relocations_by_offset(ELFWriter *writer);

/**
 * Sort relocations by type
 */
void elf_sort_relocations_by_type(ELFWriter *writer);

/**
 * Apply relocations (for testing/validation)
 */
bool elf_apply_relocations(ELFWriter *writer);

/**
 * Count relocations by section
 */
size_t elf_count_relocations_for_section(ELFWriter *writer, uint16_t section_index);

/**
 * Generate relocation section data
 */
bool elf_generate_relocation_section_data(ELFWriter *writer, uint16_t section_index);

/**
 * Print relocation table for debugging
 */
void elf_print_relocation_table(ELFWriter *writer);

/**
 * Print relocations for specific section
 */
void elf_print_section_relocations(ELFWriter *writer, uint16_t section_index);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ELF_WRITER_RELOCATIONS_H 
