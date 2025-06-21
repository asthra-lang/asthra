#ifndef MACHO_FILE_WRITER_H
#define MACHO_FILE_WRITER_H

#include "macho_writer.h"
#include <stdio.h>
#include <stdbool.h>

// Layout calculation
bool macho_calculate_layout(MachoWriter *writer);

// Object file generation
bool macho_generate_object_file(MachoWriter *writer, ASTNode *program, 
                                const char *output_filename);
bool macho_write_object_file(MachoWriter *writer, const char *filename);

// Individual component writing
bool macho_write_load_commands(MachoWriter *writer, FILE *file);
bool macho_write_section_data(MachoWriter *writer, FILE *file);
bool macho_write_symbol_table(MachoWriter *writer, FILE *file);
bool macho_write_string_table(MachoWriter *writer, FILE *file);

#endif // MACHO_FILE_WRITER_H