#include "../../runtime/core/asthra_runtime_core.h"
#include "../../runtime/memory/asthra_runtime_memory.h"
#include <assert.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declarations
void asthra_set_error(int code, const char *message);

// Enhanced runtime stub implementations with zone-based allocation and memory safety

// Memory zone tracking structure
typedef struct MemoryZoneInfo {
    asthra_atomic_size_t allocated_bytes;
    asthra_atomic_size_t peak_usage;
    asthra_atomic_counter_t allocation_count;
    asthra_atomic_counter_t deallocation_count;
    void **allocations; // Track allocations for leak detection
    size_t *allocation_sizes;
    size_t max_tracked_allocations;
    size_t current_tracked_count;
    asthra_atomic_flag zone_lock;
} MemoryZoneInfo;

// Memory zones for different allocation patterns
static MemoryZoneInfo memory_zones[4] = {[ASTHRA_ZONE_GC] = {0},
                                         [ASTHRA_ZONE_MANUAL] = {0},
                                         [ASTHRA_ZONE_PINNED] = {0},
                                         [ASTHRA_ZONE_STACK] = {0}};

// Reference counting support
typedef struct RefCountedObject {
    asthra_atomic_counter_t ref_count;
    size_t size;
    AsthraMemoryZone zone;
    void (*destructor)(void *);
    char data[];
} RefCountedObject;

// Global runtime statistics
static AsthraAtomicMemoryStats global_stats = {0};

// Thread-local error state
static _Thread_local int last_error_code = 0;
static _Thread_local char last_error_message[256] = {0};

// Initialize memory zones (call once at startup)
static void init_memory_zones(void) {
    static atomic_flag initialized = ATOMIC_FLAG_INIT;

    if (atomic_flag_test_and_set(&initialized)) {
        return; // Already initialized
    }

    for (int i = 0; i < 4; i++) {
        memory_zones[i].max_tracked_allocations = 1000;
        memory_zones[i].allocations = calloc(1000, sizeof(void *));
        memory_zones[i].allocation_sizes = calloc(1000, sizeof(size_t));
        memory_zones[i].current_tracked_count = 0;
        atomic_flag_clear(&memory_zones[i].zone_lock);
    }
}

// Track allocation in zone
static void track_allocation(AsthraMemoryZone zone, void *ptr, size_t size) {
    if (zone >= 4)
        return;

    MemoryZoneInfo *zone_info = &memory_zones[zone];

    // Acquire zone lock
    while (atomic_flag_test_and_set(&zone_info->zone_lock)) {
        // Spin wait
    }

    // Update statistics
    atomic_fetch_add(&zone_info->allocated_bytes, size);
    atomic_fetch_add(&zone_info->allocation_count, 1);

    size_t current = atomic_load(&zone_info->allocated_bytes);
    size_t peak = atomic_load(&zone_info->peak_usage);
    if (current > peak) {
        atomic_store(&zone_info->peak_usage, current);
    }

    // Track allocation for leak detection
    if (zone_info->current_tracked_count < zone_info->max_tracked_allocations) {
        zone_info->allocations[zone_info->current_tracked_count] = ptr;
        zone_info->allocation_sizes[zone_info->current_tracked_count] = size;
        zone_info->current_tracked_count++;
    }

    atomic_flag_clear(&zone_info->zone_lock);

    // Update global stats
    atomic_fetch_add(&global_stats.total_allocations, 1);
    atomic_fetch_add(&global_stats.current_memory_usage, size);

    size_t global_current = atomic_load(&global_stats.current_memory_usage);
    size_t global_peak = atomic_load(&global_stats.peak_memory_usage);
    if (global_current > global_peak) {
        atomic_store(&global_stats.peak_memory_usage, global_current);
    }
}

// Track deallocation in zone
static void track_deallocation(AsthraMemoryZone zone, void *ptr, size_t size) {
    if (zone >= 4)
        return;

    MemoryZoneInfo *zone_info = &memory_zones[zone];

    // Acquire zone lock
    while (atomic_flag_test_and_set(&zone_info->zone_lock)) {
        // Spin wait
    }

    // Update statistics
    atomic_fetch_sub(&zone_info->allocated_bytes, size);
    atomic_fetch_add(&zone_info->deallocation_count, 1);

    // Remove from tracking
    for (size_t i = 0; i < zone_info->current_tracked_count; i++) {
        if (zone_info->allocations[i] == ptr) {
            // Shift remaining allocations
            for (size_t j = i; j < zone_info->current_tracked_count - 1; j++) {
                zone_info->allocations[j] = zone_info->allocations[j + 1];
                zone_info->allocation_sizes[j] = zone_info->allocation_sizes[j + 1];
            }
            zone_info->current_tracked_count--;
            break;
        }
    }

    atomic_flag_clear(&zone_info->zone_lock);

    // Update global stats
    atomic_fetch_add(&global_stats.total_deallocations, 1);
    atomic_fetch_sub(&global_stats.current_memory_usage, size);
}

// Enhanced allocation with zone-based management
void *asthra_alloc(size_t size, AsthraMemoryZone zone) {
    init_memory_zones();

    if (size == 0) {
        asthra_set_error(1, "Cannot allocate zero bytes");
        return NULL;
    }

    void *ptr = NULL;

    switch (zone) {
    case ASTHRA_ZONE_GC:
        // GC zone: use standard malloc but track for GC simulation
        ptr = malloc(size);
        break;

    case ASTHRA_ZONE_MANUAL:
        // Manual zone: standard malloc with manual management
        ptr = malloc(size);
        break;

    case ASTHRA_ZONE_PINNED:
// Pinned zone: aligned allocation for better performance
#ifdef _WIN32
        ptr = _aligned_malloc(size, 64); // 64-byte alignment
#else
        if (posix_memalign(&ptr, 64, size) != 0) {
            ptr = NULL;
        }
#endif
        break;

    case ASTHRA_ZONE_STACK:
        // Stack zone: simulate with malloc but track separately
        ptr = malloc(size);
        break;

    default:
        asthra_set_error(2, "Invalid memory zone specified");
        return NULL;
    }

    if (ptr) {
        track_allocation(zone, ptr, size);

        // Zero-initialize for safety
        memset(ptr, 0, size);

        printf("[MEMORY] Allocated %zu bytes in zone %d at %p\n", size, zone, ptr);
    } else {
        asthra_set_error(3, "Memory allocation failed");
    }

    return ptr;
}

