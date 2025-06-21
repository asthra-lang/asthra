/**
 * Asthra Programming Language Compiler
 * Mach-O Data Structure Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core data structure creation, destruction, and management
 */

#include "macho_data_structures.h"
#include "macho_platform.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>

// =============================================================================
// WRITER CREATION AND DESTRUCTION
// =============================================================================

MachoWriter *macho_writer_create(FFIAssemblyGenerator *ffi_generator) {
    MachoWriter *writer = calloc(1, sizeof(MachoWriter));
    if (!writer) return NULL;
    
    writer->ffi_generator = ffi_generator;
    
    // Initialize Mach-O header
    writer->header.magic = MH_MAGIC_64;
    writer->header.cputype = macho_get_cpu_type();
    writer->header.cpusubtype = macho_get_cpu_subtype();
    writer->header.filetype = MH_OBJECT;
    writer->header.ncmds = 0;
    writer->header.sizeofcmds = 0;
    writer->header.flags = 0;
    writer->header.reserved = 0;
    
    // Initialize string table
    writer->string_table_capacity = 1024;
    writer->string_table = calloc(1, writer->string_table_capacity);
    writer->string_table_size = 1; // Start with null byte
    
    // Initialize load commands
    writer->load_commands_capacity = 4096;
    writer->load_commands = calloc(1, writer->load_commands_capacity);
    
    // Initialize arrays
    writer->section_capacity = 16;
    writer->sections = calloc(writer->section_capacity, sizeof(MachoSection*));
    
    writer->symbol_capacity = 64;
    writer->symbols = calloc(writer->symbol_capacity, sizeof(MachoSymbol*));
    
    writer->relocation_capacity = 64;
    writer->relocations = calloc(writer->relocation_capacity, sizeof(MachoRelocation*));
    
    // Set default configuration
    writer->config.generate_debug_info = true;
    writer->config.validate_structure = true;
    writer->config.enable_optimizations = false;
    writer->config.cpu_type = writer->header.cputype;
    writer->config.cpu_subtype = writer->header.cpusubtype;
    
    printf("DEBUG: Created Mach-O writer for CPU type: 0x%x, subtype: 0x%x\n", 
           writer->header.cputype, writer->header.cpusubtype);
    fflush(stdout);
    
    return writer;
}

void macho_writer_destroy(MachoWriter *writer) {
    if (!writer) return;
    
    // Free sections
    for (size_t i = 0; i < writer->section_count; i++) {
        MachoSection *section = writer->sections[i];
        if (section) {
            free(section->name);
            free(section->segment_name);
            free(section->data);
            free(section);
        }
    }
    free(writer->sections);
    
    // Free symbols
    for (size_t i = 0; i < writer->symbol_count; i++) {
        MachoSymbol *symbol = writer->symbols[i];
        if (symbol) {
            free(symbol->name);
            free(symbol);
        }
    }
    free(writer->symbols);
    
    // Free relocations
    for (size_t i = 0; i < writer->relocation_count; i++) {
        free(writer->relocations[i]);
    }
    free(writer->relocations);
    
    // Free string table and load commands
    free(writer->string_table);
    free(writer->load_commands);
    
    free(writer);
}

// =============================================================================
// STRING TABLE MANAGEMENT
// =============================================================================

uint32_t macho_add_string_to_table(char **table, size_t *size, 
                                   size_t *capacity, const char *str) {
    printf("DEBUG: macho_add_string_to_table called with str='%s', size=%zu, capacity=%zu\n", 
           str ? str : "NULL", size ? *size : 0, capacity ? *capacity : 0);
    fflush(stdout);
    
    if (!table || !size || !capacity || !str) {
        return 0;
    }
    
    // Don't add empty strings - they should use offset 0
    if (strlen(str) == 0) {
        return 0;
    }
    
    // Check if string already exists
    for (size_t i = 1; i < *size; ) {
        if (i >= *size) break;
        
        if (strcmp(*table + i, str) == 0) {
            return (uint32_t)i;
        }
        
        size_t str_len = strlen(*table + i);
        if (str_len == 0) break;
        
        i += str_len + 1;
    }
    
    // Add new string
    size_t str_len = strlen(str) + 1; // Include null terminator
    
    // Expand table if needed
    while (*size + str_len > *capacity) {
        *capacity *= 2;
        char *new_table = realloc(*table, *capacity);
        if (!new_table) return 0;
        *table = new_table;
    }
    
    uint32_t offset = (uint32_t)*size;
    memcpy(*table + offset, str, str_len);
    *size += str_len;
    
    printf("DEBUG: Added string '%s' at offset %u, new size=%zu\n", 
           str, offset, *size);
    fflush(stdout);
    
    return offset;
}

// =============================================================================
// SYMBOL MANAGEMENT
// =============================================================================

MachoSymbol *macho_add_symbol(MachoWriter *writer, const char *name,
                             uint8_t type, uint8_t sect, uint64_t value) {
    if (!writer || !name) return NULL;
    
    // Expand array if needed
    if (writer->symbol_count >= writer->symbol_capacity) {
        size_t new_capacity = writer->symbol_capacity * 2;
        MachoSymbol **new_symbols = realloc(writer->symbols,
                                           new_capacity * sizeof(MachoSymbol*));
        if (!new_symbols) return NULL;
        writer->symbols = new_symbols;
        writer->symbol_capacity = new_capacity;
    }
    
    // Create new symbol
    MachoSymbol *symbol = calloc(1, sizeof(MachoSymbol));
    if (!symbol) return NULL;
    
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->sect = sect;
    symbol->value = value;
    symbol->is_external = (type & N_EXT) != 0;
    
    // Add to string table
    symbol->name_offset = macho_add_string_to_table(
        &writer->string_table,
        &writer->string_table_size,
        &writer->string_table_capacity,
        name
    );
    
    // Add to array
    writer->symbols[writer->symbol_count++] = symbol;
    writer->symbols_added_count++;
    
    printf("DEBUG: Added Mach-O symbol '%s' type=0x%x sect=%u value=0x%llx\n",
           name, type, sect, (unsigned long long)value);
    fflush(stdout);
    
    return symbol;
}

MachoSymbol *macho_find_symbol_by_name(MachoWriter *writer, const char *name) {
    if (!writer || !name) return NULL;
    
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, name) == 0) {
            return writer->symbols[i];
        }
    }
    return NULL;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

void macho_get_statistics(MachoWriter *writer, size_t *sections, size_t *symbols,
                         size_t *relocations) {
    if (!writer) return;
    
    if (sections) *sections = writer->section_count;
    if (symbols) *symbols = writer->symbol_count;
    if (relocations) *relocations = writer->relocation_count;
}

void macho_print_structure(MachoWriter *writer) {
    if (!writer) return;
    
    printf("Mach-O Writer Structure:\n");
    printf("  CPU Type: 0x%x\n", writer->header.cputype);
    printf("  CPU Subtype: 0x%x\n", writer->header.cpusubtype);
    printf("  File Type: 0x%x\n", writer->header.filetype);
    printf("  Sections: %zu\n", writer->section_count);
    printf("  Symbols: %zu\n", writer->symbol_count);
    printf("  Relocations: %zu\n", writer->relocation_count);
    
    printf("\nSections:\n");
    for (size_t i = 0; i < writer->section_count; i++) {
        MachoSection *section = writer->sections[i];
        printf("  [%zu] %s.%s (flags=0x%x, size=%zu)\n",
               i, section->segment_name, section->name, 
               section->flags, section->data_size);
    }
}