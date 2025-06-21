/**
 * Asthra Programming Language Memory Management
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Basic memory management types and operations.
 */

#ifndef ASTHRA_MEMORY_H
#define ASTHRA_MEMORY_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

// Include the actual runtime core definitions
#include "core/asthra_runtime_core.h"

// Additional zone types for internal use
#define ASTHRA_MEMORY_ZONE_TEMP ASTHRA_ZONE_STACK  // Temporary allocations
#define ASTHRA_MEMORY_ZONE_PERM ASTHRA_ZONE_PINNED // Permanent allocations
#define ASTHRA_MEMORY_ZONE_COMPILER ASTHRA_ZONE_MANUAL // Compiler allocations
#define ASTHRA_MEMORY_ZONE_RUNTIME ASTHRA_ZONE_GC   // Runtime allocations
#define ASTHRA_MEMORY_ZONE_COUNT 4  // Total number of zones

// Basic memory allocation functions - aligned with asthra_runtime.h
void* asthra_malloc(size_t size);
void* asthra_calloc(size_t count, size_t size);

// Use unified signature with memory zone (compatible with asthra_runtime.h)
void* asthra_realloc(void *ptr, size_t new_size, AsthraMemoryZone zone);
void asthra_zone_free(void *ptr, AsthraMemoryZone zone);

// Zone-specific allocation functions
void* asthra_malloc_zone(size_t size, AsthraMemoryZone zone);
void* asthra_calloc_zone(size_t count, size_t size, AsthraMemoryZone zone);

// Zone management functions
void asthra_zone_reset(AsthraMemoryZone zone);
void asthra_zone_get_stats(AsthraMemoryZone zone, size_t *allocated, size_t *used);

// Convenience wrappers for tests that use old signature
static inline void* asthra_realloc_simple(void *ptr, size_t size) {
    return asthra_realloc(ptr, size, ASTHRA_ZONE_GC);
}

static inline void asthra_free_simple(void *ptr) {
    asthra_zone_free(ptr, ASTHRA_ZONE_GC);
}

// Memory safety functions
bool asthra_memory_is_valid(const void *ptr, size_t size);
void asthra_memory_set_bounds(void *ptr, size_t size);

#endif // ASTHRA_MEMORY_H 
