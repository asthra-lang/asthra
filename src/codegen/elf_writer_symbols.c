/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Symbol Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

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
    
    // Expand array if needed
    if (writer->symbol_count >= writer->symbol_capacity) {
        size_t new_capacity = writer->symbol_capacity * 2;
        ELFSymbol **new_symbols = realloc(writer->symbols, 
                                         new_capacity * sizeof(ELFSymbol*));
        if (!new_symbols) {
            printf("DEBUG: Failed to expand symbol array\n");
            return NULL;
        }
        writer->symbols = new_symbols;
        writer->symbol_capacity = new_capacity;
    }
    
    // Create new symbol
    printf("DEBUG: Creating symbol structure\n");
    ELFSymbol *symbol = calloc(1, sizeof(ELFSymbol));
    if (!symbol) {
        printf("DEBUG: Failed to allocate symbol\n");
        return NULL;
    }
    
    printf("DEBUG: Setting symbol fields\n");
    symbol->name = strdup(name);
    symbol->value = value;
    symbol->size = size;
    symbol->info = ELF64_ST_INFO(binding, type);
    symbol->other = STV_DEFAULT;
    symbol->section_index = section;
    
    // Add to array
    printf("DEBUG: Adding symbol to array at index %zu\n", writer->symbol_count);
    writer->symbols[writer->symbol_count++] = symbol;
    
    printf("DEBUG: elf_add_symbol returning symbol at %p\n", (void*)symbol);
    return symbol;
}