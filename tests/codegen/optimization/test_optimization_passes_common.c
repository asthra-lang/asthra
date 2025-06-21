/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Common Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "test_optimization_passes_common.h"

// =============================================================================
// CORE INFRASTRUCTURE IMPLEMENTATIONS
// =============================================================================

Optimizer* test_optimizer_create(OptimizationLevel level) {
    Optimizer* opt = malloc(sizeof(Optimizer));
    if (opt) {
        opt->level = level;
        opt->enabled = true;
    }
    return opt;
}

void test_optimizer_destroy(Optimizer* opt) {
    free(opt);
}

InstructionBuffer* test_instruction_buffer_create(size_t capacity) {
    InstructionBuffer* buffer = malloc(sizeof(InstructionBuffer));
    if (buffer) {
        buffer->instructions = malloc(capacity * sizeof(Instruction));
        buffer->count = 0;
        buffer->capacity = capacity;
    }
    return buffer;
}

void test_instruction_buffer_destroy(InstructionBuffer* buffer) {
    if (buffer) {
        free(buffer->instructions);
        free(buffer);
    }
}

bool test_instruction_buffer_add(InstructionBuffer* buffer, const Instruction* instr) {
    if (!buffer || !instr || buffer->count >= buffer->capacity) return false;
    buffer->instructions[buffer->count++] = *instr;
    return true;
}

size_t test_instruction_buffer_size(InstructionBuffer* buffer) {
    return buffer ? buffer->count : 0;
}

OptimizationContext* test_optimization_context_create(void) {
    OptimizationContext* ctx = malloc(sizeof(OptimizationContext));
    if (ctx) {
        ctx->constant_folding_enabled = true;
        ctx->dead_code_elimination_enabled = true;
        ctx->common_subexpression_elimination_enabled = true;
        ctx->strength_reduction_enabled = true;
    }
    return ctx;
}

void test_optimization_context_destroy(OptimizationContext* ctx) {
    free(ctx);
}

// =============================================================================
// OPTIMIZATION PASS IMPLEMENTATIONS
// =============================================================================

OptimizationResult test_optimizer_constant_folding(Optimizer* opt, InstructionBuffer* buffer) {
    OptimizationResult result = {false, 0, 0, 0, 0, 0};
    if (!opt || !buffer) return result;
    
    result.success = true;
    
    // Simulate constant folding
    for (size_t i = 0; i < buffer->count; i++) {
        Instruction* instr = &buffer->instructions[i];
        
        // Fold multiplication of constants
        if (instr->opcode == OP_MUL &&
            instr->operand1.type == OPERAND_IMMEDIATE &&
            instr->operand2.type == OPERAND_IMMEDIATE) {
            
            int folded_value = instr->operand1.value * instr->operand2.value;
            instr->opcode = OP_MOV;
            instr->operand1.type = OPERAND_IMMEDIATE;
            instr->operand1.value = folded_value;
            instr->operand2.type = OPERAND_IMMEDIATE;
            instr->operand2.value = 0;
            
            result.constant_folds++;
            result.optimizations_applied++;
        }
        
        // Fold addition of constants
        if (instr->opcode == OP_ADD &&
            instr->operand1.type == OPERAND_IMMEDIATE &&
            instr->operand2.type == OPERAND_IMMEDIATE) {
            
            int folded_value = instr->operand1.value + instr->operand2.value;
            instr->opcode = OP_MOV;
            instr->operand1.type = OPERAND_IMMEDIATE;
            instr->operand1.value = folded_value;
            instr->operand2.type = OPERAND_IMMEDIATE;
            instr->operand2.value = 0;
            
            result.constant_folds++;
            result.optimizations_applied++;
        }
        
        // Fold subtraction of constants
        if (instr->opcode == OP_SUB &&
            instr->operand1.type == OPERAND_IMMEDIATE &&
            instr->operand2.type == OPERAND_IMMEDIATE) {
            
            int folded_value = instr->operand1.value - instr->operand2.value;
            instr->opcode = OP_MOV;
            instr->operand1.type = OPERAND_IMMEDIATE;
            instr->operand1.value = folded_value;
            instr->operand2.type = OPERAND_IMMEDIATE;
            instr->operand2.value = 0;
            
            result.constant_folds++;
            result.optimizations_applied++;
        }
    }
    
    return result;
}

