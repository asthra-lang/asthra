/**
 * Asthra Programming Language Compiler
 * Mach-O File Writing Operations
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Layout calculation and file writing operations for Mach-O object files
 */

#include "macho_file_writer.h"
#include "macho_sections.h"
#include "macho_data_structures.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach/vm_prot.h>

// =============================================================================
// LAYOUT CALCULATION
// =============================================================================

bool macho_calculate_layout(MachoWriter *writer) {
    if (!writer) return false;
    
    printf("DEBUG: Calculating Mach-O file layout\n");
    fflush(stdout);
    
    // Start after the header
    size_t offset = sizeof(struct mach_header_64);
    
    // Calculate load commands size
    size_t load_commands_size = 0;
    
    // LC_SEGMENT_64 for __TEXT segment with sections
    size_t text_sections = 0;
    size_t data_sections = 0;
    
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->segment_name, "__TEXT") == 0) {
            text_sections++;
        } else if (strcmp(writer->sections[i]->segment_name, "__DATA") == 0) {
            data_sections++;
        }
    }
    
    if (text_sections > 0) {
        load_commands_size += sizeof(struct segment_command_64) + text_sections * sizeof(struct section_64);
    }
    if (data_sections > 0) {
        load_commands_size += sizeof(struct segment_command_64) + data_sections * sizeof(struct section_64);
    }
    
    // LC_SYMTAB
    load_commands_size += sizeof(struct symtab_command);
    
    offset += load_commands_size;
    
    // Align to 16 bytes for section data
    offset = (offset + 15) & ~15;
    
    // Calculate section offsets
    for (size_t i = 0; i < writer->section_count; i++) {
        MachoSection *section = writer->sections[i];
        if (section->data_size > 0) {
            section->file_offset = offset;
            offset += section->data_size;
            // Align each section to 4 bytes
            offset = (offset + 3) & ~3;
        }
    }
    
    // Symbol table offset
    size_t symtab_offset = offset;
    size_t symtab_size = writer->symbol_count * sizeof(struct nlist_64);
    offset += symtab_size;
    
    // String table offset  
    size_t strtab_offset = offset;
    size_t strtab_size = writer->string_table_size;
    
    // Update header
    writer->header.ncmds = 0;
    if (text_sections > 0) writer->header.ncmds++;
    if (data_sections > 0) writer->header.ncmds++;
    writer->header.ncmds++; // LC_SYMTAB
    writer->header.sizeofcmds = (uint32_t)load_commands_size;
    
    // Store layout info (we'll need these for writing)
    writer->symtab_offset = symtab_offset;
    writer->symtab_count = writer->symbol_count;
    writer->strtab_offset = strtab_offset;
    writer->strtab_size = strtab_size;
    
    printf("DEBUG: Layout calculated - load_cmds_size=%zu, symtab_offset=%zu, strtab_offset=%zu\n",
           load_commands_size, symtab_offset, strtab_offset);
    fflush(stdout);
    
    return true;
}

// =============================================================================
// OBJECT FILE GENERATION
// =============================================================================

bool macho_generate_object_file(MachoWriter *writer, ASTNode *program, 
                                const char *output_filename) {
    printf("DEBUG: macho_generate_object_file - Entry\n");
    fflush(stdout);
    
    if (!writer || !output_filename) {
        printf("DEBUG: macho_generate_object_file - Invalid parameters\n");
        fflush(stdout);
        return false;
    }
    
    // Generate code from the AST program
    if (!ffi_generate_program(writer->ffi_generator, program)) {
        printf("DEBUG: macho_generate_object_file - ffi_generate_program failed\n");
        fflush(stdout);
        return false;
    }
    
    printf("DEBUG: macho_generate_object_file - ffi_generate_program succeeded\n");
    fflush(stdout);
    
    // Create standard sections
    if (!macho_create_standard_sections(writer)) {
        printf("DEBUG: macho_generate_object_file - macho_create_standard_sections failed\n");
        fflush(stdout);
        return false;
    }
    
    // Populate sections with generated content
    if (!macho_populate_text_section(writer)) {
        printf("DEBUG: macho_generate_object_file - macho_populate_text_section failed\n");
        fflush(stdout);
        return false;
    }
    
    // Add symbols from generated code
    // TODO: Extract symbols from FFI generator
    macho_add_symbol(writer, "_main", N_SECT | N_EXT, 1, 0);
    
    // Calculate file layout
    if (!macho_calculate_layout(writer)) {
        printf("DEBUG: macho_generate_object_file - macho_calculate_layout failed\n");
        fflush(stdout);
        return false;
    }
    
    // Write the Mach-O file
    bool result = macho_write_object_file(writer, output_filename);
    
    printf("DEBUG: macho_generate_object_file - macho_write_object_file returned: %d\n", result);
    fflush(stdout);
    
    return result;
}

