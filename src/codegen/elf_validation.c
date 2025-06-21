/**
 * Asthra Programming Language Compiler
 * ELF Structure Validation Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for validating ELF structure and C toolchain compatibility.
 */

#include "elf_writer.h"
#include "elf_writer_core.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>

// External helper functions
extern ELFSection *find_section_by_name(ELFWriter *writer, const char *name);

// =============================================================================
// ELF STRUCTURE VALIDATION
// =============================================================================

bool elf_validate_structure(ELFWriter *writer) {
    if (!writer) return false;
    
    // Validate ELF header
    if (writer->header.e_ident[EI_MAG0] != ELFMAG0 ||
        writer->header.e_ident[EI_MAG1] != ELFMAG1 ||
        writer->header.e_ident[EI_MAG2] != ELFMAG2 ||
        writer->header.e_ident[EI_MAG3] != ELFMAG3) {
        fprintf(stderr, "ELF validation error: Invalid magic number\n");
        return false;
    }
    
    if (writer->header.e_ident[EI_CLASS] != ELFCLASS64) {
        fprintf(stderr, "ELF validation error: Not ELF-64\n");
        return false;
    }
    
    if (writer->header.e_machine != EM_X86_64) {
        fprintf(stderr, "ELF validation error: Not x86-64 architecture\n");
        return false;
    }
    
    if (writer->header.e_type != ET_REL) {
        fprintf(stderr, "ELF validation error: Not relocatable object\n");
        return false;
    }
    
    // Validate sections
    if (writer->section_count == 0) {
        fprintf(stderr, "ELF validation error: No sections\n");
        return false;
    }
    
    // Check for required sections
    bool has_text = false, has_symtab = false, has_strtab = false, has_shstrtab = false;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        
        if (strcmp(section->name, ".text") == 0) has_text = true;
        if (strcmp(section->name, ".symtab") == 0) has_symtab = true;
        if (strcmp(section->name, ".strtab") == 0) has_strtab = true;
        if (strcmp(section->name, ".shstrtab") == 0) has_shstrtab = true;
        
        // Validate section alignment
        if (section->alignment > 0 && (section->address % section->alignment) != 0) {
            fprintf(stderr, "ELF validation error: Section %s not properly aligned\n", 
                   section->name);
            return false;
        }
    }
    
    if (!has_text) {
        fprintf(stderr, "ELF validation error: Missing .text section\n");
        return false;
    }
    
    if (!has_symtab) {
        fprintf(stderr, "ELF validation error: Missing .symtab section\n");
        return false;
    }
    
    if (!has_strtab) {
        fprintf(stderr, "ELF validation error: Missing .strtab section\n");
        return false;
    }
    
    if (!has_shstrtab) {
        fprintf(stderr, "ELF validation error: Missing .shstrtab section\n");
        return false;
    }
    
    // Validate symbols
    if (writer->symbol_count == 0) {
        fprintf(stderr, "ELF validation warning: No symbols\n");
    }
    
    // Validate Asthra metadata sections
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        
        if (section->is_asthra_section) {
            // Validate magic numbers in Asthra sections
            if (section->data_size >= 4) {
                uint32_t *magic = (uint32_t*)section->data;
                
                switch (section->asthra_type) {
                    case ASTHRA_SECTION_FFI:
                        if (*magic != 0x41464649) { // "AFFI"
                            fprintf(stderr, "ELF validation error: Invalid FFI section magic\n");
                            return false;
                        }
                        break;
                    case ASTHRA_SECTION_GC:
                        if (*magic != 0x41474300) { // "AGC\0"
                            fprintf(stderr, "ELF validation error: Invalid GC section magic\n");
                            return false;
                        }
                        break;
                    case ASTHRA_SECTION_SECURITY_META:
                        if (*magic != 0x41534543) { // "ASEC"
                            fprintf(stderr, "ELF validation error: Invalid security section magic\n");
                            return false;
                        }
                        break;
                    default:
                        break;
                }
            }
        }
    }
    
    printf("ELF structure validation passed\n");
    return true;
}

bool elf_validate_c_compatibility(ELFWriter *writer) {
    if (!writer) return false;
    
    // Check for C-compatible symbol names
    for (size_t i = 0; i < writer->symbol_count; i++) {
        ELFSymbol *symbol = writer->symbols[i];
        
        // Check for valid C identifier characters
        const char *name = symbol->name;
        if (name && strlen(name) > 0) {
            if (!isalpha(name[0]) && name[0] != '_') {
                fprintf(stderr, "C compatibility warning: Symbol '%s' doesn't start with letter or underscore\n", 
                       name);
            }
            
            for (size_t j = 1; j < strlen(name); j++) {
                if (!isalnum(name[j]) && name[j] != '_') {
                    fprintf(stderr, "C compatibility warning: Symbol '%s' contains invalid character\n", 
                           name);
                }
            }
        }
    }
    
    // Check for standard calling convention
    bool has_sysv_functions = false;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        ELFSymbol *symbol = writer->symbols[i];
        if (symbol->is_ffi_function) {
            has_sysv_functions = true;
            break;
        }
    }
    
    if (!has_sysv_functions) {
        fprintf(stderr, "C compatibility warning: No System V ABI functions found\n");
    }
    
    // Check for required runtime symbols
    bool has_init_function = false;
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (strcmp(writer->symbols[i]->name, "_Asthra_init") == 0) {
            has_init_function = true;
            break;
        }
    }
    
    if (!has_init_function) {
        fprintf(stderr, "C compatibility error: Missing _Asthra_init function\n");
        return false;
    }
    
    printf("C compatibility validation passed\n");
    return true;
}

bool elf_validate_debug_symbols(ELFWriter *writer) {
    if (!writer) return false;
    
    // Check for debug sections
    bool has_debug_info = false;
    bool has_asthra_debug = false;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        
        if (strncmp(section->name, ".debug_", 7) == 0) {
            has_debug_info = true;
        }
        
        if (strcmp(section->name, ".Asthra.debug_info") == 0) {
            has_asthra_debug = true;
        }
    }
    
    if (!has_debug_info) {
        fprintf(stderr, "Debug validation warning: No standard debug sections\n");
    }
    
    if (!has_asthra_debug) {
        fprintf(stderr, "Debug validation warning: No Asthra debug section\n");
    }
    
    // Validate debug symbol consistency
    for (size_t i = 0; i < writer->symbol_count; i++) {
        ELFSymbol *symbol = writer->symbols[i];
        
        // Check that function symbols have reasonable addresses
        if (ELF64_ST_TYPE(symbol->info) == STT_FUNC) {
            if (symbol->value == 0 && symbol->section_index != SHN_UNDEF) {
                fprintf(stderr, "Debug validation warning: Function symbol '%s' has zero address\n",
                       symbol->name);
            }
        }
    }
    
    printf("Debug symbol validation passed\n");
    return true;
}