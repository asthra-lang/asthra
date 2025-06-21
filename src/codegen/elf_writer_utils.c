/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Utilities Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "elf_writer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

// =============================================================================
// STRING TABLE MANAGEMENT
// =============================================================================

uint32_t elf_add_string_to_table(char **table, size_t *size, 
                                 size_t *capacity, const char *str) {
    printf("DEBUG: elf_add_string_to_table called with str='%s', size=%zu, capacity=%zu\n", 
           str ? str : "NULL", size ? *size : 0, capacity ? *capacity : 0);
    fflush(stdout);
    
    if (!table || !size || !capacity || !str) {
        printf("DEBUG: elf_add_string_to_table - Invalid parameters\n");
        fflush(stdout);
        return 0;
    }
    
    // Don't add empty strings - they should use offset 0 (the initial null byte)
    if (strlen(str) == 0) {
        printf("DEBUG: elf_add_string_to_table - Skipping empty string, returning offset 0\n");
        fflush(stdout);
        return 0;
    }
    
    // Check if string already exists
    for (size_t i = 1; i < *size; ) {
        if (i >= *size) break; // Safety check
        
        if (strcmp(*table + i, str) == 0) {
            return (uint32_t)i;
        }
        
        size_t str_len = strlen(*table + i);
        if (str_len == 0) break; // Avoid infinite loop on empty strings
        
        i += str_len + 1; // Skip to next string (including null terminator)
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
    
    printf("DEBUG: elf_add_string_to_table - Added string '%s' at offset %u, new size=%zu\n", 
           str, offset, *size);
    fflush(stdout);
    
    return offset;
}

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

void elf_get_statistics(ELFWriter *writer, size_t *sections, size_t *symbols,
                       size_t *relocations, size_t *metadata_entries) {
    if (!writer) return;
    
    if (sections) *sections = writer->section_count;
    if (symbols) *symbols = writer->symbol_count;
    if (relocations) *relocations = writer->relocation_count;
    if (metadata_entries) *metadata_entries = writer->asthra_metadata_entries_count;
}

void elf_print_structure(ELFWriter *writer) {
    if (!writer) return;
    
    printf("ELF Writer Structure:\n");
    printf("  Sections: %zu\n", writer->section_count);
    printf("  Symbols: %zu\n", writer->symbol_count);
    printf("  Relocations: %zu\n", writer->relocation_count);
    printf("  Metadata entries: %u\n", writer->asthra_metadata_entries_count);
    
    printf("\nSections:\n");
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        printf("  [%zu] %s (type=0x%x, flags=0x%llx, size=%zu)\n",
               i, section->name, section->type, (unsigned long long)section->flags, section->data_size);
    }
}

size_t elf_calculate_file_size(ELFWriter *writer) {
    if (!writer) return 0;
    
    size_t size = sizeof(Elf64_Ehdr); // ELF header
    size += writer->section_count * sizeof(Elf64_Shdr); // Section headers
    
    // Add section data sizes with alignment padding
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        if (section->type != SHT_NULL && section->type != SHT_NOBITS) {
            // Align to section alignment
            if (section->alignment > 1) {
                while (size % section->alignment != 0) {
                    size++;
                }
            }
            size += section->data_size;
        }
    }
    
    return size;
}

bool elf_calculate_layout(ELFWriter *writer) {
    if (!writer) return false;
    
    uint64_t current_offset = sizeof(Elf64_Ehdr) + 
                             writer->section_count * sizeof(Elf64_Shdr);
    uint64_t current_address = 0x1000; // Start at 4KB boundary
    
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        
        if (section->type == SHT_NULL) {
            section->offset = 0;
            section->address = 0;
            continue;
        }
        
        // Align offset and address
        if (section->alignment > 1) {
            while (current_offset % section->alignment != 0) {
                current_offset++;
            }
            while (current_address % section->alignment != 0) {
                current_address++;
            }
        }
        
        section->offset = current_offset;
        section->address = current_address;
        
        printf("DEBUG: Layout - Section %zu '%s': offset=0x%llx, size=%zu\n", 
               i, section->name ? section->name : "NULL", 
               (unsigned long long)section->offset, section->data_size);
        fflush(stdout);
        
        if (section->type != SHT_NOBITS) {
            current_offset += section->data_size;
        }
        current_address += section->data_size;
    }
    
    return true;
}

uint64_t elf_align_value(uint64_t value, uint64_t alignment) {
    if (alignment <= 1) return value;
    
    uint64_t remainder = value % alignment;
    if (remainder == 0) return value;
    
    return value + (alignment - remainder);
}

bool elf_is_aligned(uint64_t value, uint64_t alignment) {
    if (alignment <= 1) return true;
    return (value % alignment) == 0;
}

