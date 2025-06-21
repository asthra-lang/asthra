#ifndef MACHO_SECTIONS_H
#define MACHO_SECTIONS_H

#include "macho_writer.h"
#include <stdbool.h>
#include <stddef.h>

// Section creation and management
MachoSection *macho_add_section(MachoWriter *writer, const char *name,
                               const char *segment_name, uint32_t flags);
MachoSection *macho_find_section_by_name(MachoWriter *writer, const char *name);

// Section data manipulation
bool macho_set_section_data(MachoSection *section, const void *data, size_t size);
bool macho_append_section_data(MachoSection *section, const void *data, size_t size);

// Standard section creation
bool macho_create_standard_sections(MachoWriter *writer);
bool macho_populate_text_section(MachoWriter *writer);

#endif // MACHO_SECTIONS_H