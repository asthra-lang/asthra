/**
 * Asthra Programming Language Compiler
 * ELF Relocation Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for managing ELF relocations with comprehensive Asthra metadata.
 */

#include "elf_relocation_manager.h"
#include "elf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>

// =============================================================================
// RELOCATION MANAGEMENT
// =============================================================================

ELFRelocation *elf_add_relocation(ELFWriter *writer, uint64_t offset,
                                 uint32_t type, uint32_t symbol, int64_t addend) {
    if (!writer) return NULL;
    
    // Expand relocations array if needed
    if (writer->relocation_count >= writer->relocation_capacity) {
        size_t new_capacity = writer->relocation_capacity * 2;
        ELFRelocation **new_relocations = realloc(writer->relocations,
                                                 new_capacity * sizeof(ELFRelocation*));
        if (!new_relocations) return NULL;
        writer->relocations = new_relocations;
        writer->relocation_capacity = new_capacity;
    }
    
    // Create new relocation
    ELFRelocation *relocation = calloc(1, sizeof(ELFRelocation));
    if (!relocation) return NULL;
    
    relocation->offset = offset;
    relocation->info = ELF64_R_INFO(symbol, type);
    relocation->addend = addend;
    
    writer->relocations[writer->relocation_count] = relocation;
    writer->relocation_count++;
    
    return relocation;
}

bool elf_add_ffi_call_relocation(ELFWriter *writer, uint64_t call_site,
                                const char *function_name) {
    if (!writer || !function_name) return false;
    
    // Find or create symbol for the FFI function
    ELFSymbol *symbol = NULL;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, function_name) == 0) {
            symbol = writer->symbols[i];
            break;
        }
    }
    
    if (!symbol) {
        // Create external symbol
        symbol = elf_add_symbol(writer, function_name, 0, 0,
                               STT_FUNC, STB_GLOBAL, SHN_UNDEF);
        if (!symbol) return false;
        symbol->is_ffi_function = true;
    }
    
    // Add relocation (with safe cast to uint32_t)
    ptrdiff_t symbol_index = symbol - writer->symbols[0];
    if (symbol_index < 0 || symbol_index > UINT32_MAX) return false;
    ELFRelocation *relocation = elf_add_relocation(writer, call_site,
                                                  R_X86_64_PLT32, 
                                                  (uint32_t)symbol_index, -4);
    if (!relocation) return false;
    
    relocation->is_ffi_call = true;
    return true;
}

bool elf_add_pattern_match_relocation(ELFWriter *writer, uint64_t jump_site,
                                     const char *target_label) {
    if (!writer || !target_label) return false;
    
    // Find symbol for the target label
    ELFSymbol *symbol = NULL;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, target_label) == 0) {
            symbol = writer->symbols[i];
            break;
        }
    }
    
    if (!symbol) {
        // Create local symbol for the label
        symbol = elf_add_symbol(writer, target_label, 0, 0,
                               STT_NOTYPE, STB_LOCAL, 1);
        if (!symbol) return false;
        symbol->is_pattern_match = true;
    }
    
    // Add relocation (with safe cast to uint32_t)
    ptrdiff_t symbol_index = symbol - writer->symbols[0];
    if (symbol_index < 0 || symbol_index > UINT32_MAX) return false;
    ELFRelocation *relocation = elf_add_relocation(writer, jump_site,
                                                  R_X86_64_PC32,
                                                  (uint32_t)symbol_index, -4);
    if (!relocation) return false;
    
    relocation->is_pattern_match_target = true;
    return true;
}

bool elf_add_string_op_relocation(ELFWriter *writer, uint64_t call_site,
                                 const char *runtime_function) {
    if (!writer || !runtime_function) return false;
    
    // Find or create symbol for the runtime function
    ELFSymbol *symbol = NULL;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, runtime_function) == 0) {
            symbol = writer->symbols[i];
            break;
        }
    }
    
    if (!symbol) {
        // Create external symbol
        symbol = elf_add_symbol(writer, runtime_function, 0, 0,
                               STT_FUNC, STB_GLOBAL, SHN_UNDEF);
        if (!symbol) return false;
    }
    
    // Add relocation (with safe cast to uint32_t)
    ptrdiff_t symbol_index = symbol - writer->symbols[0];
    if (symbol_index < 0 || symbol_index > UINT32_MAX) return false;
    ELFRelocation *relocation = elf_add_relocation(writer, call_site,
                                                  R_X86_64_PLT32,
                                                  (uint32_t)symbol_index, -4);
    if (!relocation) return false;
    
    relocation->is_string_op_call = true;
    return true;
}