OptimizationResult test_optimizer_dead_code_elimination(Optimizer* opt, InstructionBuffer* buffer) {
    OptimizationResult result = {false, 0, 0, 0, 0, 0};
    if (!opt || !buffer) return result;
    
    result.success = true;
    
    // Simulate dead code elimination
    size_t write_index = 0;
    for (size_t read_index = 0; read_index < buffer->count; read_index++) {
        Instruction* instr = &buffer->instructions[read_index];
        
        if (!instr->is_used) {
            // Dead instruction - skip it
            result.dead_code_eliminated++;
            result.optimizations_applied++;
        } else {
            // Live instruction - keep it
            if (write_index != read_index) {
                buffer->instructions[write_index] = *instr;
            }
            write_index++;
        }
    }
    
    buffer->count = write_index;
    return result;
}

OptimizationResult test_optimizer_common_subexpression_elimination(Optimizer* opt, InstructionBuffer* buffer) {
    OptimizationResult result = {false, 0, 0, 0, 0, 0};
    if (!opt || !buffer) return result;
    
    result.success = true;
    
    // Simulate common subexpression elimination
    for (size_t i = 0; i < buffer->count; i++) {
        for (size_t j = i + 1; j < buffer->count; j++) {
            Instruction* instr1 = &buffer->instructions[i];
            Instruction* instr2 = &buffer->instructions[j];
            
            // Check for identical expressions
            if (instr1->opcode == instr2->opcode &&
                instr1->opcode == OP_ADD &&
                instr1->operand1.type == instr2->operand1.type &&
                instr1->operand2.type == instr2->operand2.type &&
                instr1->operand1.value == instr2->operand1.value &&
                instr1->operand2.value == instr2->operand2.value) {
                
                // Replace second occurrence with move from first result
                instr2->opcode = OP_MOV;
                instr2->operand1.type = OPERAND_REGISTER;
                instr2->operand1.reg = instr1->dst_reg;
                instr2->operand2.type = OPERAND_IMMEDIATE;
                instr2->operand2.value = 0;
                
                result.common_subexpressions++;
                result.optimizations_applied++;
            }
        }
    }
    
    return result;
}

OptimizationResult test_optimizer_strength_reduction(Optimizer* opt, InstructionBuffer* buffer) {
    OptimizationResult result = {false, 0, 0, 0, 0, 0};
    if (!opt || !buffer) return result;
    
    result.success = true;
    
    // Simulate strength reduction
    for (size_t i = 0; i < buffer->count; i++) {
        Instruction* instr = &buffer->instructions[i];
        
        // Replace multiplication by 2 with addition
        if (instr->opcode == OP_MUL &&
            instr->operand2.type == OPERAND_IMMEDIATE &&
            instr->operand2.value == 2) {
            
            instr->opcode = OP_ADD;
            instr->operand2 = instr->operand1; // x + x instead of x * 2
            
            result.strength_reductions++;
            result.optimizations_applied++;
        }
        
        // Replace multiplication by power of 2 with shift (simulated as ADD for simplicity)
        if (instr->opcode == OP_MUL &&
            instr->operand2.type == OPERAND_IMMEDIATE &&
            (instr->operand2.value == 4 || instr->operand2.value == 8)) {
            
            instr->opcode = OP_ADD; // Simplified representation of shift
            instr->operand2.value = 1; // Simplified shift amount
            
            result.strength_reductions++;
            result.optimizations_applied++;
        }
    }
    
    return result;
}

OptimizationResult test_optimizer_peephole_optimization(Optimizer* opt, InstructionBuffer* buffer) {
    OptimizationResult result = {false, 0, 0, 0, 0, 0};
    if (!opt || !buffer) return result;
    
    result.success = true;
    
    // Simulate peephole optimizations
    for (size_t i = 0; i < buffer->count - 1; i++) {
        Instruction* instr1 = &buffer->instructions[i];
        Instruction* instr2 = &buffer->instructions[i + 1];
        
        // Eliminate redundant moves: MOV x, y; MOV y, x
        if (instr1->opcode == OP_MOV && instr2->opcode == OP_MOV &&
            instr1->dst_reg == instr2->operand1.reg &&
            instr2->dst_reg == instr1->operand1.reg) {
            
            // Mark second instruction as NOP
            instr2->opcode = OP_NOP;
            result.optimizations_applied++;
        }
        
        // Eliminate ADD x, 0
        if (instr1->opcode == OP_ADD &&
            instr1->operand2.type == OPERAND_IMMEDIATE &&
            instr1->operand2.value == 0) {
            
            instr1->opcode = OP_NOP;
            result.optimizations_applied++;
        }
    }
    
    return result;
} 
