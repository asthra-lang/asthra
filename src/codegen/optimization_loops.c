/**
 * Asthra Programming Language Compiler
 * Loop Optimization Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file implements advanced optimizations for loops,
 * including loop unrolling, strength reduction, and invariant code motion.
 */

#include "optimizer.h"
#include "code_generator.h"
#include "optimization_analysis.h"
#include "code_generator_instructions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

// =============================================================================
// STUB IMPLEMENTATIONS FOR MISSING FUNCTIONS
// =============================================================================

// Missing constants
#define COND_GE 5  // Greater or equal condition code

// Stub implementations for missing functions
static void emit_comment(CodeGenerator *generator, const char *format, ...) {
    (void)generator;
    (void)format;
    // TODO: Implement comment emission
}

static char *generate_unique_label(CodeGenerator *generator, const char *prefix) {
    (void)generator;
    static int label_counter = 0;
    char *label = malloc(64);
    if (label) {
        snprintf(label, 64, "%s_%d", prefix, label_counter++);
    }
    return label;
}

static void emit_label(CodeGenerator *generator, const char *label) {
    (void)generator;
    (void)label;
    // TODO: Implement label emission
}

static AssemblyInstruction *create_cmp_registers(Register reg1, Register reg2) {
    (void)reg1;
    (void)reg2;
    // TODO: Implement register comparison instruction creation
    return NULL;
}

static AssemblyInstruction *create_jcc_instruction(int condition, const char *label) {
    (void)condition;
    (void)label;
    // TODO: Implement conditional jump instruction creation
    return NULL;
}

static AssemblyInstruction *create_add_immediate(Register reg, int64_t immediate) {
    (void)reg;
    (void)immediate;
    // TODO: Implement add immediate instruction creation
    return NULL;
}

// Remove static version - use the one from code_generator_instructions.h

static bool is_operand_loop_invariant(AssemblyOperand *operand, struct LoopInfo *loop) {
    (void)operand;
    (void)loop;
    // TODO: Implement operand loop invariance check
    return false;
}

// =============================================================================
// LOOP ANALYSIS STRUCTURES
// =============================================================================

typedef struct LoopInfo {
    BasicBlock *header;
    BasicBlock *preheader;
    BasicBlock **body_blocks;
    size_t body_block_count;
    BasicBlock *exit_block;
    
    // Loop characteristics
    bool is_counted_loop;       // Has known iteration count
    bool has_invariant_code;    // Contains loop-invariant computations
    bool is_vectorizable;       // Can be auto-vectorized
    bool has_simple_increment;  // i++ or i += constant
    
    // For counted loops
    int64_t start_value;
    int64_t end_value;
    int64_t step_value;
    size_t iteration_count;
    
    // Loop variable info
    char *induction_var_name;
    Register induction_var_reg;
    
    // Invariant instructions
    AssemblyInstruction **invariant_instructions;
    size_t invariant_count;
} LoopInfo;

// =============================================================================
// LOOP ANALYSIS
// =============================================================================

static bool is_loop_invariant_instruction(AssemblyInstruction *inst, struct LoopInfo *loop) {
    if (!inst || !loop) return false;
    
    // Check if instruction depends only on values defined outside the loop
    switch (inst->type) {
        case INST_MOV:
        case INST_ADD:
        case INST_SUB:
        case INST_MUL:
            // Check if all operands are either constants or defined outside loop
            for (size_t i = 0; i < inst->operand_count; i++) {
                if (inst->operands[i].type == OPERAND_REGISTER) {
                    // TODO: Check if register is defined within loop
                    // For now, conservatively assume it's not invariant
                    return false;
                }
            }
            return true;
            
        case INST_LEA:
            // LEA (Load Effective Address) is typically invariant
            if (inst->operand_count >= 2) {
                for (size_t i = 0; i < inst->operand_count; i++) {
                    if (!is_operand_loop_invariant(&inst->operands[i], loop)) {
                        return false;
                    }
                }
            }
            return true;
            
        default:
            return false;
    }
}

static struct LoopInfo* analyze_for_loop(ASTNode *for_stmt, ControlFlowGraph *cfg) {
    if (!for_stmt || for_stmt->type != AST_FOR_STMT || !cfg) {
        return NULL;
    }
    
    struct LoopInfo *info = calloc(1, sizeof(struct LoopInfo));
    if (!info) return NULL;
    
    // Extract loop information from AST
    info->induction_var_name = strdup(for_stmt->data.for_stmt.variable);
    
