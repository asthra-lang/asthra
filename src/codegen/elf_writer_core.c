/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Core Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "elf_writer_core.h"
#include "elf_writer.h"
#include "elf_writer_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// =============================================================================
// ELF WRITER CREATION AND DESTRUCTION
// =============================================================================

ELFWriter *elf_writer_create(FFIAssemblyGenerator *ffi_generator) {
    ELFWriter *writer = calloc(1, sizeof(ELFWriter));
    if (!writer) return NULL;
    
    writer->ffi_generator = ffi_generator;
    
    // Initialize ELF header
    memcpy(writer->header.e_ident, ELFMAG, SELFMAG);
    writer->header.e_ident[EI_CLASS] = ELFCLASS64;
    writer->header.e_ident[EI_DATA] = ELFDATA2LSB;
    writer->header.e_ident[EI_VERSION] = EV_CURRENT;
    writer->header.e_ident[EI_OSABI] = ELFOSABI_SYSV;
    writer->header.e_ident[EI_ABIVERSION] = 0;
    
    writer->header.e_type = ET_REL;
    writer->header.e_machine = EM_X86_64;
    writer->header.e_version = EV_CURRENT;
    writer->header.e_ehsize = sizeof(Elf64_Ehdr);
    writer->header.e_shentsize = sizeof(Elf64_Shdr);
    
    // Initialize string tables
    writer->string_table_capacity = 1024;
    writer->string_table = calloc(1, writer->string_table_capacity);
    writer->string_table_size = 1; // Start with null byte
    
    writer->section_string_table_capacity = 1024;
    writer->section_string_table = calloc(1, writer->section_string_table_capacity);
    writer->section_string_table_size = 1; // Start with null byte
    
    // Initialize arrays
    writer->section_capacity = 16;
    writer->sections = calloc(writer->section_capacity, sizeof(ELFSection*));
    
    writer->symbol_capacity = 64;
    writer->symbols = calloc(writer->symbol_capacity, sizeof(ELFSymbol*));
    
    writer->relocation_capacity = 64;
    writer->relocations = calloc(writer->relocation_capacity, sizeof(ELFRelocation*));
    
    // Create Asthra metadata
    writer->asthra_metadata = calloc(1, sizeof(AsthraSectionMetadata));
    writer->asthra_metadata->metadata_version = ASTHRA_METADATA_VERSION;
    
    // Set default configuration
    writer->config.generate_debug_info = true;
    writer->config.validate_elf_structure = true;
    writer->config.enable_all_asthra_sections = true;
    
    return writer;
}

void elf_writer_destroy(ELFWriter *writer) {
    if (!writer) return;
    
    // Free sections
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        if (section) {
            free(section->name);
            free(section->data);
            free(section);
        }
    }
    free(writer->sections);
    
    // Free symbols
    for (size_t i = 0; i < writer->symbol_count; i++) {
        ELFSymbol *symbol = writer->symbols[i];
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
    
    // Free string tables
    free(writer->string_table);
    free(writer->section_string_table);
    
    // Free metadata
    free(writer->asthra_metadata);
    
    free(writer);
}

// =============================================================================
// OBJECT FILE GENERATION
// =============================================================================