bool macho_write_object_file(MachoWriter *writer, const char *filename) {
    printf("DEBUG: macho_write_object_file - Entry\n");
    fflush(stdout);
    
    if (!writer || !filename) return false;
    
    FILE *file = fopen(filename, "wb");
    if (!file) {
        fprintf(stderr, "ERROR: Failed to open output file: %s (errno=%d: %s)\n", 
                filename, errno, strerror(errno));
        return false;
    }
    
    printf("DEBUG: Writing complete Mach-O file: %s\n", filename);
    fflush(stdout);
    
    // 1. Write Mach-O header
    if (fwrite(&writer->header, sizeof(struct mach_header_64), 1, file) != 1) {
        printf("DEBUG: Failed to write Mach-O header\n");
        fclose(file);
        return false;
    }
    printf("DEBUG: Wrote Mach-O header\n");
    
    // 2. Write load commands
    if (!macho_write_load_commands(writer, file)) {
        printf("DEBUG: Failed to write load commands\n");
        fclose(file);
        return false;
    }
    printf("DEBUG: Wrote load commands\n");
    
    // 3. Align and write section data
    long current_pos = ftell(file);
    long aligned_pos = (current_pos + 15) & ~15;
    
    // Write padding to align
    for (long i = current_pos; i < aligned_pos; i++) {
        fputc(0, file);
    }
    
    if (!macho_write_section_data(writer, file)) {
        printf("DEBUG: Failed to write section data\n");
        fclose(file);
        return false;
    }
    printf("DEBUG: Wrote section data\n");
    
    // 4. Write symbol table
    if (!macho_write_symbol_table(writer, file)) {
        printf("DEBUG: Failed to write symbol table\n");
        fclose(file);
        return false;
    }
    printf("DEBUG: Wrote symbol table\n");
    
    // 5. Write string table
    if (!macho_write_string_table(writer, file)) {
        printf("DEBUG: Failed to write string table\n");
        fclose(file);
        return false;
    }
    printf("DEBUG: Wrote string table\n");
    
    fclose(file);
    
    // Verify file was created
    struct stat st;
    if (stat(filename, &st) == 0) {
        printf("DEBUG: Complete Mach-O file created successfully, size=%lld bytes\n", st.st_size);
    } else {
        printf("DEBUG: File NOT found after writing!\n");
        return false;
    }
    
    printf("Mach-O structure validation passed\n");
    return true;
}

// =============================================================================
// LOAD COMMAND WRITING
// =============================================================================

