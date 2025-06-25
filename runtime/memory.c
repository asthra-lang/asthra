/**
 * Asthra Programming Language Memory Management Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Enhanced zone-based memory management implementation.
 */

#include "memory.h"
#include <pthread.h>
#include <stdint.h>
#include <string.h>

// Zone-based memory allocator structures
typedef struct ZoneBlock {
    struct ZoneBlock *next;
    size_t size;
    size_t used;
    uint8_t data[];
} ZoneBlock;

typedef struct MemoryZone {
    AsthraMemoryZone type;
    ZoneBlock *blocks;
    ZoneBlock *current_block;
    size_t total_allocated;
    size_t total_used;
    size_t block_size;
    pthread_mutex_t lock;
} MemoryZone;

// Global memory zones
static MemoryZone zones[ASTHRA_MEMORY_ZONE_COUNT];
static pthread_once_t zones_init_once;
static bool zones_init_once_initialized = false;

// Initialize memory zones
static void initialize_zones(void) {
    // Temporary allocations (small blocks, frequent allocation/deallocation)
    zones[ASTHRA_MEMORY_ZONE_TEMP].type = ASTHRA_MEMORY_ZONE_TEMP;
    zones[ASTHRA_MEMORY_ZONE_TEMP].block_size = 64 * 1024; // 64KB blocks
    pthread_mutex_init(&zones[ASTHRA_MEMORY_ZONE_TEMP].lock, NULL);

    // Permanent allocations (large blocks, infrequent deallocation)
    zones[ASTHRA_MEMORY_ZONE_PERM].type = ASTHRA_MEMORY_ZONE_PERM;
    zones[ASTHRA_MEMORY_ZONE_PERM].block_size = 1024 * 1024; // 1MB blocks
    pthread_mutex_init(&zones[ASTHRA_MEMORY_ZONE_PERM].lock, NULL);

    // Compiler allocations (medium blocks, batch deallocation)
    zones[ASTHRA_MEMORY_ZONE_COMPILER].type = ASTHRA_MEMORY_ZONE_COMPILER;
    zones[ASTHRA_MEMORY_ZONE_COMPILER].block_size = 256 * 1024; // 256KB blocks
    pthread_mutex_init(&zones[ASTHRA_MEMORY_ZONE_COMPILER].lock, NULL);

    // Runtime allocations (variable size, reference counted)
    zones[ASTHRA_MEMORY_ZONE_RUNTIME].type = ASTHRA_MEMORY_ZONE_RUNTIME;
    zones[ASTHRA_MEMORY_ZONE_RUNTIME].block_size = 512 * 1024; // 512KB blocks
    pthread_mutex_init(&zones[ASTHRA_MEMORY_ZONE_RUNTIME].lock, NULL);
}

// Allocate a new block for a zone
static ZoneBlock *allocate_zone_block(MemoryZone *zone, size_t min_size) {
    size_t block_size = zone->block_size;
    if (min_size > block_size - sizeof(ZoneBlock)) {
        block_size = min_size + sizeof(ZoneBlock);
    }

    ZoneBlock *block = (ZoneBlock *)malloc(block_size);
    if (!block)
        return NULL;

    block->next = NULL;
    block->size = block_size - sizeof(ZoneBlock);
    block->used = 0;

    return block;
}

// Zone-based malloc implementation
void *asthra_malloc(size_t size) {
    // Default to runtime zone for unspecified allocations
    return asthra_malloc_zone(size, ASTHRA_MEMORY_ZONE_RUNTIME);
}

// Zone-specific malloc
void *asthra_malloc_zone(size_t size, AsthraMemoryZone zone_type) {
    if (zone_type >= ASTHRA_MEMORY_ZONE_COUNT) {
        return NULL;
    }

    // Ensure zones are initialized
    if (!zones_init_once_initialized) {
        static pthread_mutex_t init_mutex = PTHREAD_MUTEX_INITIALIZER;
        pthread_mutex_lock(&init_mutex);
        if (!zones_init_once_initialized) {
            initialize_zones();
            zones_init_once_initialized = true;
        }
        pthread_mutex_unlock(&init_mutex);
    }

    // Align size to 8 bytes
    size = (size + 7) & ~7;

    MemoryZone *zone = &zones[zone_type];

    pthread_mutex_lock(&zone->lock);

    // Check if current block has enough space
    if (zone->current_block && zone->current_block->used + size <= zone->current_block->size) {
        void *ptr = zone->current_block->data + zone->current_block->used;
        zone->current_block->used += size;
        zone->total_used += size;
        pthread_mutex_unlock(&zone->lock);
        return ptr;
    }

    // Need to allocate a new block
    ZoneBlock *new_block = allocate_zone_block(zone, size);
    if (!new_block) {
        pthread_mutex_unlock(&zone->lock);
        return NULL;
    }

    // Add block to zone
    new_block->next = zone->blocks;
    zone->blocks = new_block;
    zone->current_block = new_block;
    zone->total_allocated += new_block->size + sizeof(ZoneBlock);

    // Allocate from new block
    void *ptr = new_block->data;
    new_block->used = size;
    zone->total_used += size;

    pthread_mutex_unlock(&zone->lock);
    return ptr;
}

