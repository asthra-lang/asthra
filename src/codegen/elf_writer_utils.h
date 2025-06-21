/**
 * Asthra Programming Language Compiler
 * ELF-64 Object File Writer - Utility Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_ELF_WRITER_UTILS_H
#define ASTHRA_ELF_WRITER_UTILS_H

#include "elf_writer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STRING TABLE MANAGEMENT
// =============================================================================

/**
 * Add string to string table and return offset
 */
uint32_t elf_add_string_to_table(char **table, size_t *size, 
                                 size_t *capacity, const char *str);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Find section by name
 */
ELFSection *elf_find_section_by_name(ELFWriter *writer, const char *name);

/**
 * Expand section data capacity to accommodate required size
 */
bool elf_expand_section_data(ELFSection *section, size_t required_size);

/**
 * Get ELF writer statistics
 */
void elf_get_statistics(ELFWriter *writer, size_t *sections, size_t *symbols,
                       size_t *relocations, size_t *metadata_entries);

/**
 * Print ELF structure for debugging
 */
void elf_print_structure(ELFWriter *writer);

/**
 * Get section by name
 */
ELFSection *elf_get_section_by_name(ELFWriter *writer, const char *name);

/**
 * Get section by index
 */
ELFSection *elf_get_section_by_index(ELFWriter *writer, size_t index);

/**
 * Get section index by name
 */
int elf_get_section_index_by_name(ELFWriter *writer, const char *name);

/**
 * Calculate total file size
 */
size_t elf_calculate_file_size(ELFWriter *writer);

/**
 * Calculate section offsets and sizes
 */
bool elf_calculate_layout(ELFWriter *writer);

/**
 * Align value to specified boundary
 */
uint64_t elf_align_value(uint64_t value, uint64_t alignment);

/**
 * Check if value is aligned to boundary
 */
bool elf_is_aligned(uint64_t value, uint64_t alignment);

/**
 * Convert ELF symbol type to string
 */
const char *elf_symbol_type_to_string(uint8_t type);

/**
 * Convert ELF symbol binding to string
 */
const char *elf_symbol_binding_to_string(uint8_t binding);

/**
 * Convert ELF section type to string
 */
const char *elf_section_type_to_string(uint32_t type);

/**
 * Convert ELF relocation type to string
 */
const char *elf_relocation_type_to_string(uint32_t type);

/**
 * Format ELF header for display
 */
void elf_format_header_info(ELFWriter *writer, char *buffer, size_t buffer_size);

/**
 * Format section info for display
 */
void elf_format_section_info(ELFSection *section, char *buffer, size_t buffer_size);

/**
 * Format symbol info for display
 */
void elf_format_symbol_info(ELFSymbol *symbol, char *buffer, size_t buffer_size);

/**
 * Format relocation info for display
 */
void elf_format_relocation_info(ELFRelocation *relocation, char *buffer, size_t buffer_size);

/**
 * Generate unique label name
 */
char *elf_generate_unique_label(ELFWriter *writer, const char *prefix);

/**
 * Generate unique symbol name
 */
char *elf_generate_unique_symbol_name(ELFWriter *writer, const char *prefix);

/**
 * Check if symbol name is reserved
 */
bool elf_is_reserved_symbol_name(const char *name);

/**
 * Check if section name is reserved
 */
bool elf_is_reserved_section_name(const char *name);

/**
 * Sanitize symbol name for ELF compatibility
 */
char *elf_sanitize_symbol_name(const char *name);

/**
 * Sanitize section name for ELF compatibility
 */
char *elf_sanitize_section_name(const char *name);

/**
 * Calculate checksum for section data
 */
uint32_t elf_calculate_section_checksum(ELFSection *section);

/**
 * Calculate checksum for symbol table
 */
uint32_t elf_calculate_symbol_table_checksum(ELFWriter *writer);

/**
 * Calculate checksum for entire ELF file
 */
uint32_t elf_calculate_file_checksum(ELFWriter *writer);

/**
 * Create backup of ELF writer state
 */
ELFWriter *elf_create_backup(ELFWriter *writer);

/**
 * Restore ELF writer from backup
 */
bool elf_restore_from_backup(ELFWriter *writer, ELFWriter *backup);

/**
 * Clone ELF writer (deep copy)
 */
ELFWriter *elf_clone_writer(ELFWriter *writer);

/**
 * Merge two ELF writers
 */
ELFWriter *elf_merge_writers(ELFWriter *writer1, ELFWriter *writer2);

/**
 * Clear all data from ELF writer (reset to initial state)
 */
void elf_clear_writer(ELFWriter *writer);

/**
 * Resize ELF writer capacity
 */
bool elf_resize_writer_capacity(ELFWriter *writer, size_t new_sections_capacity,
                               size_t new_symbols_capacity, size_t new_relocations_capacity);

/**
 * Compact ELF writer memory usage
 */
bool elf_compact_writer_memory(ELFWriter *writer);

/**
 * Export ELF writer state to JSON
 */
char *elf_export_to_json(ELFWriter *writer);

/**
 * Import ELF writer state from JSON
 */
ELFWriter *elf_import_from_json(const char *json_data);

/**
 * Export symbol table to CSV
 */
bool elf_export_symbol_table_csv(ELFWriter *writer, const char *filename);

/**
 * Export section table to CSV
 */
bool elf_export_section_table_csv(ELFWriter *writer, const char *filename);

/**
 * Export relocation table to CSV
 */
bool elf_export_relocation_table_csv(ELFWriter *writer, const char *filename);

/**
 * Generate comprehensive report
 */
bool elf_generate_comprehensive_report(ELFWriter *writer, const char *output_dir);

/**
 * Benchmark ELF writer performance
 */
void elf_benchmark_writer_performance(ELFWriter *writer);

/**
 * Profile memory usage
 */
void elf_profile_memory_usage(ELFWriter *writer);

/**
 * Run stress test on ELF writer
 */
bool elf_run_stress_test(ELFWriter *writer, size_t iterations);

/**
 * Convert endianness if needed
 */
void elf_convert_endianness_if_needed(void *data, size_t size, bool is_little_endian);

/**
 * Check system endianness
 */
bool elf_is_system_little_endian(void);

/**
 * Get platform-specific ELF constants
 */
void elf_get_platform_constants(uint16_t *machine, uint8_t *osabi, uint8_t *data_encoding);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_ELF_WRITER_UTILS_H 
