/**
 * Asthra Programming Language Compiler
 * Code Generator - Statistics and Diagnostics
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for tracking and reporting code generation statistics
 */

#include "code_generator.h"
#include <stdio.h>
#include <pthread.h>
#include <stdatomic.h>

CodeGenStatistics code_generator_get_statistics(const CodeGenerator *generator) {
    if (!generator) {
        // Return zero-initialized statistics
        return (CodeGenStatistics){0};
    }
    
    // Create a copy of the statistics and update with current instruction buffer counts
    CodeGenStatistics stats = generator->stats;
    
    if (generator->instruction_buffer) {
        // Update instruction count from the instruction buffer
        atomic_store(&stats.instructions_generated, 
                    atomic_load(&generator->instruction_buffer->total_instructions_generated));
        atomic_store(&stats.bytes_generated, 
                    atomic_load(&generator->instruction_buffer->total_bytes_estimated));
    }
    
    if (generator->register_allocator) {
        // Update register allocation statistics
        atomic_store(&stats.max_register_pressure, 
                    atomic_load(&generator->register_allocator->max_registers_used));
        atomic_store(&stats.spills_generated, 
                    atomic_load(&generator->register_allocator->spill_count));
    }
    
    return stats;
}

void code_generator_reset_statistics(CodeGenerator *generator) {
    if (!generator) return;
    
    // Reset all statistics to zero
    atomic_store(&generator->stats.instructions_generated, 0);
    atomic_store(&generator->stats.basic_blocks_generated, 0);
    atomic_store(&generator->stats.functions_generated, 0);
    atomic_store(&generator->stats.bytes_generated, 0);
    atomic_store(&generator->stats.stack_bytes_allocated, 0);
    atomic_store(&generator->stats.heap_allocations, 0);
    atomic_store(&generator->stats.optimizations_applied, 0);
    atomic_store(&generator->stats.dead_code_eliminated, 0);
    atomic_store(&generator->stats.constant_folded, 0);
    atomic_store(&generator->stats.registers_allocated, 0);
    atomic_store(&generator->stats.spills_generated, 0);
    atomic_store(&generator->stats.max_register_pressure, 0);
    atomic_store(&generator->stats.generation_time_ns, 0);
    atomic_store(&generator->stats.cache_hits, 0);
    atomic_store(&generator->stats.cache_misses, 0);
}

void code_generator_print_statistics(const CodeGenerator *generator) {
    if (!generator) return;
    
    printf("Code Generation Statistics:\n");
    printf("  Instructions generated: %lu\n", 
           (unsigned long)atomic_load(&generator->stats.instructions_generated));
    printf("  Functions generated: %lu\n", 
           (unsigned long)atomic_load(&generator->stats.functions_generated));
    printf("  Bytes generated: %lu\n", 
           (unsigned long)atomic_load(&generator->stats.bytes_generated));
    
    if (generator->register_allocator) {
        printf("  Max register pressure: %u\n", 
               atomic_load(&generator->register_allocator->max_registers_used));
        printf("  Spills generated: %u\n", 
               atomic_load(&generator->register_allocator->spill_count));
    }
}

void code_generator_print_instructions(const CodeGenerator *generator) {
    if (!generator || !generator->instruction_buffer) return;
    
    InstructionBuffer *buffer = generator->instruction_buffer;
    
    printf("Assembly Instructions:\n");
    
    pthread_mutex_lock(&buffer->mutex);
    
    for (size_t i = 0; i < buffer->count; i++) {
        AssemblyInstruction *inst = buffer->instructions[i];
        if (!inst) continue;
        
        printf("  %04zu: ", i);
        
        // Print instruction type
        switch (inst->type) {
            case INST_MOV:   printf("MOV"); break;
            case INST_ADD:   printf("ADD"); break;
            case INST_SUB:   printf("SUB"); break;
            case INST_PUSH:  printf("PUSH"); break;
            case INST_POP:   printf("POP"); break;
            case INST_CALL:  printf("CALL"); break;
            case INST_RET:   printf("RET"); break;
            default:         printf("UNKNOWN(%d)", inst->type); break;
        }
        
        // Print operands
        for (size_t j = 0; j < inst->operand_count; j++) {
            AssemblyOperand *op = &inst->operands[j];
            
            if (j == 0) {
                printf(" ");
            } else {
                printf(", ");
            }
            
            switch (op->type) {
                case OPERAND_REGISTER:
                    printf("REG(%d)", op->data.reg);
                    break;
                case OPERAND_IMMEDIATE:
                    printf("IMM(%lld)", (long long)op->data.immediate);
                    break;
                case OPERAND_MEMORY:
                    printf("MEM[");
                    if (op->data.memory.base != REG_NONE) {
                        printf("REG(%d)", op->data.memory.base);
                    }
                    if (op->data.memory.index != REG_NONE) {
                        printf("+REG(%d)*%d", op->data.memory.index, op->data.memory.scale);
                    }
                    if (op->data.memory.displacement != 0) {
                        printf("%+d", op->data.memory.displacement);
                    }
                    printf("]");
                    break;
                case OPERAND_LABEL:
                    printf("LABEL(%s)", op->data.label ? op->data.label : "NULL");
                    break;
                default:
                    printf("UNKNOWN_OPERAND");
                    break;
            }
        }
        
        // Print comment if present
        if (inst->comment) {
            printf("  ; %s", inst->comment);
        }
        
        printf("\n");
    }
    
    pthread_mutex_unlock(&buffer->mutex);
}

void code_generator_print_register_allocation(const CodeGenerator *generator) {
    if (!generator || !generator->register_allocator) return;
    
    RegisterAllocator *allocator = generator->register_allocator;
    
    printf("Register Allocation:\n");
    
    pthread_mutex_lock(&allocator->mutex);
    
    printf("  Allocated registers: ");
    bool first = true;
    for (int reg = 0; reg < REG_COUNT; reg++) {
        if (allocator->allocated_mask & (1ULL << reg)) {
            if (!first) {
                printf(", ");
            }
            printf("REG(%d)", reg);
            first = false;
        }
    }
    printf("\n");
    
    printf("  Current pressure: %u\n", allocator->register_pressure);
    printf("  Max pressure: %u\n", allocator->max_registers_used);
    printf("  Spill count: %u\n", allocator->spill_count);
    
    pthread_mutex_unlock(&allocator->mutex);
}