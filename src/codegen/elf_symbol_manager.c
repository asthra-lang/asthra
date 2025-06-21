/**
 * Asthra Programming Language Compiler
 * ELF Symbol Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for managing ELF symbols with comprehensive Asthra metadata.
 */

#include "elf_symbol_manager.h"
#include "elf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// SYMBOL MANAGEMENT
// =============================================================================

ELFSymbol *elf_add_symbol(ELFWriter *writer, const char *name, 
                         uint64_t value, uint64_t size, 
                         uint8_t type, uint8_t binding, uint16_t section) {
    if (!writer || !name) return NULL;
    
    printf("DEBUG: elf_add_symbol called with name='%s', value=0x%llx\n", name, (unsigned long long)value);
    
    // Expand symbols array if needed
    if (writer->symbol_count >= writer->symbol_capacity) {
        printf("DEBUG: Expanding symbols array from %zu to %zu\n", writer->symbol_capacity, writer->symbol_capacity * 2);
        size_t new_capacity = writer->symbol_capacity * 2;
        ELFSymbol **new_symbols = realloc(writer->symbols, 
                                         new_capacity * sizeof(ELFSymbol*));
        if (!new_symbols) return NULL;
        writer->symbols = new_symbols;
        writer->symbol_capacity = new_capacity;
    }
    
    printf("DEBUG: Creating symbol structure\n");
    // Create new symbol
    ELFSymbol *symbol = calloc(1, sizeof(ELFSymbol));
    if (!symbol) return NULL;
    
    printf("DEBUG: Setting symbol fields\n");
    symbol->name = strdup(name);
    symbol->value = value;
    symbol->size = size;
    symbol->info = (uint8_t)ELF64_ST_INFO(binding, type);
    symbol->other = STV_DEFAULT;
    symbol->section_index = section;
    
    if (!symbol->name) {
        free(symbol);
        return NULL;
    }
    
    printf("DEBUG: Adding symbol to array at index %zu\n", writer->symbol_count);
    writer->symbols[writer->symbol_count] = symbol;
    writer->symbol_count++;
    
    writer->symbols_added_count++;
    printf("DEBUG: elf_add_symbol returning symbol at %p\n", (void*)symbol);
    return symbol;
}

ELFSymbol *elf_add_ffi_symbol(ELFWriter *writer, const char *name,
                             uint64_t address, FFICallContext *ffi_context) {
    if (!writer || !name) return NULL;
    
    ELFSymbol *symbol = elf_add_symbol(writer, name, address, 0, 
                                      STT_FUNC, STB_GLOBAL, 1);
    if (!symbol) return NULL;
    
    // Mark as FFI function
    symbol->is_ffi_function = true;
    
    // Add FFI-specific metadata if context is provided
    if (ffi_context) {
        // Store additional FFI metadata in symbol (simplified)
        // In a full implementation, this would link to the FFI metadata section
    }
    
    return symbol;
}

ELFSymbol *elf_add_pattern_match_symbol(ELFWriter *writer, const char *name,
                                       uint64_t address, PatternMatchContext *context) {
    if (!writer || !name) return NULL;
    
    ELFSymbol *symbol = elf_add_symbol(writer, name, address, 0,
                                      STT_NOTYPE, STB_LOCAL, 1);
    if (!symbol) return NULL;
    
    symbol->is_pattern_match = true;
    
    // Add pattern matching metadata if context is provided
    if (context) {
        // Store pattern matching metadata (simplified)
    }
    
    return symbol;
}

ELFSymbol *elf_add_string_op_symbol(ELFWriter *writer, const char *name,
                                   uint64_t address, StringOperationContext *context) {
    if (!writer || !name) return NULL;
    
    ELFSymbol *symbol = elf_add_symbol(writer, name, address, 0,
                                      STT_NOTYPE, STB_LOCAL, 1);
    if (!symbol) return NULL;
    
    symbol->is_string_operation = true;
    
    // Add string operation metadata if context is provided
    if (context) {
        // Store string operation metadata (simplified)
    }
    
    return symbol;
}

