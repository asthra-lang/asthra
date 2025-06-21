/**
 * Asthra Memory Manager - Diagnostics Implementation
 * Statistics, Debugging, and Reporting Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>

#include "memory_manager.h"

// =============================================================================
// DIAGNOSTICS AND DEBUGGING
// =============================================================================

MemoryZoneStats memory_manager_get_zone_stats(const MemoryManager *manager, 
                                              AsthraMemoryZone zone) {
    if (!manager || zone >= 4) {
        MemoryZoneStats empty = {0};
        return empty;
    }
    
    return manager->zone_stats[zone];
}

void memory_manager_print_summary(const MemoryManager *manager) {
    if (!manager) return;
    
    printf("Memory Manager Summary:\n");
    printf("  Strict mode: %s\n", manager->strict_mode ? "enabled" : "disabled");
    printf("  Debug mode: %s\n", manager->debug_mode ? "enabled" : "disabled");
    
    const char *zone_names[] = {"GC", "Manual", "Pinned", "Stack"};
    
    for (int i = 0; i < 4; i++) {
        MemoryZoneStats stats = manager->zone_stats[i];
        printf("  Zone %s:\n", zone_names[i]);
        printf("    Total allocations: %llu\n", 
               (unsigned long long)atomic_load(&stats.total_allocations));
        printf("    Total deallocations: %llu\n", 
               (unsigned long long)atomic_load(&stats.total_deallocations));
        printf("    Current bytes: %zu\n", 
               atomic_load(&stats.current_bytes));
        printf("    Peak bytes: %zu\n", 
               atomic_load(&stats.peak_bytes));
        printf("    Active allocations: %u\n", 
               atomic_load(&stats.active_allocations));
        printf("    Ownership violations: %u\n", 
               atomic_load(&stats.ownership_violations));
    }
}

void memory_manager_dump_allocations(const MemoryManager *manager, 
                                    AsthraMemoryZone zone) {
    if (!manager || zone >= 4) return;
    
    printf("Active allocations in zone %d:\n", zone);
    
    MemoryAllocation *current = manager->allocations[zone];
    int count = 0;
    
    while (current) {
        printf("  [%d] ID: %llu, Ptr: %p, Size: %zu, Type: %s\n",
               count++,
               (unsigned long long)current->allocation_id,
               current->ptr,
               current->size,
               current->type_name ? current->type_name : "unknown");
        current = current->next;
    }
    
    if (count == 0) {
        printf("  No active allocations\n");
    }
}

bool memory_manager_validate_all(const MemoryManager *manager) {
    if (!manager) return false;
    
    bool all_valid = true;
    
    for (int zone = 0; zone < 4; zone++) {
        MemoryAllocation *current = manager->allocations[zone];
        while (current) {
            // Basic validation - check that pointer is not NULL
            if (!current->ptr) {
                printf("[VALIDATION] Error: NULL pointer in allocation %llu\n",
                       (unsigned long long)current->allocation_id);
                all_valid = false;
            }
            
            // Check ownership consistency
            if (current->ownership.memory_zone != (AsthraMemoryZone)zone) {
                printf("[VALIDATION] Error: Zone mismatch for allocation %llu\n",
                       (unsigned long long)current->allocation_id);
                all_valid = false;
            }
            
            current = current->next;
        }
    }
    
    return all_valid;
} 
