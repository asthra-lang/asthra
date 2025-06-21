/**
 * Asthra Programming Language Compiler
 * ELF Debug Helper Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Helper functions for ELF debug operations.
 */

#include "elf_writer.h"
#include "elf_writer_core.h"
#include <string.h>
#include <stdlib.h>

// Note: Helper functions elf_find_section_by_name and elf_expand_section_data
// are implemented in elf_utils.c

// =============================================================================
// DEBUG UTILITY FUNCTIONS
// =============================================================================

size_t elf_writer_get_cu_size(ELFWriter *writer) {
    if (!writer) return 0;
    
    // Look for the debug_info section
    ELFSection *debug_info = elf_find_section_by_name(writer, ".debug_info");
    if (!debug_info) {
        // Also check for Asthra debug info
        debug_info = elf_find_section_by_name(writer, ".Asthra.debug_info");
    }
    
    if (debug_info && debug_info->data) {
        // Simple implementation: return the section size
        // In a real implementation, this would parse DWARF data
        return debug_info->size;
    }
    
    return 0;
}

size_t elf_writer_get_function_debug_size(ELFWriter *writer) {
    if (!writer) return 0;
    
    // Look for the debug_info section
    ELFSection *debug_info = elf_find_section_by_name(writer, ".debug_info");
    if (!debug_info) {
        // Also check for Asthra debug info
        debug_info = elf_find_section_by_name(writer, ".Asthra.debug_info");
    }
    
    if (debug_info && debug_info->data) {
        // Simple implementation: estimate based on section size
        // In a real implementation, this would parse DWARF data to count function entries
        // For now, assume functions take up about 60% of debug info
        return (debug_info->size * 6) / 10;
    }
    
    return 0;
}

size_t elf_writer_get_abbreviation_table_size(ELFWriter *writer) {
    if (!writer) return 0;
    
    // Look for the debug_abbrev section
    ELFSection *debug_abbrev = elf_find_section_by_name(writer, ".debug_abbrev");
    if (!debug_abbrev) {
        // Also check for Asthra debug abbrev
        debug_abbrev = elf_find_section_by_name(writer, ".Asthra.debug_abbrev");
    }
    
    if (debug_abbrev && debug_abbrev->data) {
        // Simple implementation: return the section size
        // In a real implementation, this would parse DWARF abbreviation data
        return debug_abbrev->size;
    }
    
    return 0;
}

size_t elf_writer_get_type_debug_size(ELFWriter *writer) {
    if (!writer) return 0;
    
    // Look for the debug_info section
    ELFSection *debug_info = elf_find_section_by_name(writer, ".debug_info");
    if (!debug_info) {
        // Also check for Asthra debug info
        debug_info = elf_find_section_by_name(writer, ".Asthra.debug_info");
    }
    
    if (debug_info && debug_info->data) {
        // Simple implementation: estimate based on section size
        // In a real implementation, this would parse DWARF data to count type entries
        // For now, assume types take up about 40% of debug info
        return (debug_info->size * 4) / 10;
    }
    
    return 0;
}