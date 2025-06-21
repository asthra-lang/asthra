# Memory API

### Four-Zone Memory Model

#### Memory Zone Architecture

**Memory Zones** (`src/runtime/memory/asthra_memory_core.h`):
```c
typedef enum {
    MEMORY_ZONE_GC,        // Garbage collected memory
    MEMORY_ZONE_MANUAL,    // Manually managed memory
    MEMORY_ZONE_PINNED,    // Pinned memory (no GC movement)
    MEMORY_ZONE_STACK      // Stack-allocated memory
} MemoryZone;

typedef struct MemoryManager {
    GCHeap* gc_heap;               // Garbage collector
    ManualHeap* manual_heap;       // Manual memory manager
    PinnedHeap* pinned_heap;       // Pinned memory manager
    StackManager* stack_manager;   // Stack memory manager
    
    // Global state
    size_t total_allocated;        // Total memory allocated
    size_t gc_threshold;           // GC trigger threshold
    bool gc_enabled;               // GC enable/disable flag
    
    // Statistics
    MemoryStatistics stats;        // Memory usage statistics
    
    // Thread safety
    pthread_mutex_t memory_mutex;  // Memory allocation mutex
} MemoryManager;
```

#### Garbage Collection Zone

**Garbage Collector Implementation** (`src/runtime/memory/asthra_gc.c`):
```c
typedef struct GCHeap {
    GCObject** objects;            // Tracked objects
    size_t object_count;
    size_t object_capacity;
    
    // GC state
    GCPhase current_phase;         // Current GC phase
    size_t bytes_allocated;        // Bytes allocated since last GC
    size_t gc_threshold;           // Threshold for triggering GC
    
    // Root set
    GCRoot** roots;                // GC roots
    size_t root_count;
    
    // Mark and sweep state
    bool* mark_bits;               // Mark bits for objects
    GCObject** sweep_list;         // Objects to be swept
} GCHeap;

// Allocate GC memory
void* asthra_gc_alloc(size_t size) {
    GCObject* obj = gc_allocate_object(size);
    
    if (!obj) {
        // Trigger GC and retry
        asthra_gc_collect();
        obj = gc_allocate_object(size);
        
        if (!obj) {
            asthra_panic("Out of memory");
        }
    }
    
    return obj->data;
}

// Garbage collection cycle
void asthra_gc_collect(void) {
    GCHeap* heap = get_gc_heap();
    
    // Mark phase
    gc_mark_phase(heap);
    
    // Sweep phase
    gc_sweep_phase(heap);
    
    // Update statistics
    update_gc_statistics(heap);
}
```

#### Manual Memory Zone

**Manual Memory Management** (`src/runtime/memory/asthra_manual.c`):
```c
typedef struct ManualHeap {
    MemoryBlock* blocks;           // Allocated blocks
    size_t block_count;
    
    // Free list management
    FreeBlock* free_list;          // Free block list
    size_t total_size;             // Total heap size
    size_t used_size;              // Used memory size
    
    // Safety tracking
    AllocationInfo* allocations;   // Allocation tracking
    size_t allocation_count;
} ManualHeap;

// Manual allocation
void* asthra_manual_alloc(size_t size) {
    ManualHeap* heap = get_manual_heap();
    
    // Find suitable free block
    FreeBlock* block = find_free_block(heap, size);
    
    if (!block) {
        // Expand heap
        if (!expand_manual_heap(heap, size)) {
            return NULL;
        }
        block = find_free_block(heap, size);
    }
    
    // Allocate from block
    void* ptr = allocate_from_block(block, size);
    
    // Track allocation for safety
    track_allocation(heap, ptr, size);
    
    return ptr;
}

// Manual deallocation
void asthra_manual_free(void* ptr) {
    ManualHeap* heap = get_manual_heap();
    
    // Validate pointer
    if (!validate_manual_pointer(heap, ptr)) {
        asthra_panic("Invalid pointer in manual_free");
    }
    
    // Find allocation info
    AllocationInfo* info = find_allocation_info(heap, ptr);
    
    // Add to free list
    add_to_free_list(heap, ptr, info->size);
    
    // Remove from tracking
    remove_allocation_tracking(heap, ptr);
}
```

#### Pinned Memory Zone

**Pinned Memory Management** (`src/runtime/memory/asthra_pinned.c`):
```c
typedef struct PinnedHeap {
    PinnedBlock* blocks;           // Pinned memory blocks
    size_t block_count;
    
    // Address stability
    void** stable_addresses;       // Stable address table
    size_t address_count;
    
    // FFI integration
    FFIMapping* ffi_mappings;      // FFI pointer mappings
    size_t mapping_count;
} PinnedHeap;

// Allocate pinned memory
void* asthra_pinned_alloc(size_t size) {
    PinnedHeap* heap = get_pinned_heap();
    
    // Allocate pinned block
    PinnedBlock* block = allocate_pinned_block(heap, size);
    
    if (!block) {
        return NULL;
    }
    
    // Register stable address
    register_stable_address(heap, block->data);
    
    return block->data;
}

// Pin existing memory
bool asthra_pin_memory(void* ptr, size_t size) {
    PinnedHeap* heap = get_pinned_heap();
    
    // Create pinned mapping
    PinnedBlock* block = create_pinned_mapping(heap, ptr, size);
    
    if (!block) {
        return false;
    }
    
    // Prevent GC movement
    gc_pin_object(ptr);
    
    return true;
}
```

### Memory Safety Validation

**Bounds Checking** (`src/runtime/safety/asthra_safety_memory.c`):
```c
// Safe array access with bounds checking
void* asthra_array_get_checked(void* array, size_t index) {
    ArrayHeader* header = get_array_header(array);
    
    if (index >= header->length) {
        asthra_panic("Array index out of bounds: %zu >= %zu", 
                    index, header->length);
    }
    
    return ((char*)array) + (index * header->element_size);
}

// Safe pointer dereference
void* asthra_deref_checked(void* ptr) {
    if (!ptr) {
        asthra_panic("Null pointer dereference");
    }
    
    if (!is_valid_pointer(ptr)) {
        asthra_panic("Invalid pointer dereference");
    }
    
    return *(void**)ptr;
}
``` 