/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Main Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ELF_WRITER_H
#define ASTHRA_ELF_WRITER_H

#include "elf_writer_core.h"
#include "elf_writer_utils.h"
#include "elf_writer_sections.h"
#include "elf_writer_symbols.h"
#include "elf_writer_relocations.h"

// Export all necessary functions from core
ELFSection *elf_add_section(ELFWriter *writer, const char *name, 
                           uint32_t type, uint64_t flags);
ELFSymbol *elf_add_symbol(ELFWriter *writer, const char *name,
                         uint64_t value, uint64_t size,
                         uint8_t type, uint8_t binding, uint16_t section);

// Section management
bool elf_create_standard_sections(ELFWriter *writer);
bool elf_create_asthra_sections(ELFWriter *writer);
bool elf_populate_text_section(ELFWriter *writer);
bool elf_populate_ffi_section(ELFWriter *writer);
bool elf_populate_gc_section(ELFWriter *writer);
bool elf_populate_security_section(ELFWriter *writer);
bool elf_populate_pattern_matching_section(ELFWriter *writer);
bool elf_populate_string_ops_section(ELFWriter *writer);
bool elf_populate_slice_meta_section(ELFWriter *writer);
bool elf_populate_concurrency_section(ELFWriter *writer);

// Utils
bool elf_calculate_layout(ELFWriter *writer);
ELFSection *elf_find_section_by_name(ELFWriter *writer, const char *name);
bool elf_expand_section_data(ELFSection *section, size_t required_size);

// Runtime initialization
bool elf_generate_runtime_init(ELFWriter *writer);
bool elf_add_runtime_init_metadata(ELFWriter *writer);

// Optimization
bool elf_optimize_metadata(ELFWriter *writer);

// Debug information types (stubs for DWARF support)
typedef struct {
    const char *producer;
    uint32_t language;
    const char *directory;
    const char *filename;
    const char *source_filename;
    uint64_t low_pc;
    uint64_t high_pc;
    uint32_t stmt_list_offset;
} ELFCompilationUnit;

typedef struct {
    const char *name;
    uint64_t low_pc;
    uint64_t high_pc;
    const char *file;
    uint32_t line;
    const char *return_type;
    uint32_t frame_base;
    uint32_t param_count;
    void *params;
} ELFFunctionDebugInfo;

typedef struct {
    const char *name;
    uint32_t byte_size;
    uint32_t encoding;
    uint32_t tag;
    uint32_t base_type_ref;
    uint32_t array_size;
    uint32_t member_count;
    void *members;
} ELFTypeDebugInfo;

typedef struct {
    const char *name;
    const char *type;
    uint64_t location;
    const char *file;
    uint32_t line;
} ELFVariableDebugInfo;

typedef struct {
    const char *name;
    const char *type;
    uint32_t location;
} ELFParameterDebugInfo;

typedef struct {
    const char *name;
    const char *type;
    uint32_t offset;
    uint32_t bit_size;
    uint32_t bit_offset;
} ELFStructMemberDebugInfo;

typedef struct {
    uint32_t op_count;
    uint32_t ops[16]; // Support up to 16 operations
} ELFDwarfExpression;

// Debug function declarations
bool elf_writer_add_compilation_unit(ELFWriter *writer, const ELFCompilationUnit *cu_info);
bool elf_writer_add_function_debug_info(ELFWriter *writer, const ELFFunctionDebugInfo *func_info);
bool elf_writer_add_type_debug_info(ELFWriter *writer, const ELFTypeDebugInfo *type_info);
bool elf_writer_add_variable_debug_info(ELFWriter *writer, const ELFVariableDebugInfo *var_info);
bool elf_writer_add_parameter_debug_info(ELFWriter *writer, const ELFParameterDebugInfo *param_info);
bool elf_writer_add_struct_member_debug_info(ELFWriter *writer, const ELFStructMemberDebugInfo *member_info);
bool elf_writer_add_dwarf_expression(ELFWriter *writer, const char *name, const ELFDwarfExpression *expression);

// Additional debug and validation functions
bool elf_generate_debug_info(ELFWriter *writer);
bool elf_validate_structure(ELFWriter *writer);
bool elf_validate_c_compatibility(ELFWriter *writer);
bool elf_validate_debug_symbols(ELFWriter *writer);

// DWARF generation functions
bool elf_writer_generate_dwarf_cu(ELFWriter *writer);
size_t elf_writer_get_cu_size(ELFWriter *writer);
bool elf_writer_generate_function_debug_info(ELFWriter *writer);
size_t elf_writer_get_function_debug_size(ELFWriter *writer);
bool elf_writer_generate_type_debug_info(ELFWriter *writer);
size_t elf_writer_get_type_debug_size(ELFWriter *writer);

// Additional DWARF functions
bool elf_writer_generate_dwarf_expressions(ELFWriter *writer);
bool elf_writer_validate_dwarf_expressions(ELFWriter *writer);
bool elf_writer_generate_dwarf_abbreviations(ELFWriter *writer);
size_t elf_writer_get_abbreviation_table_size(ELFWriter *writer);
bool elf_writer_has_abbreviation(ELFWriter *writer, uint32_t tag);

#endif // ASTHRA_ELF_WRITER_H