bool elf_add_slice_bounds_relocation(ELFWriter *writer, uint64_t check_site,
                                    const char *bounds_check_function) {
    if (!writer || !bounds_check_function) return false;
    
    // Similar to string operation relocation
    ELFSymbol *symbol = NULL;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, bounds_check_function) == 0) {
            symbol = writer->symbols[i];
            break;
        }
    }
    
    if (!symbol) {
        symbol = elf_add_symbol(writer, bounds_check_function, 0, 0,
                               STT_FUNC, STB_GLOBAL, SHN_UNDEF);
        if (!symbol) return false;
    }
    
    // Add relocation (with safe cast to uint32_t)
    ptrdiff_t symbol_index = symbol - writer->symbols[0];
    if (symbol_index < 0 || symbol_index > UINT32_MAX) return false;
    ELFRelocation *relocation = elf_add_relocation(writer, check_site,
                                                  R_X86_64_PLT32,
                                                  (uint32_t)symbol_index, -4);
    if (!relocation) return false;
    
    relocation->is_slice_bounds_check = true;
    return true;
}

bool elf_add_spawn_relocation(ELFWriter *writer, uint64_t spawn_site,
                             const char *scheduler_function) {
    if (!writer || !scheduler_function) return false;
    
    // Similar to other runtime function relocations
    ELFSymbol *symbol = NULL;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, scheduler_function) == 0) {
            symbol = writer->symbols[i];
            break;
        }
    }
    
    if (!symbol) {
        symbol = elf_add_symbol(writer, scheduler_function, 0, 0,
                               STT_FUNC, STB_GLOBAL, SHN_UNDEF);
        if (!symbol) return false;
    }
    
    // Add relocation (with safe cast to uint32_t)
    ptrdiff_t symbol_index = symbol - writer->symbols[0];
    if (symbol_index < 0 || symbol_index > UINT32_MAX) return false;
    ELFRelocation *relocation = elf_add_relocation(writer, spawn_site,
                                                  R_X86_64_PLT32,
                                                  (uint32_t)symbol_index, -4);
    if (!relocation) return false;
    
    relocation->is_spawn_call = true;
    return true;
}

// =============================================================================
// RELOCATION MANAGER IMPLEMENTATION
// =============================================================================

ELFRelocationManager* elf_relocation_manager_create(void) {
    ELFRelocationManager* manager = calloc(1, sizeof(ELFRelocationManager));
    if (!manager) return NULL;
    
    // Create an ELF writer for internal use
    manager->writer = elf_writer_create(NULL);
    if (!manager->writer) {
        free(manager);
        return NULL;
    }
    
    manager->relocation_capacity = 64;
    manager->relocations = calloc(manager->relocation_capacity, sizeof(ELFRelocation*));
    if (!manager->relocations) {
        elf_writer_destroy(manager->writer);
        free(manager);
        return NULL;
    }
    
    manager->relocation_count = 0;
    manager->is_finalized = false;
    
    return manager;
}

void elf_relocation_manager_destroy(ELFRelocationManager* manager) {
    if (!manager) return;
    
    if (manager->relocations) {
        for (size_t i = 0; i < manager->relocation_count; i++) {
            free(manager->relocations[i]);
        }
        free(manager->relocations);
    }
    
    if (manager->writer) {
        elf_writer_destroy(manager->writer);
    }
    
    free(manager);
}

bool elf_relocation_manager_add_relocation(ELFRelocationManager* manager, 
                                           const ELFRelocation* relocation) {
    if (!manager || !relocation || manager->is_finalized) return false;
    
    // Expand array if needed
    if (manager->relocation_count >= manager->relocation_capacity) {
        size_t new_capacity = manager->relocation_capacity * 2;
        ELFRelocation** new_relocations = realloc(manager->relocations, 
                                                 new_capacity * sizeof(ELFRelocation*));
        if (!new_relocations) return false;
        manager->relocations = new_relocations;
        manager->relocation_capacity = new_capacity;
    }
    
    // Create a copy of the relocation
    ELFRelocation* relocation_copy = calloc(1, sizeof(ELFRelocation));
    if (!relocation_copy) return false;
    
    *relocation_copy = *relocation;
    
    manager->relocations[manager->relocation_count] = relocation_copy;
    manager->relocation_count++;
    
    return true;
}

