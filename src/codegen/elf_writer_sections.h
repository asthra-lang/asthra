#ifndef ELF_WRITER_SECTIONS_H
#define ELF_WRITER_SECTIONS_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "elf_writer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// Section creation and management
ELFSection *elf_add_section(ELFWriter *writer, const char *name, 
                           uint32_t type, uint64_t flags);
bool elf_create_standard_sections(ELFWriter *writer);
bool elf_create_asthra_sections(ELFWriter *writer);

// Section data manipulation
bool elf_append_section_data(ELFSection *section, const void *data, size_t size);
bool elf_set_section_data(ELFSection *section, const void *data, size_t size);

// Section population functions
bool elf_populate_text_section(ELFWriter *writer);
bool elf_populate_ffi_section(ELFWriter *writer);
bool elf_populate_gc_section(ELFWriter *writer);
bool elf_populate_security_section(ELFWriter *writer);
bool elf_populate_pattern_matching_section(ELFWriter *writer);
bool elf_populate_string_ops_section(ELFWriter *writer);
bool elf_populate_slice_meta_section(ELFWriter *writer);
bool elf_populate_concurrency_section(ELFWriter *writer);

#ifdef __cplusplus
}
#endif

#endif // ELF_WRITER_SECTIONS_H
