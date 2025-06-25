#ifndef ASTHRA_MEMORY_H
#define ASTHRA_MEMORY_H

#include "core/asthra_runtime_core.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int asthra_memory_zones_init(void);
void asthra_memory_zones_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_MEMORY_H
