/**
 * Asthra Programming Language Compiler
 * Mach-O Section Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Section creation, data management, and standard section handling
 */

#include "macho_sections.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mach-o/loader.h>

// =============================================================================
// SECTION MANAGEMENT
// =============================================================================

MachoSection *macho_add_section(MachoWriter *writer, const char *name,
                               const char *segment_name, uint32_t flags) {
    if (!writer || !name || !segment_name) return NULL;
    
    // Expand array if needed
    if (writer->section_count >= writer->section_capacity) {
        size_t new_capacity = writer->section_capacity * 2;
        MachoSection **new_sections = realloc(writer->sections, 
                                             new_capacity * sizeof(MachoSection*));
        if (!new_sections) return NULL;
        writer->sections = new_sections;
        writer->section_capacity = new_capacity;
    }
    
    // Create new section
    MachoSection *section = calloc(1, sizeof(MachoSection));
    if (!section) return NULL;
    
    section->name = strdup(name);
    section->segment_name = strdup(segment_name);
    section->flags = flags;
    section->alignment = 1;
    
    // Add to array
    writer->sections[writer->section_count++] = section;
    writer->sections_created_count++;
    
    printf("DEBUG: Created Mach-O section '%s' in segment '%s'\n", name, segment_name);
    fflush(stdout);
    
    return section;
}

MachoSection *macho_find_section_by_name(MachoWriter *writer, const char *name) {
    if (!writer || !name) return NULL;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, name) == 0) {
            return writer->sections[i];
        }
    }
    return NULL;
}

bool macho_set_section_data(MachoSection *section, const void *data, size_t size) {
    if (!section || !data) return false;
    
    if (size > section->data_capacity) {
        void *new_data = realloc(section->data, size);
        if (!new_data) return false;
        section->data = new_data;
        section->data_capacity = size;
    }
    
    memcpy(section->data, data, size);
    section->data_size = size;
    
    return true;
}

bool macho_append_section_data(MachoSection *section, const void *data, size_t size) {
    if (!section || !data || size == 0) return false;
    
    size_t required_size = section->data_size + size;
    if (required_size > section->data_capacity) {
        size_t new_capacity = section->data_capacity;
        if (new_capacity == 0) new_capacity = 1024;
        
        while (new_capacity < required_size) {
            new_capacity *= 2;
        }
        
        void *new_data = realloc(section->data, new_capacity);
        if (!new_data) return false;
        
        section->data = new_data;
        section->data_capacity = new_capacity;
    }
    
    memcpy((char*)section->data + section->data_size, data, size);
    section->data_size += size;
    
    return true;
}

// =============================================================================
// STANDARD SECTIONS
// =============================================================================

bool macho_create_standard_sections(MachoWriter *writer) {
    if (!writer) return false;
    
    printf("DEBUG: Creating standard Mach-O sections\n");
    fflush(stdout);
    
    // Create __TEXT.__text section (executable code)
    MachoSection *text_section = macho_add_section(writer, "__text", "__TEXT", 
                                                  S_ATTR_PURE_INSTRUCTIONS | S_ATTR_SOME_INSTRUCTIONS);
    if (!text_section) return false;
    text_section->alignment = 4; // 16-byte alignment
    
    // Create __DATA.__data section (initialized data)
    MachoSection *data_section = macho_add_section(writer, "__data", "__DATA", 0);
    if (!data_section) return false;
    data_section->alignment = 3; // 8-byte alignment
    
    // Create __DATA.__bss section (uninitialized data)
    MachoSection *bss_section = macho_add_section(writer, "__bss", "__DATA", S_ZEROFILL);
    if (!bss_section) return false;
    bss_section->alignment = 3; // 8-byte alignment
    
    // Create __TEXT.__const section (read-only data)
    MachoSection *const_section = macho_add_section(writer, "__const", "__TEXT", 0);
    if (!const_section) return false;
    const_section->alignment = 3; // 8-byte alignment
    
    printf("DEBUG: Created %zu standard Mach-O sections\n", writer->section_count);
    fflush(stdout);
    
    return true;
}

bool macho_populate_text_section(MachoWriter *writer) {
    if (!writer || !writer->ffi_generator) return false;
    
    printf("DEBUG: Populating Mach-O __text section\n");
    fflush(stdout);
    
    // Find __text section
    MachoSection *text_section = macho_find_section_by_name(writer, "__text");
    if (!text_section) {
        printf("DEBUG: __text section not found\n");
        fflush(stdout);
        return false;
    }
    
    // Generate assembly code
    char assembly_buffer[65536];
    memset(assembly_buffer, 0, sizeof(assembly_buffer));
    
    if (!ffi_print_nasm_assembly(writer->ffi_generator, assembly_buffer, sizeof(assembly_buffer))) {
        printf("DEBUG: Failed to generate assembly code\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: Generated assembly code (%zu bytes)\n", strlen(assembly_buffer));
    fflush(stdout);
    
    // For now, we'll create a simple machine code stub
    // TODO: Implement proper assembly to machine code conversion
    uint8_t machine_code[] = {
        0xb8, 0x00, 0x00, 0x00, 0x00,  // mov eax, 0
        0xc3                            // ret
    };
    
    if (!macho_set_section_data(text_section, machine_code, sizeof(machine_code))) {
        printf("DEBUG: Failed to set __text section data\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: Set __text section data (%zu bytes)\n", text_section->data_size);
    fflush(stdout);
    
    return true;
}