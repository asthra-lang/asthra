/**
 * Asthra Memory Manager - Core Implementation
 * Basic Memory Management and Allocation Tracking
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "memory_manager.h"
#include "../../runtime/asthra_runtime.h"

// =============================================================================
// GLOBAL MEMORY MANAGER INSTANCE
// =============================================================================

MemoryManager g_memory_manager = {0};

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

static uint64_t generate_allocation_id(MemoryManager *manager) {
    return atomic_fetch_add(&manager->next_allocation_id, 1);
}

static MemoryAllocation *find_allocation(MemoryManager *manager, void *ptr) {
    for (int zone = 0; zone < 4; zone++) {
        MemoryAllocation *current = manager->allocations[zone];
        while (current) {
            if (current->ptr == ptr) {
                return current;
            }
            current = current->next;
        }
    }
    return NULL;
}

static void add_allocation(MemoryManager *manager, MemoryAllocation *allocation) {
    AsthraMemoryZone zone = allocation->ownership.memory_zone;
    allocation->next = manager->allocations[zone];
    manager->allocations[zone] = allocation;
    
    // Update statistics
    atomic_fetch_add(&manager->zone_stats[zone].total_allocations, 1);
    atomic_fetch_add(&manager->zone_stats[zone].active_allocations, 1);
    atomic_fetch_add(&manager->zone_stats[zone].current_bytes, allocation->size);
    
    // Update peak bytes if necessary
    size_t current_bytes = atomic_load(&manager->zone_stats[zone].current_bytes);
    size_t peak_bytes = atomic_load(&manager->zone_stats[zone].peak_bytes);
    while (current_bytes > peak_bytes) {
        if (atomic_compare_exchange_weak(&manager->zone_stats[zone].peak_bytes, 
                                        &peak_bytes, current_bytes)) {
            break;
        }
    }
}

static bool remove_allocation(MemoryManager *manager, void *ptr) {
    for (int zone = 0; zone < 4; zone++) {
        MemoryAllocation **current = &manager->allocations[zone];
        while (*current) {
            if ((*current)->ptr == ptr) {
                MemoryAllocation *to_remove = *current;
                *current = (*current)->next;
                
                // Update statistics
                atomic_fetch_add(&manager->zone_stats[zone].total_deallocations, 1);
                atomic_fetch_sub(&manager->zone_stats[zone].active_allocations, 1);
                atomic_fetch_sub(&manager->zone_stats[zone].current_bytes, to_remove->size);
                
                free(to_remove);
                return true;
            }
            current = &(*current)->next;
        }
    }
    return false;
}

// =============================================================================
// MEMORY MANAGER LIFECYCLE
// =============================================================================

bool memory_manager_init(MemoryManager *manager, bool strict_mode, bool debug_mode) {
    if (!manager) return false;
    
    // Initialize structure
    memset(manager, 0, sizeof(MemoryManager));
    
    // Initialize mutex
    if (pthread_mutex_init(&manager->allocation_mutex, NULL) != 0) {
        return false;
    }
    
    // Initialize atomic counters
    atomic_init(&manager->next_allocation_id, 1);
    
    // Initialize zone statistics
    for (int i = 0; i < 4; i++) {
        atomic_init(&manager->zone_stats[i].total_allocations, 0);
        atomic_init(&manager->zone_stats[i].total_deallocations, 0);
        atomic_init(&manager->zone_stats[i].current_bytes, 0);
        atomic_init(&manager->zone_stats[i].peak_bytes, 0);
        atomic_init(&manager->zone_stats[i].active_allocations, 0);
        atomic_init(&manager->zone_stats[i].ownership_violations, 0);
    }
    
    manager->strict_mode = strict_mode;
    manager->debug_mode = debug_mode;
    manager->initialized = true;
    
    return true;
}

void memory_manager_cleanup(MemoryManager *manager) {
    if (!manager || !manager->initialized) return;
    
    pthread_mutex_lock(&manager->allocation_mutex);
    
    // Free all tracked allocations
    for (int zone = 0; zone < 4; zone++) {
        MemoryAllocation *current = manager->allocations[zone];
        while (current) {
            MemoryAllocation *next = current->next;
            
            if (manager->debug_mode) {
                printf("[MEMORY] Cleanup: freeing allocation %llu at %p\n", 
                       (unsigned long long)current->allocation_id, current->ptr);
            }
            
            // Free the actual memory based on ownership
            if (current->ownership.ownership_type == OWNERSHIP_GC) {
                asthra_free(current->ptr, ASTHRA_ZONE_GC);
            } else if (current->ownership.ownership_type == OWNERSHIP_C) {
                free(current->ptr);
            } else if (current->ownership.ownership_type == OWNERSHIP_PINNED) {
                asthra_free(current->ptr, ASTHRA_ZONE_PINNED);
            }
            
            free(current);
            current = next;
        }
        manager->allocations[zone] = NULL;
    }
    
    pthread_mutex_unlock(&manager->allocation_mutex);
    pthread_mutex_destroy(&manager->allocation_mutex);
    
    manager->initialized = false;
}

// =============================================================================
// MEMORY ALLOCATION INTERFACE
// =============================================================================

void *memory_manager_alloc(MemoryManager *manager, 
                          size_t size, 
                          const OwnershipContext *ownership,
                          const char *type_name) {
    if (!manager || !manager->initialized || !ownership || size == 0) {
        return NULL;
    }
    
    pthread_mutex_lock(&manager->allocation_mutex);
    
    // Allocate memory based on ownership type
    void *ptr = NULL;
    switch (ownership->ownership_type) {
        case OWNERSHIP_GC:
            ptr = asthra_alloc(size, ASTHRA_ZONE_GC);
            break;
        case OWNERSHIP_C:
            ptr = malloc(size);
            break;
        case OWNERSHIP_PINNED:
            ptr = asthra_alloc(size, ASTHRA_ZONE_PINNED);
            if (ptr) {
                asthra_gc_pin_memory(ptr, size);
            }
            break;
        default:
            pthread_mutex_unlock(&manager->allocation_mutex);
            return NULL;
    }
    
    if (!ptr) {
        pthread_mutex_unlock(&manager->allocation_mutex);
        return NULL;
    }
    
    // Create allocation tracking structure
    MemoryAllocation *allocation = malloc(sizeof(MemoryAllocation));
    if (!allocation) {
        // Free the allocated memory since we can't track it
        if (ownership->ownership_type == OWNERSHIP_GC) {
            asthra_free(ptr, ASTHRA_ZONE_GC);
        } else if (ownership->ownership_type == OWNERSHIP_C) {
            free(ptr);
        } else if (ownership->ownership_type == OWNERSHIP_PINNED) {
            asthra_gc_unpin_memory(ptr);
            asthra_free(ptr, ASTHRA_ZONE_PINNED);
        }
        pthread_mutex_unlock(&manager->allocation_mutex);
        return NULL;
    }
    
    // Initialize allocation metadata
    allocation->ptr = ptr;
    allocation->size = size;
    allocation->ownership = *ownership;
    allocation->allocation_id = generate_allocation_id(manager);
    allocation->type_name = type_name ? strdup(type_name) : NULL;
    allocation->next = NULL;
    
    // Add to tracking list
    add_allocation(manager, allocation);
    
    MEMORY_TRACK_ALLOCATION(manager, ptr, size, ownership, type_name);
    
    pthread_mutex_unlock(&manager->allocation_mutex);
    return ptr;
}

bool memory_manager_free(MemoryManager *manager, 
                        void *ptr, 
                        const OwnershipContext *ownership) {
    if (!manager || !manager->initialized || !ptr) {
        return false;
    }
    
    pthread_mutex_lock(&manager->allocation_mutex);
    
    // Find the allocation
    MemoryAllocation *allocation = find_allocation(manager, ptr);
    if (!allocation) {
        if (manager->strict_mode) {
            atomic_fetch_add(&manager->zone_stats[0].ownership_violations, 1);
            fprintf(stderr, "[MEMORY] Error: Attempting to free untracked pointer %p\n", ptr);
        }
        pthread_mutex_unlock(&manager->allocation_mutex);
        return false;
    }
    
    // Validate ownership consistency
    if (ownership && allocation->ownership.ownership_type != ownership->ownership_type) {
        atomic_fetch_add(&manager->zone_stats[allocation->ownership.memory_zone].ownership_violations, 1);
        if (manager->strict_mode) {
            fprintf(stderr, "[MEMORY] Error: Ownership mismatch for pointer %p\n", ptr);
            pthread_mutex_unlock(&manager->allocation_mutex);
            return false;
        }
    }
    
    MEMORY_TRACK_DEALLOCATION(manager, ptr);
    
    // Free the memory based on ownership type
    switch (allocation->ownership.ownership_type) {
        case OWNERSHIP_GC:
            asthra_free(ptr, ASTHRA_ZONE_GC);
            break;
        case OWNERSHIP_C:
            free(ptr);
            break;
        case OWNERSHIP_PINNED:
            asthra_gc_unpin_memory(ptr);
            asthra_free(ptr, ASTHRA_ZONE_PINNED);
            break;
        case OWNERSHIP_COUNT:
            // Invalid ownership type - should not happen
            break;
    }
    
    // Remove from tracking
    bool removed = remove_allocation(manager, ptr);
    
    pthread_mutex_unlock(&manager->allocation_mutex);
    return removed;
}

bool memory_manager_register_external(MemoryManager *manager,
                                     void *ptr,
                                     size_t size,
                                     const OwnershipContext *ownership,
                                     const char *type_name) {
    if (!manager || !manager->initialized || !ptr || !ownership) {
        return false;
    }
    
    pthread_mutex_lock(&manager->allocation_mutex);
    
    // Check if already registered
    if (find_allocation(manager, ptr)) {
        pthread_mutex_unlock(&manager->allocation_mutex);
        return false;
    }
    
    // Create allocation tracking structure
    MemoryAllocation *allocation = malloc(sizeof(MemoryAllocation));
    if (!allocation) {
        pthread_mutex_unlock(&manager->allocation_mutex);
        return false;
    }
    
    allocation->ptr = ptr;
    allocation->size = size;
    allocation->ownership = *ownership;
    allocation->allocation_id = generate_allocation_id(manager);
    allocation->type_name = type_name ? strdup(type_name) : NULL;
    allocation->next = NULL;
    
    add_allocation(manager, allocation);
    
    if (manager->debug_mode) {
        printf("[MEMORY] Registered external allocation %llu at %p (size: %zu)\n",
               (unsigned long long)allocation->allocation_id, ptr, size);
    }
    
    pthread_mutex_unlock(&manager->allocation_mutex);
    return true;
}

bool memory_manager_unregister_external(MemoryManager *manager, void *ptr) {
    if (!manager || !manager->initialized || !ptr) {
        return false;
    }
    
    pthread_mutex_lock(&manager->allocation_mutex);
    
    bool removed = remove_allocation(manager, ptr);
    
    if (manager->debug_mode && removed) {
        printf("[MEMORY] Unregistered external allocation at %p\n", ptr);
    }
    
    pthread_mutex_unlock(&manager->allocation_mutex);
    return removed;
}

// =============================================================================
// GLOBAL MEMORY MANAGER FUNCTIONS
// =============================================================================

bool init_global_memory_manager(bool strict_mode, bool debug_mode) {
    return memory_manager_init(&g_memory_manager, strict_mode, debug_mode);
}

void cleanup_global_memory_manager(void) {
    memory_manager_cleanup(&g_memory_manager);
}

MemoryManager *get_global_memory_manager(void) {
    return g_memory_manager.initialized ? &g_memory_manager : NULL;
} 
