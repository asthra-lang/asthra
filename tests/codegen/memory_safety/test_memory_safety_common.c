#include "test_memory_safety_common.h"

// Core memory management functions
void init_memory_safety_context(MemorySafetyContext* ctx) {
    ctx->block_count = 0;
    ctx->lifetime_count = 0;
    ctx->borrow_count = 0;
    ctx->current_scope = 0;
    ctx->safety_enabled = true;
    ctx->violations_detected = 0;
    ctx->last_violation = NULL;
    ctx->memory_used = 0;
    ctx->allocations_made = 0;
    ctx->deallocations_made = 0;
    ctx->ownership_transfers = 0;
    ctx->borrow_violations = 0;
    
    for (int i = 0; i < 256; i++) {
        ctx->blocks[i] = (MemoryBlock){0};
    }
    
    for (int i = 0; i < 64; i++) {
        ctx->lifetimes[i] = (Lifetime){0};
    }
    
    for (int i = 0; i < 128; i++) {
        ctx->borrows[i] = (BorrowReference){0};
    }
    
    memset(ctx->memory_pool, 0, sizeof(ctx->memory_pool));
}

void* safe_allocate(MemorySafetyContext* ctx, size_t size, const char* location) {
    if (ctx->memory_used + size > sizeof(ctx->memory_pool) || ctx->block_count >= 256) {
        return NULL;
    }
    
    // Align allocation
    size_t aligned_size = (size + 7) & ~7;
    size_t aligned_offset = (ctx->memory_used + 7) & ~7;
    
    if (aligned_offset + aligned_size > sizeof(ctx->memory_pool)) {
        return NULL;
    }
    
    void* ptr = &ctx->memory_pool[aligned_offset];
    ctx->memory_used = aligned_offset + aligned_size;
    
    // Create memory block
    MemoryBlock* block = &ctx->blocks[ctx->block_count++];
    block->ptr = ptr;
    block->size = size;
    block->state = MEMORY_STATE_OWNED;
    block->ownership = OWNERSHIP_OWNED;
    block->ref_count = 1;
    block->lifetime_id = -1;
    block->allocated_at = location;
    block->allocation_time = (uint64_t)ctx->allocations_made;
    block->is_array = false;
    block->array_length = 0;
    block->bounds_checked = true;
    
    ctx->allocations_made++;
    
    // Initialize memory to zero
    memset(ptr, 0, size);
    
    return ptr;
}

void* safe_allocate_array(MemorySafetyContext* ctx, size_t element_size, size_t count, const char* location) {
    size_t total_size = element_size * count;
    void* ptr = safe_allocate(ctx, total_size, location);
    
    if (ptr) {
        // Find the block and mark it as an array
        for (int i = 0; i < ctx->block_count; i++) {
            if (ctx->blocks[i].ptr == ptr) {
                ctx->blocks[i].is_array = true;
                ctx->blocks[i].array_length = count;
                break;
            }
        }
    }
    
    return ptr;
}

MemoryBlock* find_memory_block(MemorySafetyContext* ctx, void* ptr) {
    for (int i = 0; i < ctx->block_count; i++) {
        if (ctx->blocks[i].ptr == ptr) {
            return &ctx->blocks[i];
        }
    }
    return NULL;
}

bool safe_deallocate(MemorySafetyContext* ctx, void* ptr, const char* location) {
    MemoryBlock* block = find_memory_block(ctx, ptr);
    
    if (!block) {
        ctx->violations_detected++;
        ctx->last_violation = "Attempt to free unknown pointer";
        return false;
    }
    
    if (block->state == MEMORY_STATE_FREED) {
        ctx->violations_detected++;
        ctx->last_violation = "Double free detected";
        return false;
    }
    
    if (block->ownership != OWNERSHIP_OWNED) {
        ctx->violations_detected++;
        ctx->last_violation = "Attempt to free non-owned memory";
        return false;
    }
    
    if (block->ref_count > 1) {
        ctx->violations_detected++;
        ctx->last_violation = "Attempt to free memory with outstanding references";
        return false;
    }
    
    block->state = MEMORY_STATE_FREED;
    block->ownership = OWNERSHIP_NONE;
    ctx->deallocations_made++;
    
    return true;
}

// Ownership and borrowing functions
bool transfer_ownership(MemorySafetyContext* ctx, void* ptr, const char* location) {
    MemoryBlock* block = find_memory_block(ctx, ptr);
    
    if (!block) {
        ctx->violations_detected++;
        ctx->last_violation = "Attempt to transfer ownership of unknown pointer";
        return false;
    }
    
    if (block->state == MEMORY_STATE_MOVED) {
        ctx->violations_detected++;
        ctx->last_violation = "Use after move detected";
        return false;
    }
    
    if (block->ownership != OWNERSHIP_OWNED) {
        ctx->violations_detected++;
        ctx->last_violation = "Cannot transfer non-owned memory";
        return false;
    }
    
    block->state = MEMORY_STATE_MOVED;
    ctx->ownership_transfers++;
    
    return true;
}

