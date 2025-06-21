/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Section Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "elf_writer.h"
#include "elf_writer_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// SECTION MANAGEMENT
// =============================================================================

ELFSection *elf_add_section(ELFWriter *writer, const char *name, 
                           uint32_t type, uint64_t flags) {
    if (!writer || !name) return NULL;
    
    // Expand array if needed
    if (writer->section_count >= writer->section_capacity) {
        size_t new_capacity = writer->section_capacity * 2;
        ELFSection **new_sections = realloc(writer->sections, 
                                           new_capacity * sizeof(ELFSection*));
        if (!new_sections) return NULL;
        writer->sections = new_sections;
        writer->section_capacity = new_capacity;
    }
    
    // Create new section
    ELFSection *section = calloc(1, sizeof(ELFSection));
    if (!section) return NULL;
    
    section->name = strdup(name);
    section->type = type;
    section->flags = flags;
    section->alignment = 1;
    
    // Add to array
    writer->sections[writer->section_count++] = section;
    
    return section;
}

// Note: elf_find_section_by_name and elf_expand_section_data are implemented in elf_utils.c

bool elf_create_standard_sections(ELFWriter *writer) {
    if (!writer) return false;
    
    // Create null section (index 0)
    ELFSection *null_section = elf_add_section(writer, "", SHT_NULL, 0);
    if (!null_section) return false;
    
    // Create .text section
    ELFSection *text_section = elf_add_section(writer, ".text", SHT_PROGBITS, 
                                              SHF_ALLOC | SHF_EXECINSTR);
    if (!text_section) return false;
    text_section->alignment = 16;
    
    // Create .data section
    ELFSection *data_section = elf_add_section(writer, ".data", SHT_PROGBITS,
                                              SHF_ALLOC | SHF_WRITE);
    if (!data_section) return false;
    data_section->alignment = 8;
    
    // Create .bss section
    ELFSection *bss_section = elf_add_section(writer, ".bss", SHT_NOBITS,
                                             SHF_ALLOC | SHF_WRITE);
    if (!bss_section) return false;
    bss_section->alignment = 8;
    
    // Create .rodata section
    ELFSection *rodata_section = elf_add_section(writer, ".rodata", SHT_PROGBITS,
                                                SHF_ALLOC);
    if (!rodata_section) return false;
    rodata_section->alignment = 8;
    
    // Create .symtab section
    ELFSection *symtab_section = elf_add_section(writer, ".symtab", SHT_SYMTAB, 0);
    if (!symtab_section) return false;
    symtab_section->alignment = 8;
    symtab_section->entry_size = sizeof(Elf64_Sym);
    
    // Create .strtab section
    ELFSection *strtab_section = elf_add_section(writer, ".strtab", SHT_STRTAB, 0);
    if (!strtab_section) return false;
    strtab_section->alignment = 1;
    
    // Create .shstrtab section
    ELFSection *shstrtab_section = elf_add_section(writer, ".shstrtab", SHT_STRTAB, 0);
    if (!shstrtab_section) return false;
    shstrtab_section->alignment = 1;
    
    // Create .rela.text section for relocations
    ELFSection *rela_text_section = elf_add_section(writer, ".rela.text", SHT_RELA, 0);
    if (!rela_text_section) return false;
    rela_text_section->alignment = 8;
    rela_text_section->entry_size = sizeof(Elf64_Rela);
    
    // Set up section linkages - this is critical for proper ELF structure
    // Find section indices
    size_t symtab_idx = 0, strtab_idx = 0, text_idx = 0;
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, ".symtab") == 0) {
            symtab_idx = i;
        } else if (strcmp(writer->sections[i]->name, ".strtab") == 0) {
            strtab_idx = i;
        } else if (strcmp(writer->sections[i]->name, ".text") == 0) {
            text_idx = i;
        }
    }
    
    // .symtab section should link to .strtab
    if (symtab_idx > 0 && strtab_idx > 0) {
        symtab_section->link = strtab_idx;
        symtab_section->info = 1; // Index of first non-local symbol (after null symbol)
    }
    
    // .rela.text section should link to .symtab and info should point to .text
    if (text_idx > 0 && symtab_idx > 0) {
        rela_text_section->link = symtab_idx;
        rela_text_section->info = text_idx;
    }
    
    return true;
}

