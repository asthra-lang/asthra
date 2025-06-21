#ifndef MACHO_DATA_STRUCTURES_H
#define MACHO_DATA_STRUCTURES_H

#include "macho_writer.h"
#include <stdbool.h>
#include <stddef.h>

// Core data structure management
MachoWriter *macho_writer_create(FFIAssemblyGenerator *ffi_generator);
void macho_writer_destroy(MachoWriter *writer);

// String table management
uint32_t macho_add_string_to_table(char **table, size_t *size, 
                                   size_t *capacity, const char *str);

// Symbol management
MachoSymbol *macho_add_symbol(MachoWriter *writer, const char *name,
                             uint8_t type, uint8_t sect, uint64_t value);
MachoSymbol *macho_find_symbol_by_name(MachoWriter *writer, const char *name);

// Utility functions
void macho_get_statistics(MachoWriter *writer, size_t *sections, size_t *symbols,
                         size_t *relocations);
void macho_print_structure(MachoWriter *writer);

#endif // MACHO_DATA_STRUCTURES_H