const char *elf_symbol_type_to_string(uint8_t type) {
    switch (type) {
        case STT_NOTYPE: return "NOTYPE";
        case STT_OBJECT: return "OBJECT";
        case STT_FUNC: return "FUNC";
        case STT_SECTION: return "SECTION";
        case STT_FILE: return "FILE";
        case STT_COMMON: return "COMMON";
        case STT_TLS: return "TLS";
        default: return "UNKNOWN";
    }
}

const char *elf_symbol_binding_to_string(uint8_t binding) {
    switch (binding) {
        case STB_LOCAL: return "LOCAL";
        case STB_GLOBAL: return "GLOBAL";
        case STB_WEAK: return "WEAK";
        default: return "UNKNOWN";
    }
}

const char *elf_section_type_to_string(uint32_t type) {
    switch (type) {
        case SHT_NULL: return "NULL";
        case SHT_PROGBITS: return "PROGBITS";
        case SHT_SYMTAB: return "SYMTAB";
        case SHT_STRTAB: return "STRTAB";
        case SHT_RELA: return "RELA";
        case SHT_HASH: return "HASH";
        case SHT_DYNAMIC: return "DYNAMIC";
        case SHT_NOTE: return "NOTE";
        case SHT_NOBITS: return "NOBITS";
        case SHT_REL: return "REL";
        case SHT_SHLIB: return "SHLIB";
        case SHT_DYNSYM: return "DYNSYM";
        default: return "UNKNOWN";
    }
}

const char *elf_relocation_type_to_string(uint32_t type) {
    switch (type) {
        case R_X86_64_NONE: return "R_X86_64_NONE";
        case R_X86_64_64: return "R_X86_64_64";
        case R_X86_64_PC32: return "R_X86_64_PC32";
        case R_X86_64_GOT32: return "R_X86_64_GOT32";
        case R_X86_64_PLT32: return "R_X86_64_PLT32";
        case R_X86_64_COPY: return "R_X86_64_COPY";
        case R_X86_64_GLOB_DAT: return "R_X86_64_GLOB_DAT";
        case R_X86_64_JUMP_SLOT: return "R_X86_64_JUMP_SLOT";
        case R_X86_64_RELATIVE: return "R_X86_64_RELATIVE";
        case R_X86_64_GOTPCREL: return "R_X86_64_GOTPCREL";
        case R_X86_64_32: return "R_X86_64_32";
        case R_X86_64_32S: return "R_X86_64_32S";
        case R_X86_64_16: return "R_X86_64_16";
        case R_X86_64_PC16: return "R_X86_64_PC16";
        case R_X86_64_8: return "R_X86_64_8";
        case R_X86_64_PC8: return "R_X86_64_PC8";
        default: return "UNKNOWN";
    }
}

void elf_format_header_info(ELFWriter *writer, char *buffer, size_t buffer_size) {
    if (!writer || !buffer || buffer_size == 0) return;
    
    snprintf(buffer, buffer_size,
        "ELF Header:\n"
        "  Class: %s\n"
        "  Data: %s\n"
        "  Type: %s\n"
        "  Machine: %s\n"
        "  Entry: 0x%llx\n"
        "  Sections: %d\n",
        (writer->header.e_ident[EI_CLASS] == ELFCLASS64) ? "ELF64" : "ELF32",
        (writer->header.e_ident[EI_DATA] == ELFDATA2LSB) ? "2's complement, little endian" : "big endian",
        (writer->header.e_type == ET_REL) ? "Relocatable" : "Other",
        (writer->header.e_machine == EM_X86_64) ? "x86-64" : "Other",
        (unsigned long long)writer->header.e_entry,
        writer->header.e_shnum
    );
}

void elf_format_section_info(ELFSection *section, char *buffer, size_t buffer_size) {
    if (!section || !buffer || buffer_size == 0) return;
    
    snprintf(buffer, buffer_size,
        "Section: %s\n"
        "  Type: %s\n"
        "  Flags: 0x%llx\n"
        "  Address: 0x%llx\n"
        "  Offset: 0x%llx\n"
        "  Size: %zu\n"
        "  Alignment: %llu\n",
        section->name,
        elf_section_type_to_string(section->type),
        (unsigned long long)section->flags,
        (unsigned long long)section->address,
        (unsigned long long)section->offset,
        section->data_size,
        (unsigned long long)section->alignment
    );
}

char *elf_generate_unique_label(ELFWriter *writer, const char *prefix) {
    if (!writer || !prefix) return NULL;
    
    static int label_counter = 0;
    char *label = malloc(strlen(prefix) + 32);
    if (!label) return NULL;
    
    snprintf(label, strlen(prefix) + 32, "%s_%d", prefix, label_counter++);
    return label;
}

char *elf_generate_unique_symbol_name(ELFWriter *writer, const char *prefix) {
    if (!writer || !prefix) return NULL;
    
    static int symbol_counter = 0;
    char *name = malloc(strlen(prefix) + 32);
    if (!name) return NULL;
    
    snprintf(name, strlen(prefix) + 32, "%s_%d", prefix, symbol_counter++);
    return name;
}

