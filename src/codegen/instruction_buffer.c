/**
 * Asthra Programming Language Compiler
 * Instruction Buffer Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "code_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

// =============================================================================
// INSTRUCTION BUFFER IMPLEMENTATION
// =============================================================================

InstructionBuffer *instruction_buffer_create(size_t initial_capacity) {
    InstructionBuffer *buffer = malloc(sizeof(InstructionBuffer));
    if (!buffer) return NULL;
    
    buffer->instructions = calloc(initial_capacity, sizeof(AssemblyInstruction*));
    if (!buffer->instructions) {
        free(buffer);
        return NULL;
    }
    
    // C17 designated initializer
    *buffer = (InstructionBuffer) {
        .instructions = buffer->instructions,
        .count = 0,
        .capacity = initial_capacity,
        .total_instructions_generated = 0,
        .total_bytes_estimated = 0
    };
    
    if (pthread_mutex_init(&buffer->mutex, NULL) != 0) {
        free(buffer->instructions);
        free(buffer);
        return NULL;
    }
    
    return buffer;
}

void instruction_buffer_destroy(InstructionBuffer *buffer) {
    if (!buffer) return;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // Free all instructions
    for (size_t i = 0; i < buffer->count; i++) {
        if (buffer->instructions[i]) {
            free(buffer->instructions[i]->comment);
            free(buffer->instructions[i]);
        }
    }
    
    free(buffer->instructions);
    pthread_mutex_unlock(&buffer->mutex);
    pthread_mutex_destroy(&buffer->mutex);
    free(buffer);
}

bool instruction_buffer_add(InstructionBuffer * restrict buffer, 
                           AssemblyInstruction * restrict instruction) {
    if (!buffer || !instruction) return false;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // Resize if necessary
    if (buffer->count >= buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        AssemblyInstruction **new_instructions = realloc(buffer->instructions, 
                                                         new_capacity * sizeof(AssemblyInstruction*));
        if (!new_instructions) {
            pthread_mutex_unlock(&buffer->mutex);
            return false;
        }
        buffer->instructions = new_instructions;
        buffer->capacity = new_capacity;
    }
    
    buffer->instructions[buffer->count++] = instruction;
    
    // Update atomic statistics
    atomic_fetch_add(&buffer->total_instructions_generated, 1);
    atomic_fetch_add(&buffer->total_bytes_estimated, 8); // Rough estimate
    
    pthread_mutex_unlock(&buffer->mutex);
    return true;
}

bool instruction_buffer_insert(InstructionBuffer * restrict buffer, 
                              size_t index, 
                              AssemblyInstruction * restrict instruction) {
    if (!buffer || !instruction) return false;
    if (index > buffer->count) return false;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // Resize if necessary
    if (buffer->count >= buffer->capacity) {
        size_t new_capacity = buffer->capacity * 2;
        AssemblyInstruction **new_instructions = realloc(buffer->instructions, 
                                                         new_capacity * sizeof(AssemblyInstruction*));
        if (!new_instructions) {
            pthread_mutex_unlock(&buffer->mutex);
            return false;
        }
        buffer->instructions = new_instructions;
        buffer->capacity = new_capacity;
    }
    
    // Shift elements to make room for new instruction
    for (size_t i = buffer->count; i > index; i--) {
        buffer->instructions[i] = buffer->instructions[i-1];
    }
    
    buffer->instructions[index] = instruction;
    buffer->count++;
    
    // Update atomic statistics
    atomic_fetch_add(&buffer->total_instructions_generated, 1);
    atomic_fetch_add(&buffer->total_bytes_estimated, 8); // Rough estimate
    
    pthread_mutex_unlock(&buffer->mutex);
    return true;
}

bool instruction_buffer_remove(InstructionBuffer * restrict buffer, size_t index) {
    if (!buffer || index >= buffer->count) return false;
    
    pthread_mutex_lock(&buffer->mutex);
    
    // Free the instruction
    free(buffer->instructions[index]->comment);
    free(buffer->instructions[index]);
    
    // Shift elements to fill the gap
    for (size_t i = index; i < buffer->count - 1; i++) {
        buffer->instructions[i] = buffer->instructions[i+1];
    }
    
    buffer->count--;
    
    // Update atomic statistics
    atomic_fetch_sub(&buffer->total_instructions_generated, 1);
    atomic_fetch_sub(&buffer->total_bytes_estimated, 8); // Rough estimate
    
    pthread_mutex_unlock(&buffer->mutex);
    return true;
} 