    // Analyze iteration pattern
    ASTNode *iterable = for_stmt->data.for_stmt.iterable;
    if (iterable && iterable->type == AST_CALL_EXPR) {
        // Check for range(n) or range(start, end, step) patterns
        ASTNode *func = iterable->data.call_expr.function;
        if (func && func->type == AST_IDENTIFIER && 
            strcmp(func->data.identifier.name, "range") == 0) {
            
            ASTNodeList *args = iterable->data.call_expr.args;
            if (args && args->count > 0) {
                info->is_counted_loop = true;
                
                // Extract range parameters
                if (args->count == 1) {
                    // range(n) - from 0 to n-1
                    info->start_value = 0;
                    // End value would need constant evaluation
                    info->step_value = 1;
                } else if (args->count >= 2) {
                    // range(start, end) or range(start, end, step)
                    // Would need constant evaluation for these values
                    info->step_value = 1; // Default step
                }
                
                // Calculate iteration count if possible
                if (info->end_value > info->start_value && info->step_value > 0) {
                    info->iteration_count = (size_t)((info->end_value - info->start_value + info->step_value - 1) / info->step_value);
                }
            }
        }
    }
    
    // TODO: Identify loop blocks in CFG
    // This would require data flow analysis to find back edges
    
    return info;
}

// =============================================================================
// LOOP UNROLLING
// =============================================================================

static bool should_unroll_loop(struct LoopInfo *loop, size_t unroll_factor) {
    if (!loop || !loop->is_counted_loop) {
        return false;
    }
    
    // Heuristics for loop unrolling
    if (loop->iteration_count == 0 || loop->iteration_count > 1000) {
        return false; // Don't unroll very large loops
    }
    
    // Prefer small unroll factors for loops with unknown or large iteration counts
    if (loop->iteration_count > 100 && unroll_factor > 4) {
        return false;
    }
    
    // Always unroll very small loops completely
    if (loop->iteration_count <= 8) {
        return true;
    }
    
    // Unroll if iteration count is divisible by unroll factor
    if (loop->iteration_count % unroll_factor == 0) {
        return true;
    }
    
    return false;
}

