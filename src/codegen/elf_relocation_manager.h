/**
 * Asthra Programming Language Compiler
 * ELF Relocation Manager
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * High-level relocation management API for ELF generation tests
 */

#ifndef ASTHRA_ELF_RELOCATION_MANAGER_H
#define ASTHRA_ELF_RELOCATION_MANAGER_H

#include "elf_writer.h"
#include "elf_writer_core.h"
#include "elf_writer_relocations.h"
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// RELOCATION MANAGER TYPE
// =============================================================================

/**
 * Relocation manager structure for high-level relocation operations
 */
typedef struct ELFRelocationManager {
    ELFWriter* writer;
    size_t relocation_count;
    size_t relocation_capacity;
    ELFRelocation** relocations;
    bool is_finalized;
} ELFRelocationManager;

// =============================================================================
// RELOCATION MANAGER LIFECYCLE
// =============================================================================

/**
 * Create a new relocation manager
 */
ELFRelocationManager* elf_relocation_manager_create(void);

/**
 * Destroy relocation manager and free resources
 */
void elf_relocation_manager_destroy(ELFRelocationManager* manager);

// =============================================================================
// RELOCATION OPERATIONS
// =============================================================================

/**
 * Add relocation to manager
 */
bool elf_relocation_manager_add_relocation(ELFRelocationManager* manager, 
                                           const ELFRelocation* relocation);

/**
 * Add FFI call relocation
 */
bool elf_relocation_manager_add_ffi_call(ELFRelocationManager* manager,
                                         uint64_t call_site,
                                         const char* function_name);

/**
 * Add pattern match relocation
 */
bool elf_relocation_manager_add_pattern_match(ELFRelocationManager* manager,
                                              uint64_t jump_site,
                                              const char* target_label);

/**
 * Add string operation relocation
 */
bool elf_relocation_manager_add_string_op(ELFRelocationManager* manager,
                                          uint64_t call_site,
                                          const char* runtime_function);

/**
 * Add slice bounds check relocation
 */
bool elf_relocation_manager_add_slice_bounds(ELFRelocationManager* manager,
                                             uint64_t check_site,
                                             const char* bounds_check_function);

/**
 * Add spawn relocation
 */
bool elf_relocation_manager_add_spawn(ELFRelocationManager* manager,
                                      uint64_t spawn_site,
                                      const char* scheduler_function);

// =============================================================================
// TABLE GENERATION
// =============================================================================

/**
 * Generate relocation table
 */
bool elf_relocation_manager_generate_table(ELFRelocationManager* manager,
                                           ELFRelocation*** relocation_table,
                                           size_t* relocation_count);

/**
 * Generate relocation table for specific section
 */
bool elf_relocation_manager_generate_section_table(ELFRelocationManager* manager,
                                                   uint16_t section_index,
                                                   ELFRelocation*** relocation_table,
                                                   size_t* relocation_count);

// =============================================================================
// VALIDATION
// =============================================================================

/**
 * Validate relocation table consistency
 */
bool elf_relocation_manager_validate_table(ELFRelocationManager* manager);

/**
 * Validate relocation types
 */
bool elf_relocation_manager_validate_types(ELFRelocationManager* manager);

/**
 * Process AST for relocations
 */
bool elf_relocation_manager_process_ast(ELFRelocationManager* manager, ASTNode* ast);

/**
 * Get relocation count
 */
size_t elf_relocation_manager_get_count(ELFRelocationManager* manager);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ELF_RELOCATION_MANAGER_H 