bool elf_generate_object_file(ELFWriter *writer, ASTNode *program, 
                             const char *output_filename) {
    printf("DEBUG: elf_generate_object_file - Entry\n");
    fflush(stdout);
    
    if (!writer || !output_filename) {
        printf("DEBUG: elf_generate_object_file - Invalid parameters\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: elf_generate_object_file - About to call ffi_generate_program\n");
    fflush(stdout);
    
    // Generate code from the AST program
    if (!ffi_generate_program(writer->ffi_generator, program)) {
        printf("DEBUG: elf_generate_object_file - ffi_generate_program failed\n");
        fflush(stdout);
        fprintf(stderr, "ERROR: Failed to generate code from AST\n");
        return false;
    }
    
    printf("DEBUG: elf_generate_object_file - ffi_generate_program succeeded\n");
    fflush(stdout);
    
    printf("DEBUG: elf_generate_object_file - About to call elf_create_standard_sections\n");
    fflush(stdout);
    
    // Create standard ELF sections
    if (!elf_create_standard_sections(writer)) {
        printf("DEBUG: elf_generate_object_file - elf_create_standard_sections failed\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: elf_generate_object_file - elf_create_standard_sections succeeded\n");
    fflush(stdout);
    
    // Create Asthra-specific sections
    if (writer->config.enable_all_asthra_sections) {
        printf("DEBUG: elf_generate_object_file - About to call elf_create_asthra_sections\n");
        fflush(stdout);
        if (!elf_create_asthra_sections(writer)) {
            printf("DEBUG: elf_generate_object_file - elf_create_asthra_sections failed\n");
            fflush(stdout);
            return false;
        }
        printf("DEBUG: elf_generate_object_file - elf_create_asthra_sections succeeded\n");
        fflush(stdout);
    }
    
    printf("DEBUG: elf_generate_object_file - About to call elf_populate_text_section\n");
    fflush(stdout);
    
    // Populate sections with generated content
    if (!elf_populate_text_section(writer)) {
        printf("DEBUG: elf_generate_object_file - elf_populate_text_section failed\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: elf_generate_object_file - elf_populate_text_section succeeded\n");
    fflush(stdout);
    
    printf("DEBUG: elf_generate_object_file - About to call elf_write_object_file\n");
    fflush(stdout);
    
    // Write the ELF file
    bool result = elf_write_object_file(writer, output_filename);
    
    printf("DEBUG: elf_generate_object_file - elf_write_object_file returned: %d\n", result);
    fflush(stdout);
    
    return result;
}

// =============================================================================
// FILE WRITING
// =============================================================================

bool elf_write_object_file(ELFWriter *writer, const char *filename) {
    if (!writer || !filename) return false;
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "ERROR: Failed to open output file: %s\n", filename);
        return false;
    }
    
    // Update section header string table index
    writer->header.e_shstrndx = 0;
    for (uint16_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, ".shstrtab") == 0) {
            writer->header.e_shstrndx = i;
            break;
        }
    }
    
    // Populate string tables
    printf("DEBUG: Starting section name offset assignment for %zu sections\n", writer->section_count);
    fflush(stdout);
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        printf("DEBUG: Section %zu: name='%s'\n", i, section->name ? section->name : "NULL");
        fflush(stdout);
        section->name_offset = elf_add_string_to_table(
            &writer->section_string_table,
            &writer->section_string_table_size,
            &writer->section_string_table_capacity,
            section->name
        );
        printf("DEBUG: Section %zu: assigned name_offset=%u\n", i, section->name_offset);
        fflush(stdout);
    }
    
    // Update .shstrtab section data
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, ".shstrtab") == 0) {
            printf("DEBUG: Updating .shstrtab section data, size=%zu\n", writer->section_string_table_size);
            printf("DEBUG: String table contents: ");
            for (size_t j = 0; j < writer->section_string_table_size && j < 100; j++) {
                if (writer->section_string_table[j] == 0) {
                    printf("\\0");
                } else {
                    printf("%c", writer->section_string_table[j]);
                }
            }
            printf("\n");
            fflush(stdout);
            
            writer->sections[i]->data = writer->section_string_table;
            writer->sections[i]->data_size = writer->section_string_table_size;
            break;
        }
    }
    
    // Populate symbol string table BEFORE layout calculation
    for (size_t i = 0; i < writer->symbol_count; i++) {
        ELFSymbol *symbol = writer->symbols[i];
        symbol->name_offset = elf_add_string_to_table(
            &writer->string_table,
            &writer->string_table_size,
            &writer->string_table_capacity,
            symbol->name
        );
    }
    
    // Update .strtab section data BEFORE layout calculation
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, ".strtab") == 0) {
            writer->sections[i]->data = writer->string_table;
            writer->sections[i]->data_size = writer->string_table_size;
            break;
        }
    }
    
    // Set up symbol table data BEFORE layout calculation
    size_t symtab_idx = 0;
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->name, ".symtab") == 0) {
            symtab_idx = i;
            break;
        }
    }
    
    if (symtab_idx > 0) {
        ELFSection *symtab = writer->sections[symtab_idx];
        size_t sym_size = (writer->symbol_count + 1) * sizeof(Elf64_Sym); // +1 for null symbol
        
        Elf64_Sym *syms = calloc(writer->symbol_count + 1, sizeof(Elf64_Sym));
        if (!syms) {
            return false;
        }
        
        // First symbol is always null
        syms[0].st_name = 0;
        syms[0].st_value = 0;
        syms[0].st_size = 0;
        syms[0].st_info = 0;
        syms[0].st_other = 0;
        syms[0].st_shndx = SHN_UNDEF;
        
        // Add other symbols
        for (size_t i = 0; i < writer->symbol_count; i++) {
            ELFSymbol *symbol = writer->symbols[i];
            syms[i + 1].st_name = symbol->name_offset;
            syms[i + 1].st_value = symbol->value;
            syms[i + 1].st_size = symbol->size;
            syms[i + 1].st_info = symbol->info;
            syms[i + 1].st_other = symbol->other;
            syms[i + 1].st_shndx = symbol->section_index;
        }
        
        // Update symtab section data
        symtab->data = syms;
        symtab->data_size = sym_size;
        
        printf("DEBUG: Set up symbol table data before layout, size=%zu\n", sym_size);
        fflush(stdout);
        
        // Update link to string table
        for (size_t i = 0; i < writer->section_count; i++) {
            if (strcmp(writer->sections[i]->name, ".strtab") == 0) {
                symtab->link = i;
                break;
            }
        }
    }
    
    // Calculate layout AFTER ALL section data sizes are finalized
    printf("DEBUG: Recalculating layout after all section data sizes are set\n");
    fflush(stdout);
    elf_calculate_layout(writer);
    
    // Update header
    writer->header.e_shoff = sizeof(Elf64_Ehdr);
    writer->header.e_shnum = writer->section_count;
    
    // Write ELF header
    if (fwrite(&writer->header, sizeof(Elf64_Ehdr), 1, file) != 1) {
        fclose(file);
        return false;
    }
    
    // Write section headers
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        Elf64_Shdr shdr = {0};
        
        printf("DEBUG: Writing section %zu '%s' with name_offset=%u\n", 
               i, section->name ? section->name : "NULL", section->name_offset);
        fflush(stdout);
        
        shdr.sh_name = section->name_offset;
        shdr.sh_type = section->type;
        shdr.sh_flags = section->flags;
        shdr.sh_addr = section->address;
        shdr.sh_offset = section->offset;
        shdr.sh_size = section->data_size;
        shdr.sh_link = section->link;
        shdr.sh_info = section->info;
        shdr.sh_addralign = section->alignment;
        shdr.sh_entsize = section->entry_size;
        
        if (fwrite(&shdr, sizeof(Elf64_Shdr), 1, file) != 1) {
            fclose(file);
            return false;
        }
    }
    
    // Write section data
    for (size_t i = 0; i < writer->section_count; i++) {
        ELFSection *section = writer->sections[i];
        if (section->type != SHT_NULL && section->type != SHT_NOBITS && section->data_size > 0) {
            printf("DEBUG: Writing section %zu '%s' data: seeking to offset=0x%llx, size=%zu\n", 
                   i, section->name ? section->name : "NULL", 
                   (unsigned long long)section->offset, section->data_size);
            fflush(stdout);
            
            // Seek to section offset
            if (fseek(file, section->offset, SEEK_SET) != 0) {
                printf("DEBUG: Failed to seek to offset 0x%llx for section %zu\n", 
                       (unsigned long long)section->offset, i);
                fflush(stdout);
                fclose(file);
                return false;
            }
            
            // Write section data
            if (fwrite(section->data, 1, section->data_size, file) != section->data_size) {
                printf("DEBUG: Failed to write %zu bytes for section %zu\n", section->data_size, i);
                fflush(stdout);
                fclose(file);
                return false;
            }
            
            printf("DEBUG: Successfully wrote section %zu data\n", i);
            fflush(stdout);
        }
    }
    
    // Symbol table data already set up before layout calculation
    
    fclose(file);
    
    printf("ELF structure validation passed\n");
    return true;
}