BorrowReference* create_borrow(MemorySafetyContext* ctx, void* ptr, OwnershipType borrow_type, int lifetime_id, const char* location) {
    MemoryBlock* block = find_memory_block(ctx, ptr);
    
    if (!block) {
        ctx->violations_detected++;
        ctx->last_violation = "Attempt to borrow unknown pointer";
        return NULL;
    }
    
    if (block->state == MEMORY_STATE_MOVED || block->state == MEMORY_STATE_FREED) {
        ctx->violations_detected++;
        ctx->last_violation = "Attempt to borrow invalid memory";
        ctx->borrow_violations++;
        return NULL;
    }
    
    // Check for conflicting borrows
    if (borrow_type == OWNERSHIP_BORROWED_MUTABLE) {
        for (int i = 0; i < ctx->borrow_count; i++) {
            BorrowReference* existing = &ctx->borrows[i];
            if (existing->from_block == block && existing->is_active) {
                ctx->violations_detected++;
                ctx->last_violation = "Multiple mutable borrows detected";
                ctx->borrow_violations++;
                return NULL;
            }
        }
    } else if (borrow_type == OWNERSHIP_BORROWED_SHARED) {
        for (int i = 0; i < ctx->borrow_count; i++) {
            BorrowReference* existing = &ctx->borrows[i];
            if (existing->from_block == block && existing->is_active && 
                existing->borrow_type == OWNERSHIP_BORROWED_MUTABLE) {
                ctx->violations_detected++;
                ctx->last_violation = "Shared borrow while mutable borrow exists";
                ctx->borrow_violations++;
                return NULL;
            }
        }
    }
    
    if (ctx->borrow_count >= 128) {
        return NULL;
    }
    
    BorrowReference* borrow = &ctx->borrows[ctx->borrow_count++];
    borrow->from_block = block;
    borrow->to_block = NULL;
    borrow->borrow_type = borrow_type;
    borrow->lifetime_id = lifetime_id;
    borrow->is_active = true;
    borrow->created_at = location;
    
    block->ref_count++;
    
    return borrow;
}

void end_borrow(MemorySafetyContext* ctx, BorrowReference* borrow) {
    if (borrow && borrow->is_active) {
        borrow->is_active = false;
        if (borrow->from_block) {
            borrow->from_block->ref_count--;
        }
    }
}

// Lifetime management functions
Lifetime* create_lifetime(MemorySafetyContext* ctx, const char* name, int start_scope) {
    if (ctx->lifetime_count >= 64) {
        return NULL;
    }
    
    Lifetime* lifetime = &ctx->lifetimes[ctx->lifetime_count];
    lifetime->lifetime_id = ctx->lifetime_count;
    lifetime->name = name;
    lifetime->start_scope = start_scope;
    lifetime->end_scope = -1;
    lifetime->is_active = true;
    lifetime->block_count = 0;
    
    ctx->lifetime_count++;
    return lifetime;
}

void end_lifetime(MemorySafetyContext* ctx, Lifetime* lifetime, int end_scope) {
    if (lifetime && lifetime->is_active) {
        lifetime->end_scope = end_scope;
        lifetime->is_active = false;
        
        // End all borrows associated with this lifetime
        for (int i = 0; i < ctx->borrow_count; i++) {
            BorrowReference* borrow = &ctx->borrows[i];
            if (borrow->lifetime_id == lifetime->lifetime_id && borrow->is_active) {
                end_borrow(ctx, borrow);
            }
        }
    }
}

// Safety validation functions
bool check_array_bounds(MemorySafetyContext* ctx, void* ptr, size_t index, const char* location) {
    MemoryBlock* block = find_memory_block(ctx, ptr);
    
    if (!block) {
        ctx->violations_detected++;
        ctx->last_violation = "Array bounds check on unknown pointer";
        return false;
    }
    
    if (!block->is_array) {
        ctx->violations_detected++;
        ctx->last_violation = "Array access on non-array memory";
        return false;
    }
    
    if (index >= block->array_length) {
        ctx->violations_detected++;
        ctx->last_violation = "Array index out of bounds";
        return false;
    }
    
    if (block->state == MEMORY_STATE_FREED || block->state == MEMORY_STATE_MOVED) {
        ctx->violations_detected++;
        ctx->last_violation = "Array access on invalid memory";
        return false;
    }
    
    return true;
}

int count_memory_leaks(MemorySafetyContext* ctx) {
    int leaks = 0;
    
    for (int i = 0; i < ctx->block_count; i++) {
        MemoryBlock* block = &ctx->blocks[i];
        if (block->state == MEMORY_STATE_OWNED && block->ownership == OWNERSHIP_OWNED) {
            leaks++;
        }
    }
    
    return leaks;
} 