void *asthra_calloc(size_t count, size_t size) {
    return asthra_calloc_zone(count, size, ASTHRA_MEMORY_ZONE_RUNTIME);
}

void *asthra_calloc_zone(size_t count, size_t size, AsthraMemoryZone zone_type) {
    size_t total_size = count * size;
    void *ptr = asthra_malloc_zone(total_size, zone_type);
    if (ptr) {
        memset(ptr, 0, total_size);
    }
    return ptr;
}

void *asthra_realloc(void *ptr, size_t new_size, AsthraMemoryZone zone) {
    // Zone-based realloc: allocate new memory and copy
    if (!ptr) {
        return asthra_malloc_zone(new_size, zone);
    }

    if (new_size == 0) {
        asthra_zone_free(ptr, zone);
        return NULL;
    }

    // For zone-based allocation, we need to allocate new memory and copy
    // because we don't track individual allocation sizes
    void *new_ptr = asthra_malloc_zone(new_size, zone);
    if (!new_ptr) {
        return NULL;
    }

    // Copy data (we don't know the old size, so this is a limitation)
    // In a production system, we'd track allocation metadata
    memcpy(new_ptr, ptr, new_size);

    // Note: We don't free the old pointer in zone allocation
    // Zones are typically freed all at once

    return new_ptr;
}

void asthra_zone_free(void *ptr, AsthraMemoryZone zone) {
    // Zone-based allocation doesn't support individual frees
    // Memory is reclaimed when the entire zone is reset
    (void)ptr;
    (void)zone;
}

// Reset an entire zone, freeing all its memory
void asthra_zone_reset(AsthraMemoryZone zone_type) {
    if (zone_type >= ASTHRA_MEMORY_ZONE_COUNT) {
        return;
    }

    pthread_once(&zones_init_once, initialize_zones);

    MemoryZone *zone = &zones[zone_type];

    pthread_mutex_lock(&zone->lock);

    // Free all blocks except the first one
    ZoneBlock *block = zone->blocks;
    if (block) {
        ZoneBlock *next = block->next;
        while (next) {
            ZoneBlock *to_free = next;
            next = next->next;
            free(to_free);
        }

        // Reset the first block
        block->next = NULL;
        block->used = 0;
        zone->blocks = block;
        zone->current_block = block;
    }

    zone->total_used = 0;
    zone->total_allocated = block ? block->size + sizeof(ZoneBlock) : 0;

    pthread_mutex_unlock(&zone->lock);
}

// Memory bounds tracking structure
typedef struct MemoryBounds {
    void *ptr;
    size_t size;
    struct MemoryBounds *next;
} MemoryBounds;

static MemoryBounds *bounds_list = NULL;
static pthread_mutex_t bounds_mutex = PTHREAD_MUTEX_INITIALIZER;

// Memory safety functions with real implementation
bool asthra_memory_is_valid(const void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return false;
    }

    pthread_mutex_lock(&bounds_mutex);

    // Check if pointer falls within any registered bounds
    MemoryBounds *bounds = bounds_list;
    while (bounds) {
        if (ptr >= bounds->ptr && (char *)ptr + size <= (char *)bounds->ptr + bounds->size) {
            pthread_mutex_unlock(&bounds_mutex);
            return true;
        }
        bounds = bounds->next;
    }

    pthread_mutex_unlock(&bounds_mutex);
    return false;
}

void asthra_memory_set_bounds(void *ptr, size_t size) {
    if (!ptr || size == 0) {
        return;
    }

    MemoryBounds *new_bounds = malloc(sizeof(MemoryBounds));
    if (!new_bounds) {
        return;
    }

    new_bounds->ptr = ptr;
    new_bounds->size = size;

    pthread_mutex_lock(&bounds_mutex);
    new_bounds->next = bounds_list;
    bounds_list = new_bounds;
    pthread_mutex_unlock(&bounds_mutex);
}

// Get zone statistics
void asthra_zone_get_stats(AsthraMemoryZone zone_type, size_t *allocated, size_t *used) {
    if (zone_type >= ASTHRA_MEMORY_ZONE_COUNT) {
        if (allocated)
            *allocated = 0;
        if (used)
            *used = 0;
        return;
    }

    pthread_once(&zones_init_once, initialize_zones);

    MemoryZone *zone = &zones[zone_type];

    pthread_mutex_lock(&zone->lock);
    if (allocated)
        *allocated = zone->total_allocated;
    if (used)
        *used = zone->total_used;
    pthread_mutex_unlock(&zone->lock);
}