bool optimize_loop_unrolling(Optimizer *optimizer, ASTNode *for_stmt, CodeGenerator *generator) {
    if (!optimizer || !for_stmt || !generator) {
        return false;
    }
    
    // Analyze loop
    struct LoopInfo *loop = analyze_for_loop(for_stmt, NULL); // CFG would be passed here
    if (!loop) {
        return false;
    }
    
    // Determine unroll factor
    size_t unroll_factor = 4; // Default unroll factor
    
    if (loop->iteration_count > 0 && loop->iteration_count <= 8) {
        // Fully unroll small loops
        unroll_factor = loop->iteration_count;
    }
    
    if (!should_unroll_loop(loop, unroll_factor)) {
        free(loop->induction_var_name);
        free(loop);
        return false;
    }
    
    emit_comment(generator, "Loop unrolling optimization (factor: %zu)", unroll_factor);
    
    // Generate unrolled loop
    if (loop->iteration_count > 0 && unroll_factor == loop->iteration_count) {
        // Full unrolling - generate loop body N times
        emit_comment(generator, "Fully unrolled loop (%zu iterations)", loop->iteration_count);
        
        for (size_t i = 0; i < loop->iteration_count; i++) {
            // Set induction variable value
            Register ind_reg = register_allocate(generator->register_allocator, true);
            if (ind_reg != REG_NONE) {
                AssemblyInstruction *mov_inst = create_mov_immediate(ind_reg, 
                    loop->start_value + (int64_t)i * loop->step_value);
                instruction_buffer_add(generator->instruction_buffer, mov_inst);
                
                // Generate loop body with induction variable substituted
                code_generate_statement(generator, for_stmt->data.for_stmt.body);
                
                register_free(generator->register_allocator, ind_reg);
            }
        }
    } else {
        // Partial unrolling - generate main loop + cleanup
        emit_comment(generator, "Partially unrolled loop (factor: %zu)", unroll_factor);
        
        // Main unrolled loop
        size_t main_iterations = loop->iteration_count / unroll_factor;
        
        // Generate loop header
        Register ind_reg = register_allocate(generator->register_allocator, true);
        Register limit_reg = register_allocate(generator->register_allocator, true);
        
        if (ind_reg != REG_NONE && limit_reg != REG_NONE) {
            // Initialize induction variable
            AssemblyInstruction *init_inst = create_mov_immediate(ind_reg, loop->start_value);
            instruction_buffer_add(generator->instruction_buffer, init_inst);
            
            // Calculate unrolled loop limit
            int64_t unrolled_limit = loop->start_value + 
                (int64_t)(main_iterations * unroll_factor) * loop->step_value;
            AssemblyInstruction *limit_inst = create_mov_immediate(limit_reg, unrolled_limit);
            instruction_buffer_add(generator->instruction_buffer, limit_inst);
            
            // Generate unrolled loop body
            char *loop_start_label = generate_unique_label(generator, "unrolled_loop");
            char *loop_end_label = generate_unique_label(generator, "unrolled_end");
            
            emit_label(generator, loop_start_label);
            
            // Compare and branch
            AssemblyInstruction *cmp_inst = create_cmp_registers(ind_reg, limit_reg);
            instruction_buffer_add(generator->instruction_buffer, cmp_inst);
            
            AssemblyInstruction *jge_inst = create_jcc_instruction(COND_GE, loop_end_label);
            instruction_buffer_add(generator->instruction_buffer, jge_inst);
            
            // Generate unrolled iterations
            for (size_t i = 0; i < unroll_factor; i++) {
                emit_comment(generator, "Unrolled iteration %zu", i);
                
                // Generate loop body
                code_generate_statement(generator, for_stmt->data.for_stmt.body);
                
                // Increment induction variable
                if (i < unroll_factor - 1) {
                    AssemblyInstruction *inc_inst = create_add_immediate(ind_reg, loop->step_value);
                    instruction_buffer_add(generator->instruction_buffer, inc_inst);
                }
            }
            
            // Final increment and jump back
            AssemblyInstruction *final_inc = create_add_immediate(ind_reg, loop->step_value);
            instruction_buffer_add(generator->instruction_buffer, final_inc);
            
            AssemblyInstruction *jmp_inst = create_jmp_instruction(loop_start_label);
            instruction_buffer_add(generator->instruction_buffer, jmp_inst);
            
            emit_label(generator, loop_end_label);
            
            // Generate cleanup loop for remaining iterations
            if (loop->iteration_count % unroll_factor != 0) {
                emit_comment(generator, "Cleanup loop for remaining iterations");
                
                // Standard loop for remaining iterations
                AssemblyInstruction *final_limit = create_mov_immediate(limit_reg, loop->end_value);
                instruction_buffer_add(generator->instruction_buffer, final_limit);
                
                char *cleanup_start = generate_unique_label(generator, "cleanup_loop");
                char *cleanup_end = generate_unique_label(generator, "cleanup_end");
                
                emit_label(generator, cleanup_start);
                
                AssemblyInstruction *cleanup_cmp = create_cmp_registers(ind_reg, limit_reg);
                instruction_buffer_add(generator->instruction_buffer, cleanup_cmp);
                
                AssemblyInstruction *cleanup_jge = create_jcc_instruction(COND_GE, cleanup_end);
                instruction_buffer_add(generator->instruction_buffer, cleanup_jge);
                
                // Generate single iteration
                code_generate_statement(generator, for_stmt->data.for_stmt.body);
                
                AssemblyInstruction *cleanup_inc = create_add_immediate(ind_reg, loop->step_value);
                instruction_buffer_add(generator->instruction_buffer, cleanup_inc);
                
                AssemblyInstruction *cleanup_jmp = create_jmp_instruction(cleanup_start);
                instruction_buffer_add(generator->instruction_buffer, cleanup_jmp);
                
                emit_label(generator, cleanup_end);
                
                free(cleanup_start);
                free(cleanup_end);
            }
            
            free(loop_start_label);
            free(loop_end_label);
            register_free(generator->register_allocator, ind_reg);
            register_free(generator->register_allocator, limit_reg);
        }
    }
    
    // Update statistics
    atomic_fetch_add(&optimizer->stats.loops_unrolled, 1);
    atomic_fetch_add(&optimizer->stats.instructions_generated, unroll_factor - 1);
    
    // Cleanup
    free(loop->induction_var_name);
    free(loop);
    
    return true;
}

// =============================================================================
// LOOP INVARIANT CODE MOTION
// =============================================================================