bool elf_relocation_manager_add_ffi_call(ELFRelocationManager* manager,
                                         uint64_t call_site,
                                         const char* function_name) {
    if (!manager || !function_name) return false;
    
    return elf_add_ffi_call_relocation(manager->writer, call_site, function_name);
}

bool elf_relocation_manager_add_pattern_match(ELFRelocationManager* manager,
                                              uint64_t jump_site,
                                              const char* target_label) {
    if (!manager || !target_label) return false;
    
    return elf_add_pattern_match_relocation(manager->writer, jump_site, target_label);
}

bool elf_relocation_manager_add_string_op(ELFRelocationManager* manager,
                                          uint64_t call_site,
                                          const char* runtime_function) {
    if (!manager || !runtime_function) return false;
    
    return elf_add_string_op_relocation(manager->writer, call_site, runtime_function);
}

bool elf_relocation_manager_add_slice_bounds(ELFRelocationManager* manager,
                                             uint64_t check_site,
                                             const char* bounds_check_function) {
    if (!manager || !bounds_check_function) return false;
    
    return elf_add_slice_bounds_relocation(manager->writer, check_site, bounds_check_function);
}

bool elf_relocation_manager_add_spawn(ELFRelocationManager* manager,
                                      uint64_t spawn_site,
                                      const char* scheduler_function) {
    if (!manager || !scheduler_function) return false;
    
    return elf_add_spawn_relocation(manager->writer, spawn_site, scheduler_function);
}

bool elf_relocation_manager_generate_table(ELFRelocationManager* manager,
                                           ELFRelocation*** relocation_table,
                                           size_t* relocation_count) {
    if (!manager || !relocation_table || !relocation_count) return false;
    
    *relocation_table = manager->relocations;
    *relocation_count = manager->relocation_count;
    
    return true;
}

bool elf_relocation_manager_generate_section_table(ELFRelocationManager* manager,
                                                   uint16_t section_index,
                                                   ELFRelocation*** relocation_table,
                                                   size_t* relocation_count) {
    if (!manager || !relocation_table || !relocation_count) return false;
    
    // Count relocations for the specific section
    size_t section_count = 0;
    for (size_t i = 0; i < manager->relocation_count; i++) {
        // For simplicity, we'll return all relocations
        // In a full implementation, this would filter by section
        section_count++;
    }
    
    if (section_count == 0) {
        *relocation_table = NULL;
        *relocation_count = 0;
        return true;
    }
    
    // For now, just return all relocations
    *relocation_table = manager->relocations;
    *relocation_count = manager->relocation_count;
    
    return true;
}

bool elf_relocation_manager_validate_table(ELFRelocationManager* manager) {
    if (!manager) return false;
    
    // Basic validation
    if (manager->relocation_count > manager->relocation_capacity) return false;
    
    // Validate each relocation
    for (size_t i = 0; i < manager->relocation_count; i++) {
        if (!manager->relocations[i]) return false;
        
        // Basic relocation validation would go here
        // For now, we'll just check that the relocation exists
    }
    
    return true;
}

bool elf_relocation_manager_validate_types(ELFRelocationManager* manager) {
    if (!manager) return false;
    
    // Validate relocation types
    for (size_t i = 0; i < manager->relocation_count; i++) {
        if (!manager->relocations[i]) continue;
        
        uint32_t type = ELF64_R_TYPE(manager->relocations[i]->info);
        
        // Check for valid x86_64 relocation types
        switch (type) {
            case R_X86_64_NONE:
            case R_X86_64_64:
            case R_X86_64_PC32:
            case R_X86_64_PLT32:
            case R_X86_64_32:
            case R_X86_64_32S:
                // Valid types
                break;
            default:
                // Unknown or unsupported type
                return false;
        }
    }
    
    return true;
}

bool elf_relocation_manager_process_ast(ELFRelocationManager* manager, ASTNode* ast) {
    if (!manager || !ast) return false;
    // Stub implementation - just return success for now
    // TODO: Implement AST processing for relocations
    (void)manager;
    (void)ast;
    return true;
}

size_t elf_relocation_manager_get_count(ELFRelocationManager* manager) {
    if (!manager) return 0;
    return manager->relocation_count;
}