bool elf_is_reserved_symbol_name(const char *name) {
    if (!name) return false;
    
    // Check for reserved ELF symbol names
    const char *reserved[] = {
        "_start", "_init", "_fini", "_GLOBAL_OFFSET_TABLE_",
        "_DYNAMIC", "_etext", "_edata", "_end",
        NULL
    };
    
    for (int i = 0; reserved[i]; i++) {
        if (strcmp(name, reserved[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

bool elf_is_reserved_section_name(const char *name) {
    if (!name) return false;
    
    // Check for reserved ELF section names
    const char *reserved[] = {
        ".text", ".data", ".bss", ".rodata", ".symtab", ".strtab", ".shstrtab",
        ".rela.text", ".rela.data", ".rel.text", ".rel.data",
        ".note", ".comment", ".debug_info", ".debug_line",
        NULL
    };
    
    for (int i = 0; reserved[i]; i++) {
        if (strcmp(name, reserved[i]) == 0) {
            return true;
        }
    }
    
    return false;
}

char *elf_sanitize_symbol_name(const char *name) {
    if (!name) return NULL;
    
    size_t len = strlen(name);
    char *sanitized = malloc(len + 1);
    if (!sanitized) return NULL;
    
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (isalnum(c) || c == '_' || c == '$' || c == '.') {
            sanitized[i] = c;
        } else {
            sanitized[i] = '_';
        }
    }
    sanitized[len] = '\0';
    
    return sanitized;
}

char *elf_sanitize_section_name(const char *name) {
    if (!name) return NULL;
    
    size_t len = strlen(name);
    char *sanitized = malloc(len + 1);
    if (!sanitized) return NULL;
    
    for (size_t i = 0; i < len; i++) {
        char c = name[i];
        if (isalnum(c) || c == '_' || c == '.') {
            sanitized[i] = c;
        } else {
            sanitized[i] = '_';
        }
    }
    sanitized[len] = '\0';
    
    return sanitized;
}

uint32_t elf_calculate_section_checksum(ELFSection *section) {
    if (!section || !section->data || section->data_size == 0) {
        return 0;
    }
    
    uint32_t checksum = 0;
    const uint8_t *data = (const uint8_t*)section->data;
    
    for (size_t i = 0; i < section->data_size; i++) {
        checksum = ((checksum << 1) | (checksum >> 31)) ^ data[i];
    }
    
    return checksum;
}

uint32_t elf_calculate_symbol_table_checksum(ELFWriter *writer) {
    if (!writer) return 0;
    
    uint32_t checksum = 0;
    
    for (size_t i = 0; i < writer->symbol_count; i++) {
        ELFSymbol *symbol = writer->symbols[i];
        if (symbol && symbol->name) {
            const char *name = symbol->name;
            while (*name) {
                checksum = ((checksum << 1) | (checksum >> 31)) ^ (uint8_t)*name;
                name++;
            }
            checksum ^= (uint32_t)symbol->value;
            checksum ^= (uint32_t)symbol->size;
        }
    }
    
    return checksum;
}

uint32_t elf_calculate_file_checksum(ELFWriter *writer) {
    if (!writer) return 0;
    
    uint32_t checksum = 0;
    
    // Checksum the header
    const uint8_t *header_data = (const uint8_t*)&writer->header;
    for (size_t i = 0; i < sizeof(Elf64_Ehdr); i++) {
        checksum = ((checksum << 1) | (checksum >> 31)) ^ header_data[i];
    }
    
    // Checksum all sections
    for (size_t i = 0; i < writer->section_count; i++) {
        checksum ^= elf_calculate_section_checksum(writer->sections[i]);
    }
    
    // Checksum symbol table
    checksum ^= elf_calculate_symbol_table_checksum(writer);
    
    return checksum;
}

void elf_clear_writer(ELFWriter *writer) {
    if (!writer) return;
    
    // Clear sections (but don't free the writer structure itself)
    for (size_t i = 0; i < writer->section_count; i++) {
        if (writer->sections[i]) {
            free(writer->sections[i]->name);
            free(writer->sections[i]->data);
            free(writer->sections[i]);
        }
    }
    writer->section_count = 0;
    
    // Clear symbols
    for (size_t i = 0; i < writer->symbol_count; i++) {
        if (writer->symbols[i]) {
            free(writer->symbols[i]->name);
            free(writer->symbols[i]);
        }
    }
    writer->symbol_count = 0;
    
    // Clear relocations
    for (size_t i = 0; i < writer->relocation_count; i++) {
        if (writer->relocations[i]) {
            free(writer->relocations[i]);
        }
    }
    writer->relocation_count = 0;
    
    // Clear string tables
    writer->string_table_size = 1; // Keep null byte
    writer->section_string_table_size = 1;
    
    // Reset statistics
    writer->sections_created_count = 0;
    writer->symbols_added_count = 0;
    writer->relocations_added_count = 0;
    writer->asthra_metadata_entries_count = 0;
} 