ELFSymbol *elf_add_slice_op_symbol(ELFWriter *writer, const char *name,
                                  uint64_t address, SliceOperationContext *context) {
    if (!writer || !name) return NULL;
    
    ELFSymbol *symbol = elf_add_symbol(writer, name, address, 0,
                                      STT_NOTYPE, STB_LOCAL, 1);
    if (!symbol) return NULL;
    
    symbol->is_slice_operation = true;
    
    // Add slice operation metadata if context is provided
    if (context) {
        // Store slice operation metadata (simplified)
    }
    
    return symbol;
}

ELFSymbol *elf_add_spawn_symbol(ELFWriter *writer, const char *name,
                               uint64_t address, ConcurrencyContext *context) {
    if (!writer || !name) return NULL;
    
    ELFSymbol *symbol = elf_add_symbol(writer, name, address, 0,
                                      STT_NOTYPE, STB_LOCAL, 1);
    if (!symbol) return NULL;
    
    symbol->is_spawn_point = true;
    
    // Add concurrency metadata if context is provided
    if (context) {
        // Store concurrency metadata (simplified)
    }
    
    return symbol;
}

ELFSymbol *elf_add_gc_root_symbol(ELFWriter *writer, const char *name,
                                 uint64_t address, size_t size, uint32_t ownership_type) {
    if (!writer || !name) return NULL;
    
    ELFSymbol *symbol = elf_add_symbol(writer, name, address, size,
                                      STT_OBJECT, STB_LOCAL, 2); // .data section
    if (!symbol) return NULL;
    
    symbol->is_gc_root = true;
    
    // Store ownership type information (simplified)
    // In a full implementation, this would link to the GC metadata section
    
    return symbol;
}

// =============================================================================
// SYMBOL MANAGER IMPLEMENTATION
// =============================================================================

ELFSymbolManager* elf_symbol_manager_create(void) {
    ELFSymbolManager* manager = calloc(1, sizeof(ELFSymbolManager));
    if (!manager) return NULL;
    
    // Create an ELF writer for internal use
    manager->writer = elf_writer_create(NULL);
    if (!manager->writer) {
        free(manager);
        return NULL;
    }
    
    manager->symbol_capacity = 64;
    manager->symbols = calloc(manager->symbol_capacity, sizeof(ELFSymbol*));
    if (!manager->symbols) {
        elf_writer_destroy(manager->writer);
        free(manager);
        return NULL;
    }
    
    manager->symbol_count = 0;
    manager->is_finalized = false;
    
    return manager;
}

void elf_symbol_manager_destroy(ELFSymbolManager* manager) {
    if (!manager) return;
    
    if (manager->symbols) {
        for (size_t i = 0; i < manager->symbol_count; i++) {
            if (manager->symbols[i] && manager->symbols[i]->name) {
                free((void*)manager->symbols[i]->name);
            }
            free(manager->symbols[i]);
        }
        free(manager->symbols);
    }
    
    if (manager->writer) {
        elf_writer_destroy(manager->writer);
    }
    
    free(manager);
}

bool elf_symbol_manager_add_symbol(ELFSymbolManager* manager, const ELFSymbol* symbol) {
    if (!manager || !symbol || manager->is_finalized) return false;
    
    // Expand array if needed
    if (manager->symbol_count >= manager->symbol_capacity) {
        size_t new_capacity = manager->symbol_capacity * 2;
        ELFSymbol** new_symbols = realloc(manager->symbols, 
                                         new_capacity * sizeof(ELFSymbol*));
        if (!new_symbols) return false;
        manager->symbols = new_symbols;
        manager->symbol_capacity = new_capacity;
    }
    
    // Create a copy of the symbol
    ELFSymbol* symbol_copy = calloc(1, sizeof(ELFSymbol));
    if (!symbol_copy) return false;
    
    *symbol_copy = *symbol;
    if (symbol->name) {
        symbol_copy->name = strdup(symbol->name);
        if (!symbol_copy->name) {
            free(symbol_copy);
            return false;
        }
    }
    
    manager->symbols[manager->symbol_count] = symbol_copy;
    manager->symbol_count++;
    
    return true;
}

ELFSymbol* elf_symbol_manager_lookup_symbol(ELFSymbolManager* manager, const char* name) {
    if (!manager || !name) return NULL;
    
    for (size_t i = 0; i < manager->symbol_count; i++) {
        if (manager->symbols[i] && manager->symbols[i]->name &&
            strcmp(manager->symbols[i]->name, name) == 0) {
            return manager->symbols[i];
        }
    }
    
    return NULL;
}

