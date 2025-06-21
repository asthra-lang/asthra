/**
 * Asthra Programming Language Compiler
 * FFI-Compatible Assembly Generator - Pattern Matching Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "ffi_assembly_generator.h"
#include "ffi_assembly_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

// Forward declarations for static functions
PatternMatchStrategy ffi_determine_pattern_strategy(FFIAssemblyGenerator *generator, PatternMatchContext *context);
bool ffi_generate_linear_pattern_match(FFIAssemblyGenerator *generator, PatternMatchContext *context);

// =============================================================================
// PATTERN MATCHING GENERATION
// =============================================================================

bool ffi_generate_match_statement(FFIAssemblyGenerator *generator, ASTNode *match_stmt) {
    if (!generator || !match_stmt || match_stmt->type != AST_MATCH_STMT) {
        return false;
    }
    
    atomic_fetch_add(&generator->pattern_matches_generated, 1);
    
    // Create pattern match context
    PatternMatchContext *context = calloc(1, sizeof(PatternMatchContext));
    if (!context) return false;
    
    generator->current_pattern_match = context;
    
    emit_comment(generator, "Pattern match statement");
    
    // Generate match expression
    Register match_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (match_reg == REG_NONE) {
        free(context);
        return false;
    }
    
    if (!code_generate_expression(generator->base_generator, 
                                 match_stmt->data.match_stmt.expression, match_reg)) {
        register_free(generator->base_generator->register_allocator, match_reg);
        free(context);
        return false;
    }
    
    context->match_value_reg = match_reg;
    context->match_type = match_stmt->data.match_stmt.expression->type_info;
    
    // Process match arms
    ASTNodeList *arms = match_stmt->data.match_stmt.arms;
    context->arm_count = arms->count;
    context->arms = calloc(context->arm_count, sizeof(*context->arms));
    
    if (!context->arms) {
        register_free(generator->base_generator->register_allocator, match_reg);
        free(context);
        return false;
    }
    
    // Determine pattern matching strategy
    context->strategy = ffi_determine_pattern_strategy(generator, context);
    
    // TODO: Advanced optimizations would go here when optimizer is implemented
    // For now, use standard pattern matching approach
    
    // Generate labels for each arm
    for (size_t i = 0; i < context->arm_count; i++) {
        context->arms[i].pattern_label = generate_unique_label(generator, "pattern");
        context->arms[i].body_label = generate_unique_label(generator, "match_body");
        context->arms[i].end_label = generate_unique_label(generator, "match_end");
    }
    
    char *match_end_label = generate_unique_label(generator, "match_complete");
    
    // Generate pattern matching code based on strategy
    switch (context->strategy) {
        case PATTERN_STRATEGY_JUMP_TABLE:
            if (!ffi_generate_pattern_jump_table(generator, context)) {
                goto cleanup_match;
            }
            break;
            
        case PATTERN_STRATEGY_LINEAR:
            if (!ffi_generate_linear_pattern_match(generator, context)) {
                goto cleanup_match;
            }
            break;
            
        default:
            goto cleanup_match;
    }
    
// generate_arm_bodies:
    // Generate match arm bodies
    for (size_t i = 0; i < context->arm_count; i++) {
        ASTNode *arm = arms->nodes[i];
        
        // Labels will be handled during assembly output, not as instructions
        
        // Generate arm body
        if (!code_generate_statement(generator->base_generator, arm->data.match_arm.body)) {
            goto cleanup_match;
        }
        
        // Jump to end
        emit_instruction(generator, INST_JMP, 1, create_label_operand(match_end_label));
    }
    
    // Labels will be handled during assembly output, not as instructions
    
    register_free(generator->base_generator->register_allocator, match_reg);
    
    // Cleanup
    for (size_t i = 0; i < context->arm_count; i++) {
        free(context->arms[i].pattern_label);
        free(context->arms[i].body_label);
        free(context->arms[i].end_label);
    }
    free(context->arms);
    free(context);
    free(match_end_label);
    generator->current_pattern_match = NULL;
    
    return true;

cleanup_match:
    register_free(generator->base_generator->register_allocator, match_reg);
    for (size_t i = 0; i < context->arm_count; i++) {
        free(context->arms[i].pattern_label);
        free(context->arms[i].body_label);
        free(context->arms[i].end_label);
    }
    free(context->arms);
    free(context);
    free(match_end_label);
    generator->current_pattern_match = NULL;
    return false;
}

bool ffi_generate_if_let_statement(FFIAssemblyGenerator *generator, ASTNode *if_let_stmt) {
    (void)generator; // Mark as unused to suppress warning
    (void)if_let_stmt; // Mark as unused to suppress warning
    // Simplified implementation - would generate if-let pattern matching
    return true;
}

bool ffi_generate_result_pattern_match(FFIAssemblyGenerator *generator,
                                     ASTNode *pattern, Register result_reg) {
    if (!generator || !pattern) return false;
    
    emit_comment(generator, "Result<T,E> pattern match");
    
    // Result types are represented as tagged unions
    // First word: tag (0 = Ok, 1 = Err)
    // Second word: value
    
    Register tag_reg = register_allocate(generator->base_generator->register_allocator, true);
    if (tag_reg == REG_NONE) return false;
    
    // Load tag from result
    emit_instruction(generator, INST_MOV, 2,
                    create_register_operand(tag_reg),
                    create_memory_operand(result_reg, REG_NONE, 1, 0));
    
    // Compare tag
    emit_instruction(generator, INST_CMP, 2,
                    create_register_operand(tag_reg),
                    create_immediate_operand(0)); // 0 = Ok
    
    char *ok_label = generate_unique_label(generator, "result_ok");
    char *err_label = generate_unique_label(generator, "result_err");
    
    emit_instruction(generator, INST_JE, 1, create_label_operand(ok_label));
    emit_instruction(generator, INST_JMP, 1, create_label_operand(err_label));
    
    register_free(generator->base_generator->register_allocator, tag_reg);
    free(ok_label);
    free(err_label);
    
    return true;
}

bool ffi_generate_pattern_jump_table(FFIAssemblyGenerator *generator,
                                    PatternMatchContext *context) {
    (void)generator; // Mark as unused to suppress warning
    (void)context; // Mark as unused to suppress warning
    // Simplified implementation - would generate jump table
    return true;
}

bool ffi_generate_result_construction(FFIAssemblyGenerator *generator,
                                    bool is_ok, Register value_reg,
                                    TypeInfo *result_type, Register result_reg) {
    (void)generator; // Mark as unused to suppress warning
    (void)is_ok; // Mark as unused to suppress warning
    (void)value_reg; // Mark as unused to suppress warning
    (void)result_type; // Mark as unused to suppress warning
    (void)result_reg; // Mark as unused to suppress warning
    // Simplified implementation - would generate Result<T,E> construction
    return true;
}

// =============================================================================
// HELPER FUNCTION IMPLEMENTATIONS
// =============================================================================

PatternMatchStrategy ffi_determine_pattern_strategy(FFIAssemblyGenerator *generator, PatternMatchContext *context) {
    if (!generator || !context) return PATTERN_STRATEGY_LINEAR;
    
    // Analyze pattern characteristics for optimal strategy
    if (context->arm_count <= 2) {
        return PATTERN_STRATEGY_LINEAR;
    }
    
    // Check if patterns are suitable for jump table
    bool all_integer_patterns = true;
    for (size_t i = 0; i < context->arm_count; i++) {
        if (!context->arms[i].is_result_ok && !context->arms[i].is_result_err) {
            all_integer_patterns = false;
            break;
        }
    }
    
    if (all_integer_patterns && context->arm_count <= 8) {
        return PATTERN_STRATEGY_JUMP_TABLE;
    }
    
    if (context->arm_count > 8) {
        return PATTERN_STRATEGY_BINARY_SEARCH;
    }
    
    return PATTERN_STRATEGY_LINEAR;
}

bool ffi_generate_linear_pattern_match(FFIAssemblyGenerator *generator, PatternMatchContext *context) {
    if (!generator || !context) return false;
    
    emit_comment(generator, "Linear pattern matching");
    
    // Generate linear comparison chain
    for (size_t i = 0; i < context->arm_count; i++) {
        // Generate comparison for this pattern
        if (context->arms[i].is_result_ok) {
            emit_instruction(generator, INST_CMP, 2,
                            create_register_operand(context->match_value_reg),
                            create_immediate_operand(0)); // Ok tag = 0
            emit_instruction(generator, INST_JE, 1,
                            create_label_operand(context->arms[i].body_label));
        } else if (context->arms[i].is_result_err) {
            emit_instruction(generator, INST_CMP, 2,
                            create_register_operand(context->match_value_reg),
                            create_immediate_operand(1)); // Err tag = 1
            emit_instruction(generator, INST_JE, 1,
                            create_label_operand(context->arms[i].body_label));
        }
    }
    
    // Generate default case if no match
    if (context->has_default_arm) {
        for (size_t i = 0; i < context->arm_count; i++) {
            if (context->arms[i].is_default) {
                emit_instruction(generator, INST_JMP, 1,
                                create_label_operand(context->arms[i].body_label));
                break;
            }
        }
    }
    
    return true;
} 