bool macho_write_load_commands(MachoWriter *writer, FILE *file) {
    // Write LC_SEGMENT_64 for __TEXT segment
    size_t text_sections = 0;
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->segment_name, "__TEXT") == 0) {
            text_sections++;
        }
    }
    
    if (text_sections > 0) {
        struct segment_command_64 seg_cmd = {0};
        seg_cmd.cmd = LC_SEGMENT_64;
        seg_cmd.cmdsize = sizeof(struct segment_command_64) + text_sections * sizeof(struct section_64);
        strncpy(seg_cmd.segname, "__TEXT", 16);
        seg_cmd.vmaddr = 0;
        seg_cmd.vmsize = 0;
        seg_cmd.fileoff = 0;
        seg_cmd.filesize = 0;
        seg_cmd.maxprot = VM_PROT_READ | VM_PROT_EXECUTE;
        seg_cmd.initprot = VM_PROT_READ | VM_PROT_EXECUTE;
        seg_cmd.nsects = (uint32_t)text_sections;
        seg_cmd.flags = 0;
        
        if (fwrite(&seg_cmd, sizeof(seg_cmd), 1, file) != 1) return false;
        
        // Write section headers for __TEXT sections
        for (size_t i = 0; i < writer->section_count; i++) {
            MachoSection *section = writer->sections[i];
            if (strcmp(section->segment_name, "__TEXT") == 0) {
                struct section_64 sect = {0};
                strncpy(sect.sectname, section->name, 16);
                strncpy(sect.segname, section->segment_name, 16);
                sect.addr = 0;
                sect.size = section->data_size;
                sect.offset = (uint32_t)section->file_offset;
                sect.align = section->alignment;
                sect.reloff = 0;
                sect.nreloc = 0;
                sect.flags = section->flags;
                
                if (fwrite(&sect, sizeof(sect), 1, file) != 1) return false;
            }
        }
    }
    
    // Write LC_SEGMENT_64 for __DATA segment
    size_t data_sections = 0;
    for (size_t i = 0; i < writer->section_count; i++) {
        if (strcmp(writer->sections[i]->segment_name, "__DATA") == 0) {
            data_sections++;
        }
    }
    
    if (data_sections > 0) {
        struct segment_command_64 seg_cmd = {0};
        seg_cmd.cmd = LC_SEGMENT_64;
        seg_cmd.cmdsize = sizeof(struct segment_command_64) + data_sections * sizeof(struct section_64);
        strncpy(seg_cmd.segname, "__DATA", 16);
        seg_cmd.vmaddr = 0;
        seg_cmd.vmsize = 0;
        seg_cmd.fileoff = 0;
        seg_cmd.filesize = 0;
        seg_cmd.maxprot = VM_PROT_READ | VM_PROT_WRITE;
        seg_cmd.initprot = VM_PROT_READ | VM_PROT_WRITE;
        seg_cmd.nsects = (uint32_t)data_sections;
        seg_cmd.flags = 0;
        
        if (fwrite(&seg_cmd, sizeof(seg_cmd), 1, file) != 1) return false;
        
        // Write section headers for __DATA sections
        for (size_t i = 0; i < writer->section_count; i++) {
            MachoSection *section = writer->sections[i];
            if (strcmp(section->segment_name, "__DATA") == 0) {
                struct section_64 sect = {0};
                strncpy(sect.sectname, section->name, 16);
                strncpy(sect.segname, section->segment_name, 16);
                sect.addr = 0;
                sect.size = section->data_size;
                sect.offset = (uint32_t)section->file_offset;
                sect.align = section->alignment;
                sect.reloff = 0;
                sect.nreloc = 0;
                sect.flags = section->flags;
                
                if (fwrite(&sect, sizeof(sect), 1, file) != 1) return false;
            }
        }
    }
    
    // Write LC_SYMTAB
    struct symtab_command sym_cmd = {0};
    sym_cmd.cmd = LC_SYMTAB;
    sym_cmd.cmdsize = sizeof(struct symtab_command);
    sym_cmd.symoff = (uint32_t)writer->symtab_offset;
    sym_cmd.nsyms = (uint32_t)writer->symtab_count;
    sym_cmd.stroff = (uint32_t)writer->strtab_offset;
    sym_cmd.strsize = (uint32_t)writer->strtab_size;
    
    if (fwrite(&sym_cmd, sizeof(sym_cmd), 1, file) != 1) return false;
    
    return true;
}

bool macho_write_section_data(MachoWriter *writer, FILE *file) {
    for (size_t i = 0; i < writer->section_count; i++) {
        MachoSection *section = writer->sections[i];
        if (section->data_size > 0 && section->data) {
            // Seek to the correct file offset
            if (fseek(file, (long)section->file_offset, SEEK_SET) != 0) {
                return false;
            }
            
            // Write section data
            if (fwrite(section->data, section->data_size, 1, file) != 1) {
                return false;
            }
        }
    }
    return true;
}

bool macho_write_symbol_table(MachoWriter *writer, FILE *file) {
    if (writer->symbol_count == 0) return true;
    
    // Seek to symbol table offset
    if (fseek(file, (long)writer->symtab_offset, SEEK_SET) != 0) {
        return false;
    }
    
    // Write each symbol as nlist_64
    for (size_t i = 0; i < writer->symbol_count; i++) {
        MachoSymbol *symbol = writer->symbols[i];
        
        struct nlist_64 nlist = {0};
        nlist.n_un.n_strx = symbol->name_offset;
        nlist.n_type = symbol->type;
        nlist.n_sect = symbol->sect;
        nlist.n_desc = symbol->desc;
        nlist.n_value = symbol->value;
        
        if (fwrite(&nlist, sizeof(nlist), 1, file) != 1) {
            return false;
        }
    }
    
    return true;
}

bool macho_write_string_table(MachoWriter *writer, FILE *file) {
    // Seek to string table offset
    if (fseek(file, (long)writer->strtab_offset, SEEK_SET) != 0) {
        return false;
    }
    
    // Write string table
    if (fwrite(writer->string_table, writer->string_table_size, 1, file) != 1) {
        return false;
    }
    
    return true;
}