// Enhanced deallocation with zone tracking
void asthra_free(void *ptr, AsthraMemoryZone zone) {
    if (!ptr)
        return;

    init_memory_zones();

    // Find the allocation size for tracking
    size_t allocation_size = 0;
    MemoryZoneInfo *zone_info = &memory_zones[zone];

    // Acquire zone lock to find size
    while (atomic_flag_test_and_set(&zone_info->zone_lock)) {
        // Spin wait
    }

    for (size_t i = 0; i < zone_info->current_tracked_count; i++) {
        if (zone_info->allocations[i] == ptr) {
            allocation_size = zone_info->allocation_sizes[i];
            break;
        }
    }

    atomic_flag_clear(&zone_info->zone_lock);

    if (allocation_size > 0) {
        track_deallocation(zone, ptr, allocation_size);
        printf("[MEMORY] Freed %zu bytes from zone %d at %p\n", allocation_size, zone, ptr);
    }

    // Zero memory before freeing for security
    if (allocation_size > 0) {
        memset(ptr, 0, allocation_size);
    }

    switch (zone) {
    case ASTHRA_ZONE_GC:
    case ASTHRA_ZONE_MANUAL:
    case ASTHRA_ZONE_STACK:
        free(ptr);
        break;

    case ASTHRA_ZONE_PINNED:
#ifdef _WIN32
        _aligned_free(ptr);
#else
        free(ptr);
#endif
        break;

    default:
        asthra_set_error(4, "Invalid memory zone for deallocation");
        break;
    }
}

// Enhanced error handling with thread-local storage
void asthra_set_error(int code, const char *message) {
    last_error_code = code;
    if (message) {
        strncpy(last_error_message, message, sizeof(last_error_message) - 1);
        last_error_message[sizeof(last_error_message) - 1] = '\0';
    }

    // For testing, also print errors
    printf("[ERROR %d] %s\n", code, message ? message : "Unknown error");
}

// Additional runtime functions for enhanced functionality

// Get last error (thread-local)
int asthra_get_last_error(void) {
    return last_error_code;
}

// Get last error message (thread-local)
const char *asthra_get_last_error_message(void) {
    return last_error_message;
}

// Clear error state
void asthra_clear_error(void) {
    last_error_code = 0;
    last_error_message[0] = '\0';
}

// Get memory zone statistics
AsthraRuntimeStats asthra_get_runtime_stats(void) {
    AsthraRuntimeStats stats;
    stats.total_allocations = atomic_load(&global_stats.total_allocations);
    stats.total_deallocations = atomic_load(&global_stats.total_deallocations);
    stats.current_memory_usage = atomic_load(&global_stats.current_memory_usage);
    stats.peak_memory_usage = atomic_load(&global_stats.peak_memory_usage);
    stats.gc_collections = atomic_load(&global_stats.gc_collections);
    stats.gc_time_ms = atomic_load(&global_stats.gc_time_ms);
    stats.tasks_spawned = atomic_load(&global_stats.tasks_spawned);
    stats.tasks_completed = atomic_load(&global_stats.tasks_completed);
    stats.ffi_calls = atomic_load(&global_stats.ffi_calls);

    return stats;
}

// Reset runtime statistics
void asthra_reset_runtime_stats(void) {
    atomic_store(&global_stats.total_allocations, 0);
    atomic_store(&global_stats.total_deallocations, 0);
    atomic_store(&global_stats.current_memory_usage, 0);
    atomic_store(&global_stats.peak_memory_usage, 0);
    atomic_store(&global_stats.gc_collections, 0);
    atomic_store(&global_stats.gc_time_ms, 0);
    atomic_store(&global_stats.tasks_spawned, 0);
    atomic_store(&global_stats.tasks_completed, 0);
    atomic_store(&global_stats.ffi_calls, 0);
}

// Memory leak detection
void asthra_check_memory_leaks(void) {
    init_memory_zones();

    printf("\n=== Memory Leak Detection Report ===\n");

    bool leaks_found = false;
    for (int zone = 0; zone < 4; zone++) {
        MemoryZoneInfo *zone_info = &memory_zones[zone];

        if (zone_info->current_tracked_count > 0) {
            printf("Zone %d: %zu leaked allocations\n", zone, zone_info->current_tracked_count);

            for (size_t i = 0; i < zone_info->current_tracked_count; i++) {
                printf("  Leak: %p (%zu bytes)\n", zone_info->allocations[i],
                       zone_info->allocation_sizes[i]);
            }
            leaks_found = true;
        }
    }

    if (!leaks_found) {
        printf("No memory leaks detected.\n");
    }

    printf("=====================================\n\n");
}

// Cleanup memory zones (call at program exit)
void asthra_cleanup_memory_zones(void) {
    for (int i = 0; i < 4; i++) {
        if (memory_zones[i].allocations) {
            free(memory_zones[i].allocations);
            memory_zones[i].allocations = NULL;
        }
        if (memory_zones[i].allocation_sizes) {
            free(memory_zones[i].allocation_sizes);
            memory_zones[i].allocation_sizes = NULL;
        }
    }
}