bool elf_create_asthra_sections(ELFWriter *writer) {
    if (!writer) return false;
    
    // Create .Asthra.ffi section
    ELFSection *ffi_section = elf_add_section(writer, ".Asthra.ffi", SHT_PROGBITS, 0);
    if (!ffi_section) return false;
    ffi_section->asthra_type = ASTHRA_SECTION_FFI;
    ffi_section->is_asthra_section = true;
    ffi_section->alignment = 8;
    
    // Create .Asthra.gc section
    ELFSection *gc_section = elf_add_section(writer, ".Asthra.gc", SHT_PROGBITS, 0);
    if (!gc_section) return false;
    gc_section->asthra_type = ASTHRA_SECTION_GC;
    gc_section->is_asthra_section = true;
    gc_section->alignment = 8;
    
    // Create .Asthra.security_meta section
    ELFSection *security_section = elf_add_section(writer, ".Asthra.security_meta", 
                                                  SHT_PROGBITS, 0);
    if (!security_section) return false;
    security_section->asthra_type = ASTHRA_SECTION_SECURITY_META;
    security_section->is_asthra_section = true;
    security_section->alignment = 8;
    
    // Create .Asthra.pattern_matching section
    ELFSection *pattern_section = elf_add_section(writer, ".Asthra.pattern_matching",
                                                 SHT_PROGBITS, 0);
    if (!pattern_section) return false;
    pattern_section->asthra_type = ASTHRA_SECTION_PATTERN_MATCHING;
    pattern_section->is_asthra_section = true;
    pattern_section->alignment = 8;
    
    // Create .Asthra.string_ops section
    ELFSection *string_section = elf_add_section(writer, ".Asthra.string_ops",
                                                SHT_PROGBITS, 0);
    if (!string_section) return false;
    string_section->asthra_type = ASTHRA_SECTION_STRING_OPS;
    string_section->is_asthra_section = true;
    string_section->alignment = 8;
    
    // Create .Asthra.slice_meta section
    ELFSection *slice_section = elf_add_section(writer, ".Asthra.slice_meta",
                                               SHT_PROGBITS, 0);
    if (!slice_section) return false;
    slice_section->asthra_type = ASTHRA_SECTION_SLICE_META;
    slice_section->is_asthra_section = true;
    slice_section->alignment = 8;
    
    // Create .Asthra.concurrency section
    ELFSection *concurrency_section = elf_add_section(writer, ".Asthra.concurrency",
                                                     SHT_PROGBITS, 0);
    if (!concurrency_section) return false;
    concurrency_section->asthra_type = ASTHRA_SECTION_CONCURRENCY;
    concurrency_section->is_asthra_section = true;
    concurrency_section->alignment = 8;
    
    writer->sections_created_count++;
    return true;
}

// Duplicate definition removed - function already defined above

ELFSection *elf_get_section_by_name(ELFWriter *writer, const char *name) {
    if (!writer || !name) return NULL;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        if (writer->sections[i] && strcmp(writer->sections[i]->name, name) == 0) {
            return writer->sections[i];
        }
    }
    
    return NULL;
}

ELFSection *elf_get_section_by_index(ELFWriter *writer, size_t index) {
    if (!writer || index >= writer->section_count) return NULL;
    return writer->sections[index];
}

int elf_get_section_index_by_name(ELFWriter *writer, const char *name) {
    if (!writer || !name) return -1;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        if (writer->sections[i] && strcmp(writer->sections[i]->name, name) == 0) {
            return (int)i;
        }
    }
    
    return -1;
}

bool elf_append_section_data(ELFSection *section, const void *data, size_t size) {
    if (!section || !data || size == 0) return false;
    
    size_t required_size = section->data_size + size;
    if (!elf_expand_section_data(section, required_size)) {
        return false;
    }
    
    memcpy((char*)section->data + section->data_size, data, size);
    section->data_size += size;
    
    return true;
}

bool elf_set_section_data(ELFSection *section, const void *data, size_t size) {
    if (!section || !data) return false;
    
    if (!elf_expand_section_data(section, size)) {
        return false;
    }
    
    memcpy(section->data, data, size);
    section->data_size = size;
    
    return true;
} 
