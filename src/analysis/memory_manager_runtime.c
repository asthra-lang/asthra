/**
 * Asthra Memory Manager - Runtime Integration Implementation
 * Runtime Memory Zone Management and Integration
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>

#include "memory_manager.h"
#include "../../runtime/asthra_runtime.h"

// =============================================================================
// RUNTIME INTEGRATION
// =============================================================================

bool runtime_init_memory_zones(void) {
    // Initialize the runtime memory zones
    return asthra_runtime_init(NULL) == 0;
}

void *runtime_zone_alloc(AsthraMemoryZone zone, size_t size, const char *type_name) {
    void *ptr = asthra_alloc(size, zone);
    
    if (ptr && g_memory_manager.debug_mode) {
        printf("[RUNTIME] Zone allocation: %zu bytes in zone %d (type: %s)\n",
               size, zone, type_name ? type_name : "unknown");
    }
    
    return ptr;
}

void runtime_zone_free(AsthraMemoryZone zone, void *ptr) {
    if (g_memory_manager.debug_mode) {
        printf("[RUNTIME] Zone deallocation: %p from zone %d\n", ptr, zone);
    }
    
    asthra_free(ptr, zone);
}

bool runtime_pin_memory(void *ptr, size_t size) {
    asthra_gc_pin_memory(ptr, size);
    return true;
}

void runtime_unpin_memory(void *ptr) {
    asthra_gc_unpin_memory(ptr);
}

void runtime_secure_zero(void *ptr, size_t size) {
    asthra_secure_zero(ptr, size);
} 
