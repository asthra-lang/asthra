/**
 * Asthra Programming Language Compiler
 * Code Generator - Assembly Output Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Functions for emitting and writing assembly output
 */

#include "code_generator.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>

// Forward declaration for architecture-specific formatting
extern int code_generator_format_instruction(const CodeGenerator *generator, 
                                           const AssemblyInstruction *inst,
                                           char *buffer, size_t buffer_size);

bool code_generator_emit_assembly(CodeGenerator *generator, 
                                 char * restrict output_buffer, 
                                 size_t buffer_size) {
    if (!generator || !output_buffer || buffer_size == 0) return false;
    
    size_t offset = 0;
    InstructionBuffer *buffer = generator->instruction_buffer;
    
    if (!buffer) {
        printf("DEBUG: No instruction buffer\n");
        return false;
    }
    
    printf("DEBUG: Emitting assembly for %zu instructions\n", buffer->count);
    
    pthread_mutex_lock(&buffer->mutex);
    
    for (size_t i = 0; i < buffer->count && offset < buffer_size - 1; i++) {
        AssemblyInstruction *inst = buffer->instructions[i];
        if (!inst) continue;
        
        // Check if this is a directive (stored as comment)
        if (inst->comment && strncmp(inst->comment, ".global", 7) == 0) {
            // Emit the directive
            int written = snprintf(output_buffer + offset, buffer_size - offset,
                                  "%s\n", inst->comment);
            if (written < 0 || (size_t)written >= buffer_size - offset) {
                pthread_mutex_unlock(&buffer->mutex);
                return false;
            }
            offset += (size_t)written;
        } else {
            // Debug: Print instruction details
            printf("DEBUG: Instruction %zu: type=%d, operand_count=%zu\n", 
                   i, inst->type, inst->operand_count);
            
            // Print operands for debugging
            for (size_t j = 0; j < inst->operand_count; j++) {
                AssemblyOperand *op = &inst->operands[j];
                printf("  Operand %zu: type=%d", j, op->type);
                
                switch (op->type) {
                    case OPERAND_REGISTER:
                        printf(" (register: %d)", op->data.reg);
                        break;
                    case OPERAND_IMMEDIATE:
                        printf(" (immediate: %lld)", (long long)op->data.immediate);
                        break;
                    case OPERAND_MEMORY:
                        printf(" (memory: base=%d, index=%d, scale=%d, disp=%d)",
                               op->data.memory.base, op->data.memory.index,
                               op->data.memory.scale, op->data.memory.displacement);
                        break;
                    case OPERAND_LABEL:
                        printf(" (label: %s)", op->data.label);
                        break;
                    case OPERAND_COUNT:
                        printf(" (invalid operand type: OPERAND_COUNT)");
                        break;
                }
                printf("\n");
            }
            
            if (inst->comment) {
                printf("  Comment: %s\n", inst->comment);
            }
            
            // Format architecture-specific assembly instruction
            int written = code_generator_format_instruction(generator, inst, 
                                                           output_buffer + offset, 
                                                           buffer_size - offset);
            
            if (written < 0 || (size_t)written >= buffer_size - offset) {
                pthread_mutex_unlock(&buffer->mutex);
                return false;
            }
            
            offset += (size_t)written;
        }
    }
    
    pthread_mutex_unlock(&buffer->mutex);
    output_buffer[offset] = '\0';
    return true;
}

bool code_generator_write_assembly_file(CodeGenerator *generator, 
                                       const char * restrict filename) {
    if (!generator || !filename) return false;
    
    FILE *file = fopen(filename, "w");
    if (!file) return false;
    
    // Buffer for assembly output - 64K should be enough for most functions
    const size_t buffer_size = 65536;
    char *buffer = malloc(buffer_size);
    if (!buffer) {
        fclose(file);
        return false;
    }
    
    if (!code_generator_emit_assembly(generator, buffer, buffer_size)) {
        free(buffer);
        fclose(file);
        return false;
    }
    
    fprintf(file, "%s", buffer);
    free(buffer);
    fclose(file);
    
    return true;
}