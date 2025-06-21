/**
 * Asthra Programming Language Compiler
 * Mach-O Object File Writer - Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_MACHO_WRITER_H
#define ASTHRA_MACHO_WRITER_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach/vm_prot.h>
#include "ffi_assembly_generator.h"
#include "../parser/ast_types.h"

// =============================================================================
// MACH-O STRUCTURES
// =============================================================================

/**
 * Mach-O Section representation
 */
typedef struct MachoSection {
    char *name;                    // Section name (e.g., "__text")
    char *segment_name;            // Segment name (e.g., "__TEXT")
    uint32_t flags;                // Section flags
    uint32_t alignment;            // Alignment requirement
    uint64_t address;              // Virtual address
    uint64_t file_offset;          // File offset
    void *data;                    // Section data
    size_t data_size;              // Size of section data
    size_t data_capacity;          // Allocated capacity
    uint32_t reserved1;            // Reserved field 1
    uint32_t reserved2;            // Reserved field 2
} MachoSection;

/**
 * Mach-O Symbol representation
 */
typedef struct MachoSymbol {
    char *name;                    // Symbol name
    uint32_t name_offset;          // Offset in string table
    uint8_t type;                  // Symbol type (N_TYPE)
    uint8_t sect;                  // Section number
    uint16_t desc;                 // Symbol descriptor
    uint64_t value;                // Symbol value
    bool is_external;              // External symbol flag
} MachoSymbol;

/**
 * Mach-O Relocation entry
 */
typedef struct MachoRelocation {
    uint32_t address;              // Address to relocate
    uint32_t symbolnum;            // Symbol table index
    uint8_t pcrel;                 // PC-relative flag
    uint8_t length;                // Relocation length
    uint8_t type;                  // Relocation type
    bool external;                 // External relocation flag
} MachoRelocation;

/**
 * Mach-O Writer configuration
 */
typedef struct MachoWriterConfig {
    bool generate_debug_info;      // Generate debug information
    bool validate_structure;       // Validate Mach-O structure
    bool enable_optimizations;     // Enable optimizations
    uint32_t cpu_type;             // CPU type (e.g., CPU_TYPE_ARM64)
    uint32_t cpu_subtype;          // CPU subtype
} MachoWriterConfig;

/**
 * Main Mach-O Writer structure
 */
typedef struct MachoWriter {
    // Mach-O header
    struct mach_header_64 header;
    
    // Load commands
    void *load_commands;
    size_t load_commands_size;
    size_t load_commands_capacity;
    uint32_t load_command_count;
    
    // Sections
    MachoSection **sections;
    size_t section_count;
    size_t section_capacity;
    
    // Symbols
    MachoSymbol **symbols;
    size_t symbol_count;
    size_t symbol_capacity;
    
    // Relocations
    MachoRelocation **relocations;
    size_t relocation_count;
    size_t relocation_capacity;
    
    // String table
    char *string_table;
    size_t string_table_size;
    size_t string_table_capacity;
    
    // Configuration
    MachoWriterConfig config;
    
    // FFI generator reference
    FFIAssemblyGenerator *ffi_generator;
    
    // Layout information
    size_t symtab_offset;
    size_t symtab_count;
    size_t strtab_offset;
    size_t strtab_size;
    
    // Statistics
    size_t sections_created_count;
    size_t symbols_added_count;
    size_t relocations_added_count;
} MachoWriter;

// =============================================================================
// CORE FUNCTIONS
// =============================================================================

/**
 * Create a new Mach-O writer
 */
MachoWriter *macho_writer_create(FFIAssemblyGenerator *ffi_generator);

/**
 * Destroy a Mach-O writer and free all resources
 */
void macho_writer_destroy(MachoWriter *writer);

/**
 * Generate a Mach-O object file from an AST program
 */
bool macho_generate_object_file(MachoWriter *writer, ASTNode *program, 
                                const char *output_filename);

/**
 * Write the Mach-O object file to disk
 */
bool macho_write_object_file(MachoWriter *writer, const char *filename);

// =============================================================================
// SECTION MANAGEMENT
// =============================================================================

/**
 * Add a new section to the Mach-O file
 */
MachoSection *macho_add_section(MachoWriter *writer, const char *name,
                               const char *segment_name, uint32_t flags);

/**
 * Find a section by name
 */
MachoSection *macho_find_section_by_name(MachoWriter *writer, const char *name);

/**
 * Set section data
 */
bool macho_set_section_data(MachoSection *section, const void *data, size_t size);

/**
 * Append data to a section
 */
bool macho_append_section_data(MachoSection *section, const void *data, size_t size);

// =============================================================================
// SYMBOL MANAGEMENT
// =============================================================================

/**
 * Add a symbol to the symbol table
 */
MachoSymbol *macho_add_symbol(MachoWriter *writer, const char *name,
                             uint8_t type, uint8_t sect, uint64_t value);

/**
 * Find a symbol by name
 */
MachoSymbol *macho_find_symbol_by_name(MachoWriter *writer, const char *name);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Add a string to the string table and return its offset
 */
uint32_t macho_add_string_to_table(char **table, size_t *size, 
                                   size_t *capacity, const char *str);

/**
 * Calculate the file layout and offsets
 */
bool macho_calculate_layout(MachoWriter *writer);

/**
 * Get statistics about the Mach-O file
 */
void macho_get_statistics(MachoWriter *writer, size_t *sections, size_t *symbols,
                         size_t *relocations);

/**
 * Print debug information about the Mach-O structure
 */
void macho_print_structure(MachoWriter *writer);

// =============================================================================
// STANDARD SECTIONS
// =============================================================================

/**
 * Create standard Mach-O sections (__TEXT.__text, __DATA.__data, etc.)
 */
bool macho_create_standard_sections(MachoWriter *writer);

/**
 * Populate the __text section with generated code
 */
bool macho_populate_text_section(MachoWriter *writer);

// =============================================================================
// PLATFORM DETECTION
// =============================================================================

/**
 * Get the appropriate CPU type for the current platform
 */
uint32_t macho_get_cpu_type(void);

/**
 * Get the appropriate CPU subtype for the current platform
 */
uint32_t macho_get_cpu_subtype(void);

// =============================================================================
// INTERNAL WRITING FUNCTIONS
// =============================================================================

/**
 * Write load commands to file
 */
bool macho_write_load_commands(MachoWriter *writer, FILE *file);

/**
 * Write section data to file
 */
bool macho_write_section_data(MachoWriter *writer, FILE *file);

/**
 * Write symbol table to file
 */
bool macho_write_symbol_table(MachoWriter *writer, FILE *file);

/**
 * Write string table to file
 */
bool macho_write_string_table(MachoWriter *writer, FILE *file);

#endif // ASTHRA_MACHO_WRITER_H