bool optimize_loop_invariant_code_motion(Optimizer *optimizer, ControlFlowGraph *cfg, struct LoopInfo *loop) {
    if (!optimizer || !cfg || !loop || !loop->header) {
        return false;
    }
    
    emit_comment(optimizer, "Loop invariant code motion optimization");
    
    // Find loop-invariant instructions
    loop->invariant_instructions = malloc(sizeof(AssemblyInstruction*) * 100); // Initial capacity
    loop->invariant_count = 0;
    
    // Scan loop body blocks for invariant instructions
    for (size_t i = 0; i < loop->body_block_count; i++) {
        BasicBlock *block = loop->body_blocks[i];
        
        for (size_t j = 0; j < block->instruction_count; j++) {
            AssemblyInstruction *inst = block->instructions[j];
            
            if (is_loop_invariant_instruction(inst, loop)) {
                // Add to invariant list
                loop->invariant_instructions[loop->invariant_count++] = inst;
            }
        }
    }
    
    if (loop->invariant_count == 0) {
        free(loop->invariant_instructions);
        return false;
    }
    
    // Create or find preheader block
    if (!loop->preheader) {
        // Create new preheader block
        loop->preheader = cfg_create_basic_block(cfg, "loop_preheader");
        if (!loop->preheader) {
            free(loop->invariant_instructions);
            return false;
        }
        
        // Insert preheader before loop header
        // TODO: Update CFG edges
    }
    
    // Move invariant instructions to preheader
    for (size_t i = 0; i < loop->invariant_count; i++) {
        AssemblyInstruction *inst = loop->invariant_instructions[i];
        
        // Remove from original location
        // TODO: Remove instruction from its current block
        
        // Add to preheader
        // Add instruction to preheader block
        if (loop->preheader->instruction_count >= loop->preheader->instruction_capacity) {
            size_t new_capacity = loop->preheader->instruction_capacity * 2;
            if (new_capacity == 0) new_capacity = 16;
            AssemblyInstruction **new_instructions = realloc(loop->preheader->instructions,
                                                            new_capacity * sizeof(AssemblyInstruction*));
            if (!new_instructions) {
                free(loop->invariant_instructions);
                return false;
            }
            loop->preheader->instructions = new_instructions;
            loop->preheader->instruction_capacity = new_capacity;
        }
        loop->preheader->instructions[loop->preheader->instruction_count++] = inst;
    }
    
    // Update statistics
    atomic_fetch_add(&optimizer->stats.loop_invariants_moved, loop->invariant_count);
    
    // Cleanup
    free(loop->invariant_instructions);
    
    return true;
}

// =============================================================================
// STRENGTH REDUCTION
// =============================================================================

bool optimize_loop_strength_reduction(Optimizer *optimizer, ControlFlowGraph *cfg, struct LoopInfo *loop) {
    if (!optimizer || !cfg || !loop) {
        return false;
    }
    
    bool changed = false;
    
    // Scan loop body for expensive operations
    for (size_t i = 0; i < loop->body_block_count; i++) {
        BasicBlock *block = loop->body_blocks[i];
        
        for (size_t j = 0; j < block->instruction_count; j++) {
            AssemblyInstruction *inst = block->instructions[j];
            
            // Replace multiplication by power of 2 with shift
            if (inst->type == INST_MUL && inst->operand_count >= 2) {
                if (inst->operands[1].type == OPERAND_IMMEDIATE) {
                    int64_t value = inst->operands[1].data.immediate;
                    
                    // Check if value is power of 2
                    if (value > 0 && (value & (value - 1)) == 0) {
                        // Count trailing zeros to get shift amount
                        int shift_amount = 0;
                        int64_t temp = value;
                        while ((temp & 1) == 0) {
                            shift_amount++;
                            temp >>= 1;
                        }
                        
                        // Replace MUL with SHL
                        inst->type = INST_SHL;
                        inst->operands[1].data.immediate = shift_amount;
                        
                        atomic_fetch_add(&optimizer->stats.strength_reductions, 1);
                        changed = true;
                    }
                }
            }
            
            // Replace division by power of 2 with shift (for unsigned)
            else if (inst->type == INST_DIV && inst->operand_count >= 2) {
                if (inst->operands[1].type == OPERAND_IMMEDIATE) {
                    int64_t value = inst->operands[1].data.immediate;
                    
                    // Check if value is power of 2
                    if (value > 0 && (value & (value - 1)) == 0) {
                        // Count trailing zeros to get shift amount
                        int shift_amount = 0;
                        int64_t temp = value;
                        while ((temp & 1) == 0) {
                            shift_amount++;
                            temp >>= 1;
                        }
                        
                        // Replace DIV with SHR (for unsigned)
                        inst->type = INST_SHR;
                        inst->operands[1].data.immediate = shift_amount;
                        
                        atomic_fetch_add(&optimizer->stats.strength_reductions, 1);
                        changed = true;
                    }
                }
            }
        }
    }
    
    return changed;
}

// =============================================================================
// MAIN LOOP OPTIMIZATION ENTRY POINT
// =============================================================================

bool optimize_for_loop(Optimizer *optimizer, ASTNode *for_stmt, CodeGenerator *generator) {
    if (!optimizer || !for_stmt || for_stmt->type != AST_FOR_STMT || !generator) {
        return false;
    }
    
    bool optimized = false;
    
    // Try loop unrolling first
    if (optimize_loop_unrolling(optimizer, for_stmt, generator)) {
        optimized = true;
    }
    
    // Other optimizations would be applied after CFG construction
    // - Loop invariant code motion
    // - Strength reduction
    // - Vectorization
    
    return optimized;
}