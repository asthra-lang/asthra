/**
 * Asthra Programming Language Compiler
 * ELF Utility Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Utility functions that support the ELF writer system.
 */

#include "elf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Helper function to find section by name
ELFSection *elf_find_section_by_name(ELFWriter *writer, const char *name) {
    if (!writer || !name) return NULL;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, name) == 0) {
            return writer->sections[i];
        }
    }
    return NULL;
}

// Helper function to expand section data
bool elf_expand_section_data(ELFSection *section, size_t required_size) {
    if (!section) return false;
    
    if (required_size <= section->data_capacity) {
        return true;
    }
    
    size_t new_capacity = section->data_capacity;
    while (new_capacity < required_size) {
        new_capacity *= 2;
    }
    
    void *new_data = realloc(section->data, new_capacity);
    if (!new_data) return false;
    
    section->data = new_data;
    section->data_capacity = new_capacity;
    return true;
} 
