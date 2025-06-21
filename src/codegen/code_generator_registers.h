/**
 * Asthra Programming Language Compiler
 * Code Generation Register Allocation Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_REGISTERS_H
#define ASTHRA_CODE_GENERATOR_REGISTERS_H

#include "code_generator_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// REGISTER ALLOCATION WITH ATOMIC OPERATIONS
// =============================================================================

// Register allocation state
struct RegisterAllocator {
    uint64_t allocated_mask;  // Bitmask of allocated registers
    uint64_t caller_saved_mask;  // Bitmask of caller-saved registers
    uint64_t callee_saved_mask;  // Bitmask of callee-saved registers
    
    // Register usage statistics with atomic operations
    atomic_uint_fast32_t register_pressure;
    atomic_uint_fast32_t spill_count;
    atomic_uint_fast32_t max_registers_used;
    
    // Thread safety
    pthread_mutex_t mutex;
};

// =============================================================================
// REGISTER ALLOCATION WITH ATOMIC OPERATIONS
// =============================================================================

RegisterAllocator *register_allocator_create(void);
void register_allocator_destroy(RegisterAllocator *allocator);

// Thread-safe register allocation
Register register_allocate(RegisterAllocator *allocator, bool prefer_caller_saved);
void register_free(RegisterAllocator *allocator, Register reg);
bool register_is_allocated(RegisterAllocator *allocator, Register reg);

// Register allocation for function calls (System V AMD64 ABI)
bool register_allocate_parameters(RegisterAllocator *allocator, 
                                 TypeDescriptor **param_types, 
                                 size_t param_count,
                                 Register *int_regs, 
                                 Register *float_regs,
                                 size_t *stack_offset);

// Register spilling
bool register_spill_to_stack(CodeGenerator *generator, Register reg, size_t stack_offset);
bool register_restore_from_stack(CodeGenerator *generator, Register reg, size_t stack_offset);

// Type-safe register operations
#define REGISTER_OPERATION(allocator, reg) _Generic((reg), \
    Register: register_free, \
    default: register_free \
)(allocator, reg)

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_REGISTERS_H 
