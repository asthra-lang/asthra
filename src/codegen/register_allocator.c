/**
 * Asthra Programming Language Compiler
 * Register Allocator Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "code_generator.h"
#include "register_allocator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// Forward declarations
static bool is_float_type(const TypeDescriptor *type);

// C17 designated initializer for builtin register masks
static const struct {
    uint64_t caller_saved;
    uint64_t callee_saved;
} SYSV_REGISTER_MASKS = {
    .caller_saved = (1ULL << ASTHRA_REG_RAX) | (1ULL << ASTHRA_REG_RCX) | (1ULL << ASTHRA_REG_RDX) |
                   (1ULL << ASTHRA_REG_RSI) | (1ULL << ASTHRA_REG_RDI) | (1ULL << ASTHRA_REG_R8) |
                   (1ULL << ASTHRA_REG_R9) | (1ULL << ASTHRA_REG_R10) | (1ULL << ASTHRA_REG_R11),
    .callee_saved = (1ULL << ASTHRA_REG_RBX) | (1ULL << ASTHRA_REG_RSP) | (1ULL << ASTHRA_REG_RBP) |
                   (1ULL << ASTHRA_REG_R12) | (1ULL << ASTHRA_REG_R13) | (1ULL << ASTHRA_REG_R14) |
                   (1ULL << ASTHRA_REG_R15)
};

// =============================================================================
// REGISTER ALLOCATION WITH ATOMIC OPERATIONS
// =============================================================================

RegisterAllocator *register_allocator_create(void) {
    RegisterAllocator *allocator = malloc(sizeof(RegisterAllocator));
    if (!allocator) return NULL;
    
    // C17 designated initializer with System V AMD64 register masks
    *allocator = (RegisterAllocator) {
        .allocated_mask = 0,
        .caller_saved_mask = SYSV_REGISTER_MASKS.caller_saved,
        .callee_saved_mask = SYSV_REGISTER_MASKS.callee_saved,
        .register_pressure = 0,
        .spill_count = 0,
        .max_registers_used = 0
    };
    
    if (pthread_mutex_init(&allocator->mutex, NULL) != 0) {
        free(allocator);
        return NULL;
    }
    
    return allocator;
}

void register_allocator_destroy(RegisterAllocator *allocator) {
    if (!allocator) return;
    
    pthread_mutex_destroy(&allocator->mutex);
    free(allocator);
}

Register register_allocate(RegisterAllocator *allocator, bool prefer_caller_saved) {
    if (!allocator) return ASTHRA_REG_NONE;
    
    pthread_mutex_lock(&allocator->mutex);
    
    uint64_t preferred_mask = prefer_caller_saved ? allocator->caller_saved_mask : allocator->callee_saved_mask;
    uint64_t fallback_mask = prefer_caller_saved ? allocator->callee_saved_mask : allocator->caller_saved_mask;
    
    // Try preferred registers first
    for (int reg = 0; reg < ASTHRA_REG_COUNT; reg++) {
        uint64_t reg_bit = 1ULL << reg;
        if ((preferred_mask & reg_bit) && !(allocator->allocated_mask & reg_bit)) {
            allocator->allocated_mask |= reg_bit;
            
            // Update atomic statistics
            uint32_t current_pressure = atomic_fetch_add(&allocator->register_pressure, 1) + 1;
            uint32_t max_used = atomic_load(&allocator->max_registers_used);
            if (current_pressure > max_used) {
                atomic_store(&allocator->max_registers_used, current_pressure);
            }
            
            pthread_mutex_unlock(&allocator->mutex);
            return (Register)reg;
        }
    }
    
    // Try fallback registers
    for (int reg = 0; reg < ASTHRA_REG_COUNT; reg++) {
        uint64_t reg_bit = 1ULL << reg;
        if ((fallback_mask & reg_bit) && !(allocator->allocated_mask & reg_bit)) {
            allocator->allocated_mask |= reg_bit;
            
            uint32_t current_pressure = atomic_fetch_add(&allocator->register_pressure, 1) + 1;
            uint32_t max_used = atomic_load(&allocator->max_registers_used);
            if (current_pressure > max_used) {
                atomic_store(&allocator->max_registers_used, current_pressure);
            }
            
            pthread_mutex_unlock(&allocator->mutex);
            return (Register)reg;
        }
    }
    
    // No registers available - need to spill
    atomic_fetch_add(&allocator->spill_count, 1);
    pthread_mutex_unlock(&allocator->mutex);
    return ASTHRA_REG_NONE;
}

void register_free(RegisterAllocator *allocator, Register reg) {
    if (!allocator || reg == ASTHRA_REG_NONE || reg >= ASTHRA_REG_COUNT) return;
    
    pthread_mutex_lock(&allocator->mutex);
    
    uint64_t reg_bit = 1ULL << reg;
    if (allocator->allocated_mask & reg_bit) {
        allocator->allocated_mask &= ~reg_bit;
        atomic_fetch_sub(&allocator->register_pressure, 1);
    }
    
    pthread_mutex_unlock(&allocator->mutex);
}

bool register_is_allocated(RegisterAllocator *allocator, Register reg) {
    if (!allocator || reg == ASTHRA_REG_NONE || reg >= ASTHRA_REG_COUNT) return false;
    
    pthread_mutex_lock(&allocator->mutex);
    bool allocated = (allocator->allocated_mask & (1ULL << reg)) != 0;
    pthread_mutex_unlock(&allocator->mutex);
    
    return allocated;
}

bool register_allocate_parameters(RegisterAllocator *allocator, 
                                 TypeDescriptor **param_types, 
                                 size_t param_count,
                                 Register *int_regs, 
                                 Register *float_regs,
                                 size_t *stack_offset) {
    if (!allocator || !param_types || !int_regs || !float_regs || !stack_offset) {
        return false;
    }
    
    // Implementation would allocate registers for function parameters
    // based on the System V AMD64 ABI
    
    // For now, a placeholder implementation that allocates only integer registers
    pthread_mutex_lock(&allocator->mutex);
    
    size_t int_reg_index = 0;
    *stack_offset = 0;
    
    for (size_t i = 0; i < param_count; i++) {
        TypeDescriptor *type = param_types[i];
        
        // Simple rule: if param is floating point, use float regs, otherwise use int regs
        // Helper function to check if this is a floating point type
        if (type && is_float_type(type)) {
            if (i < SYSV_FLOAT_PARAM_COUNT) {
                float_regs[i] = SYSV_FLOAT_PARAM_REGS[i];
                // Mark as allocated
                allocator->allocated_mask |= (1ULL << float_regs[i]);
            } else {
                // Passed on stack
                *stack_offset += 8; // Assume 8-byte aligned
            }
        } else {
            // Integer or pointer type
            if (int_reg_index < SYSV_INT_PARAM_COUNT) {
                int_regs[int_reg_index] = SYSV_INT_PARAM_REGS[int_reg_index];
                // Mark as allocated
                allocator->allocated_mask |= (1ULL << int_regs[int_reg_index]);
                int_reg_index++;
            } else {
                // Passed on stack
                *stack_offset += 8; // Assume 8-byte aligned
            }
        }
    }
    
    pthread_mutex_unlock(&allocator->mutex);
    return true;
}

// Helper function to check if a type is a floating-point type
static bool is_float_type(const TypeDescriptor *type) {
    if (!type) return false;
    
    if (type->category == TYPE_PRIMITIVE) {
        return type->data.primitive.primitive_kind == PRIMITIVE_F32 || 
               type->data.primitive.primitive_kind == PRIMITIVE_F64;
    }
    return false;
}

bool register_spill_to_stack(CodeGenerator *generator, Register reg, size_t stack_offset) {
    if (!generator || reg == ASTHRA_REG_NONE || reg >= ASTHRA_REG_COUNT) {
        return false;
    }
    
    // Create store instruction to save register to stack
    AssemblyInstruction *store_inst = create_store_local(reg, (int32_t)stack_offset);
    if (!store_inst) {
        return false;
    }
    
    return instruction_buffer_add(generator->instruction_buffer, store_inst);
}

bool register_restore_from_stack(CodeGenerator *generator, Register reg, size_t stack_offset) {
    if (!generator || reg == ASTHRA_REG_NONE || reg >= ASTHRA_REG_COUNT) {
        return false;
    }
    
    // Create load instruction to restore register from stack
    AssemblyInstruction *load_inst = create_load_local(reg, (int32_t)stack_offset);
    if (!load_inst) {
        return false;
    }
    
    return instruction_buffer_add(generator->instruction_buffer, load_inst);
}

// =============================================================================
// STUB IMPLEMENTATIONS FOR MISSING FUNCTIONS
// =============================================================================

SpillManager* spill_manager_create(void) {
    // Safe stub implementation - return NULL (was dangerous 0x1)
    // TODO: Implement actual spill manager
    // Calling code should handle NULL gracefully
    return NULL;
}

void spill_manager_destroy(SpillManager* manager) {
    // Stub implementation - no-op
    // TODO: Implement actual cleanup
    (void)manager;
}

LivenessAnalysis* liveness_analysis_create(void) {
    // Safe stub implementation - return NULL (was dangerous 0x1)
    // TODO: Implement actual liveness analysis
    // Calling code should handle NULL gracefully
    return NULL;
}

void liveness_analysis_destroy(LivenessAnalysis* analysis) {
    // Stub implementation - no-op
    // TODO: Implement actual cleanup
    (void)analysis;
}

InterferenceGraph* interference_graph_create(void) {
    // Safe stub implementation - return NULL (was dangerous 0x1)
    // TODO: Implement actual interference graph
    // Calling code should handle NULL gracefully
    return NULL;
}

void interference_graph_destroy(InterferenceGraph* graph) {
    // Stub implementation - no-op
    // TODO: Implement actual cleanup
    (void)graph;
} 
