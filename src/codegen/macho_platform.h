#ifndef MACHO_PLATFORM_H
#define MACHO_PLATFORM_H

#include <stdint.h>

// Platform detection functions
uint32_t macho_get_cpu_type(void);
uint32_t macho_get_cpu_subtype(void);

#endif // MACHO_PLATFORM_H