size_t elf_symbol_manager_get_symbol_index(ELFSymbolManager* manager, const char* name) {
    if (!manager || !name) return SIZE_MAX;
    
    for (size_t i = 0; i < manager->symbol_count; i++) {
        if (manager->symbols[i] && manager->symbols[i]->name &&
            strcmp(manager->symbols[i]->name, name) == 0) {
            return i;
        }
    }
    
    return SIZE_MAX;
}

size_t elf_symbol_manager_count_global_symbols(ELFSymbolManager* manager) {
    if (!manager) return 0;
    
    size_t count = 0;
    for (size_t i = 0; i < manager->symbol_count; i++) {
        if (manager->symbols[i] && 
            ELF64_ST_BIND(manager->symbols[i]->info) == STB_GLOBAL) {
            count++;
        }
    }
    
    return count;
}

bool elf_symbol_manager_generate_table(ELFSymbolManager* manager, 
                                       ELFSymbol*** symbol_table, 
                                       size_t* symbol_count) {
    if (!manager || !symbol_table || !symbol_count) return false;
    
    *symbol_table = manager->symbols;
    *symbol_count = manager->symbol_count;
    
    return true;
}

bool elf_symbol_manager_generate_dynamic_table(ELFSymbolManager* manager,
                                               ELFSymbol*** dynsym_table,
                                               size_t* dynsym_count) {
    if (!manager || !dynsym_table || !dynsym_count) return false;
    
    // Count dynamic symbols (global symbols)
    size_t dyn_count = elf_symbol_manager_count_global_symbols(manager);
    if (dyn_count == 0) {
        *dynsym_table = NULL;
        *dynsym_count = 0;
        return true;
    }
    
    // Allocate array for dynamic symbols
    ELFSymbol** dyn_symbols = calloc(dyn_count, sizeof(ELFSymbol*));
    if (!dyn_symbols) return false;
    
    // Copy global symbols
    size_t dyn_index = 0;
    for (size_t i = 0; i < manager->symbol_count; i++) {
        if (manager->symbols[i] && 
            ELF64_ST_BIND(manager->symbols[i]->info) == STB_GLOBAL) {
            dyn_symbols[dyn_index++] = manager->symbols[i];
        }
    }
    
    *dynsym_table = dyn_symbols;
    *dynsym_count = dyn_count;
    
    return true;
}

bool elf_symbol_manager_generate_sorted_table(ELFSymbolManager* manager,
                                              ELFSymbol*** symbol_table,
                                              size_t* symbol_count) {
    if (!manager || !symbol_table || !symbol_count) return false;
    
    // For now, just return the regular table
    // In a full implementation, this would sort by address or name
    return elf_symbol_manager_generate_table(manager, symbol_table, symbol_count);
}

bool elf_symbol_manager_validate_bindings(ELFSymbolManager* manager) {
    if (!manager) return false;
    
    // Check for valid symbol bindings
    for (size_t i = 0; i < manager->symbol_count; i++) {
        if (!manager->symbols[i]) continue;
        
        uint8_t binding = ELF64_ST_BIND(manager->symbols[i]->info);
        if (binding != STB_LOCAL && binding != STB_GLOBAL && binding != STB_WEAK) {
            return false;
        }
    }
    
    return true;
}

bool elf_symbol_manager_validate_table(ELFSymbolManager* manager) {
    if (!manager) return false;
    
    // Basic validation
    if (manager->symbol_count > manager->symbol_capacity) return false;
    
    // Validate bindings
    if (!elf_symbol_manager_validate_bindings(manager)) return false;
    
    // Check for duplicate names (simplified check)
    for (size_t i = 0; i < manager->symbol_count; i++) {
        if (!manager->symbols[i] || !manager->symbols[i]->name) continue;
        
        for (size_t j = i + 1; j < manager->symbol_count; j++) {
            if (!manager->symbols[j] || !manager->symbols[j]->name) continue;
            
            if (strcmp(manager->symbols[i]->name, manager->symbols[j]->name) == 0) {
                // Duplicate name found - this might be valid in some cases
                // For now, we'll allow it
            }
        }
    }
    
    return true;
}
