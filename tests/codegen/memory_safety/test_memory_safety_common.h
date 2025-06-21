#ifndef TEST_MEMORY_SAFETY_COMMON_H
#define TEST_MEMORY_SAFETY_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Memory management types
typedef enum {
    MEMORY_STATE_UNINITIALIZED,
    MEMORY_STATE_OWNED,
    MEMORY_STATE_BORROWED_SHARED,
    MEMORY_STATE_BORROWED_MUTABLE,
    MEMORY_STATE_MOVED,
    MEMORY_STATE_FREED,
    MEMORY_STATE_INVALID,
    MEMORY_STATE_COUNT
} MemoryState;

typedef enum {
    OWNERSHIP_OWNED,
    OWNERSHIP_BORROWED_SHARED,
    OWNERSHIP_BORROWED_MUTABLE,
    OWNERSHIP_WEAK_REF,
    OWNERSHIP_NONE,
    OWNERSHIP_COUNT
} OwnershipType;

typedef struct {
    void* ptr;
    size_t size;
    MemoryState state;
    OwnershipType ownership;
    int ref_count;
    int lifetime_id;
    const char* allocated_at;
    uint64_t allocation_time;
    bool is_array;
    size_t array_length;
    bool bounds_checked;
} MemoryBlock;

typedef struct {
    int lifetime_id;
    const char* name;
    int start_scope;
    int end_scope;
    bool is_active;
    MemoryBlock* blocks[32];
    int block_count;
} Lifetime;

typedef struct {
    MemoryBlock* from_block;
    MemoryBlock* to_block;
    OwnershipType borrow_type;
    int lifetime_id;
    bool is_active;
    const char* created_at;
} BorrowReference;

typedef struct {
    MemoryBlock blocks[256];
    int block_count;
    Lifetime lifetimes[64];
    int lifetime_count;
    BorrowReference borrows[128];
    int borrow_count;
    
    // Safety tracking
    int current_scope;
    bool safety_enabled;
    int violations_detected;
    const char* last_violation;
    
    // Memory pool for testing
    uint8_t memory_pool[8192];
    size_t memory_used;
    
    // Statistics
    int allocations_made;
    int deallocations_made;
    int ownership_transfers;
    int borrow_violations;
} MemorySafetyContext;

// Core memory management functions
void init_memory_safety_context(MemorySafetyContext* ctx);
void* safe_allocate(MemorySafetyContext* ctx, size_t size, const char* location);
void* safe_allocate_array(MemorySafetyContext* ctx, size_t element_size, size_t count, const char* location);
MemoryBlock* find_memory_block(MemorySafetyContext* ctx, void* ptr);
bool safe_deallocate(MemorySafetyContext* ctx, void* ptr, const char* location);

// Ownership and borrowing functions
bool transfer_ownership(MemorySafetyContext* ctx, void* ptr, const char* location);
BorrowReference* create_borrow(MemorySafetyContext* ctx, void* ptr, OwnershipType borrow_type, int lifetime_id, const char* location);
void end_borrow(MemorySafetyContext* ctx, BorrowReference* borrow);

// Lifetime management functions
Lifetime* create_lifetime(MemorySafetyContext* ctx, const char* name, int start_scope);
void end_lifetime(MemorySafetyContext* ctx, Lifetime* lifetime, int end_scope);

// Safety validation functions
bool check_array_bounds(MemorySafetyContext* ctx, void* ptr, size_t index, const char* location);
int count_memory_leaks(MemorySafetyContext* ctx);

// Test function declarations
AsthraTestResult test_ownership_tracking(AsthraTestContext* context);
AsthraTestResult test_lifetime_management(AsthraTestContext* context);
AsthraTestResult test_memory_leak_detection(AsthraTestContext* context);
AsthraTestResult test_bounds_checking(AsthraTestContext* context);

#endif // TEST_MEMORY_SAFETY_COMMON_H 
