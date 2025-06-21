/**
 * Asthra Programming Language Compiler
 * Match Expression Optimization Implementation
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * This file implements advanced optimizations for match expressions,
 * including jump tables, decision trees, and pattern analysis.
 */

#include "optimizer.h"
#include "code_generator.h"
#include "optimization_analysis.h"
#include "code_generator_instructions.h"
#include "code_generator_core.h"
#include "code_generator_registers.h"
#include "code_generator_labels.h"
#include "code_generator_types.h"
#include "ffi_patterns.h"
#include "ffi_generator_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <limits.h>
#include <stdatomic.h>

// =============================================================================
// FORWARD DECLARATIONS AND TYPE DEFINITIONS
// =============================================================================

// Forward declarations for types that may not be fully defined
typedef struct FFIAssemblyGenerator FFIAssemblyGenerator;
typedef struct PatternMatchContext PatternMatchContext;

// Stub implementation for generate_unique_label
static char *generate_unique_label(struct FFIAssemblyGenerator *generator, const char *prefix) {
    (void)generator;
    static int label_counter = 0;
    char *label = malloc(64);
    if (label) {
        snprintf(label, 64, "%s_%d", prefix, ++label_counter);
    }
    return label;
}

// Implementation for emit_data_quad
static void emit_data_quad(CodeGenerator *generator, const char *label) {
    if (!generator || !label) return;
    
    // Create a data directive instruction (using MOV as placeholder)
    AssemblyInstruction *inst = create_instruction_empty(INST_MOV, 0);
    if (!inst) return;
    
    // Set comment to indicate this is a data quad
    size_t comment_len = strlen(label) + 32;
    inst->comment = malloc(comment_len);
    if (inst->comment) {
        snprintf(inst->comment, comment_len, ".quad %s", label);
    }
    
    instruction_buffer_add(generator->instruction_buffer, inst);
}

// Implementation for emit_comment
static void emit_comment(CodeGenerator *generator, const char *format, ...) {
    if (!generator || !format) return;
    
    // Only emit comments if configured to do so
    if (!generator->config.emit_comments) return;
    
    va_list args;
    va_start(args, format);
    
    // Format the comment string
    size_t buffer_size = 256;
    char *comment = malloc(buffer_size);
    if (!comment) {
        va_end(args);
        return;
    }
    
    int result = vsnprintf(comment, buffer_size, format, args);
    va_end(args);
    
    if (result >= (int)buffer_size) {
        // Buffer was too small, allocate a larger one
        buffer_size = result + 1;
        comment = realloc(comment, buffer_size);
        if (!comment) return;
        
        va_start(args, format);
        vsnprintf(comment, buffer_size, format, args);
        va_end(args);
    }
    
    // Create a comment instruction
    AssemblyInstruction *inst = create_instruction_empty(INST_MOV, 0);
    if (!inst) {
        free(comment);
        return;
    }
    
    inst->comment = comment;
    instruction_buffer_add(generator->instruction_buffer, inst);
}

// Implementation for emit_label
static void emit_label(CodeGenerator *generator, const char *label) {
    if (!generator || !label) return;
    
    // Define the label in the label manager
    if (generator->label_manager) {
        label_manager_define_label(generator->label_manager, 
                                 label, 
                                 generator->instruction_buffer->count);
    }
}

// Implementation for emit_instruction
static void emit_instruction(CodeGenerator *generator, InstructionType opcode, int operand_count, ...) {
    if (!generator) return;
    
    va_list args;
    va_start(args, operand_count);
    
    // Create instruction with operands
    AssemblyInstruction *inst = create_instruction_empty(opcode, operand_count);
    if (!inst) {
        va_end(args);
        return;
    }
    
    // Process variadic arguments for operands
    for (int i = 0; i < operand_count; i++) {
        inst->operands[i] = va_arg(args, AssemblyOperand);
    }
    
    va_end(args);
    
    // Add instruction to buffer
    instruction_buffer_add(generator->instruction_buffer, inst);
}

// Use existing functions from included headers
// create_register_operand, create_immediate_operand, etc. are already declared
// register_allocate, register_free are already declared
// Register type is already defined

// Instruction constants
#define INST_SUB 1
#define INST_MOV 2
#define INST_CMP 3
#define INST_JAE 4
#define INST_LEA 5
#define INST_SHL 6
#define INST_ADD 7
#define INST_JMP 8
#define INST_JE 9
#define INST_JL 10

// =============================================================================
// MATCH PATTERN ANALYSIS
// =============================================================================

typedef struct MatchPatternInfo {
    enum {
        MATCH_PATTERN_INTEGER,
        MATCH_PATTERN_ENUM,
        MATCH_PATTERN_STRUCT,
        MATCH_PATTERN_WILDCARD,
        MATCH_PATTERN_RANGE,
        MATCH_PATTERN_GUARD
    } type;
    
    union {
        int64_t integer_value;
        struct {
            char *enum_name;
            char *variant_name;
            int discriminant;
        } enum_info;
        struct {
            int64_t start;
            int64_t end;
        } range;
    } data;
    
    bool has_guard;
    size_t arm_index;
} MatchPatternInfo;

typedef struct MatchAnalysisResult {
    MatchPatternInfo *patterns;
    size_t pattern_count;
    
    bool is_exhaustive;
    bool has_overlapping_patterns;
    bool suitable_for_jump_table;
    bool suitable_for_binary_search;
    
    // For jump table optimization
    int64_t min_value;
    int64_t max_value;
    size_t dense_range_size;
    
    // Pattern distribution
    size_t integer_pattern_count;
    size_t enum_pattern_count;
    size_t wildcard_count;
} MatchAnalysisResult;

// Analyze match patterns for optimization opportunities
static MatchAnalysisResult* analyze_match_patterns(ASTNode *match_stmt) {
    if (!match_stmt || match_stmt->type != AST_MATCH_STMT) {
        return NULL;
    }
    
    MatchAnalysisResult *result = calloc(1, sizeof(MatchAnalysisResult));
    if (!result) return NULL;
    
    ASTNodeList *arms = match_stmt->data.match_stmt.arms;
    result->pattern_count = arms->count;
    result->patterns = calloc(arms->count, sizeof(MatchPatternInfo));
    if (!result->patterns) {
        free(result);
        return NULL;
    }
    
    result->min_value = INT64_MAX;
    result->max_value = INT64_MIN;
    
    // Analyze each pattern
    for (size_t i = 0; i < arms->count; i++) {
        ASTNode *arm = arms->nodes[i];
        ASTNode *pattern = arm->data.match_arm.pattern;
        MatchPatternInfo *info = &result->patterns[i];
        info->arm_index = i;
        
        if (arm->data.match_arm.guard) {
            info->has_guard = true;
        }
        
        // Analyze pattern type
        switch (pattern->type) {
            case AST_INTEGER_LITERAL:
                info->type = MATCH_PATTERN_INTEGER;
                info->data.integer_value = pattern->data.integer_literal.value;
                result->integer_pattern_count++;
                
                // Update range for jump table analysis
                if (info->data.integer_value < result->min_value) {
                    result->min_value = info->data.integer_value;
                }
                if (info->data.integer_value > result->max_value) {
                    result->max_value = info->data.integer_value;
                }
                break;
                
            case AST_ENUM_PATTERN:
                info->type = MATCH_PATTERN_ENUM;
                info->data.enum_info.enum_name = pattern->data.enum_pattern.enum_name;
                info->data.enum_info.variant_name = pattern->data.enum_pattern.variant_name;
                // Discriminant would be resolved during semantic analysis
                result->enum_pattern_count++;
                break;
                
            case AST_IDENTIFIER:
                // Check if it's a wildcard pattern (_)
                if (strcmp(pattern->data.identifier.name, "_") == 0) {
                    info->type = MATCH_PATTERN_WILDCARD;
                    result->wildcard_count++;
                    result->is_exhaustive = true;
                }
                break;
                
            default:
                // Other pattern types
                break;
        }
    }
    
    // Determine optimization strategy
    if (result->integer_pattern_count > 0 && result->integer_pattern_count == result->pattern_count) {
        // All integer patterns - check for jump table viability
        result->dense_range_size = (size_t)(result->max_value - result->min_value + 1);
        
        // Jump table is efficient if range is dense (at least 75% filled)
        if (result->dense_range_size <= 256 && 
            result->pattern_count >= (result->dense_range_size * 3 / 4)) {
            result->suitable_for_jump_table = true;
        }
        // Binary search for larger sparse integer sets
        else if (result->pattern_count >= 8) {
            result->suitable_for_binary_search = true;
        }
    }
    
    return result;
}

// =============================================================================
// JUMP TABLE GENERATION
// =============================================================================

static bool generate_optimized_jump_table(FFIAssemblyGenerator *generator, 
                                        PatternMatchContext *context,
                                        MatchAnalysisResult *analysis) {
    if (!generator || !context || !analysis || !analysis->suitable_for_jump_table) {
        return false;
    }
    
    emit_comment(generator, "Optimized jump table for dense integer match");
    
    // Allocate jump table in data section
    size_t table_size = analysis->dense_range_size;
    char *jump_table_label = generate_unique_label(generator, "jump_table");
    
    // Generate bounds check
    Register match_reg = context->match_value_reg;
    Register temp_reg = register_allocate(generator->base_generator->register_allocator, true); // Prefer caller-saved
    if (temp_reg == ASTHRA_REG_NONE) {
        // Handle register allocation failure - fallback to a fixed register
        temp_reg = ASTHRA_REG_RAX;
    }
    
    // Check if value is in range [min_value, max_value]
    if (analysis->min_value != 0) {
        // Subtract min_value to normalize range to [0, range_size)
        // TODO: Implement instruction emission with proper operands
        (void)temp_reg;
        (void)analysis;
    } else {
        // Copy value to temp register
        emit_instruction(generator, INST_MOV, 2,
                        create_register_operand(temp_reg),
                        create_register_operand(match_reg));
    }
    
    // Compare with range size
    emit_instruction(generator, INST_CMP, 2,
                    create_register_operand(temp_reg),
                    create_immediate_operand((int64_t)table_size));
    
    // Jump to default case if out of bounds
    char *default_label = generate_unique_label(generator, "match_default");
    emit_instruction(generator, INST_JAE, 1, create_label_operand(default_label));
    
    // Load jump table base address
    Register table_base_reg = register_allocate(generator->base_generator->register_allocator, false); // Prefer callee-saved
    if (table_base_reg == ASTHRA_REG_NONE) {
        // Handle register allocation failure - fallback to a fixed register
        table_base_reg = ASTHRA_REG_RBX;
    }
    
    emit_instruction(generator, INST_LEA, 2,
                    create_register_operand(table_base_reg),
                    create_label_operand(jump_table_label));
    
    // Compute jump table entry address: base + index * 8
    emit_instruction(generator, INST_SHL, 2,
                    create_register_operand(temp_reg),
                    create_immediate_operand(3)); // Multiply by 8 for 64-bit addresses
    
    emit_instruction(generator, INST_ADD, 2,
                    create_register_operand(temp_reg),
                    create_register_operand(table_base_reg));
    
    // Jump through table
    emit_instruction(generator, INST_JMP, 1,
                    create_memory_operand(temp_reg, ASTHRA_REG_NONE, 1, 0));
    
    // Generate jump table in data section
    // TODO: Set data section flag when implemented
    emit_label(generator, jump_table_label);
    
    // Fill jump table entries
    for (size_t i = 0; i < table_size; i++) {
        int64_t value = analysis->min_value + (int64_t)i;
        char *target_label = default_label;
        
        // Find matching pattern for this value
        for (size_t j = 0; j < analysis->pattern_count; j++) {
            if (analysis->patterns[j].type == MATCH_PATTERN_INTEGER &&
                analysis->patterns[j].data.integer_value == value) {
                target_label = context->arms[j].body_label;
                break;
            }
        }
        
        // Emit table entry (8-byte address)
        emit_data_quad(generator, target_label);
    }
    
    // TODO: Reset data section flag when implemented
    
    // Generate default label
    emit_label(generator, default_label);
    
    // Cleanup registers
    if (temp_reg != ASTHRA_REG_NONE && temp_reg != ASTHRA_REG_RAX) {
        register_free(generator->base_generator->register_allocator, temp_reg);
    }
    if (table_base_reg != ASTHRA_REG_NONE && table_base_reg != ASTHRA_REG_RBX) {
        register_free(generator->base_generator->register_allocator, table_base_reg);
    }
    
    free(jump_table_label);
    free(default_label);
    
    // TODO: Increment jump tables generated counter when implemented
    (void)generator; // Suppress unused parameter warning
    
    return true;
}

// =============================================================================
// BINARY SEARCH GENERATION
// =============================================================================

typedef struct BinarySearchNode {
    int64_t value;
    size_t pattern_index;
} BinarySearchNode;

static int compare_search_nodes(const void *a, const void *b) {
    const BinarySearchNode *node_a = (const BinarySearchNode *)a;
    const BinarySearchNode *node_b = (const BinarySearchNode *)b;
    return (node_a->value > node_b->value) - (node_a->value < node_b->value);
}

static bool generate_binary_search_recursive(FFIAssemblyGenerator *generator,
                                           BinarySearchNode *nodes,
                                           size_t start, size_t end,
                                           Register value_reg,
                                           PatternMatchContext *context) {
    if (start > end) return false;
    
    if (start == end) {
        // Single element - direct comparison
        emit_instruction(generator, INST_CMP, 2,
                        create_register_operand(value_reg),
                        create_immediate_operand(nodes[start].value));
        emit_instruction(generator, INST_JE, 1,
                        create_label_operand(context->arms[nodes[start].pattern_index].body_label));
        return true;
    }
    
    // Find middle element
    size_t mid = start + (end - start) / 2;
    
    // Compare with middle value
    emit_instruction(generator, INST_CMP, 2,
                    create_register_operand(value_reg),
                    create_immediate_operand(nodes[mid].value));
    
    // If equal, jump to corresponding arm
    emit_instruction(generator, INST_JE, 1,
                    create_label_operand(context->arms[nodes[mid].pattern_index].body_label));
    
    // If less, search left half
    char *left_label = generate_unique_label(generator, "bsearch_left");
    emit_instruction(generator, INST_JL, 1, create_label_operand(left_label));
    
    // Otherwise, search right half
    if (mid < end) {
        generate_binary_search_recursive(generator, nodes, mid + 1, end, value_reg, context);
    }
    
    // Generate left half search
    emit_label(generator, left_label);
    free(left_label);
    
    if (mid > start) {
        generate_binary_search_recursive(generator, nodes, start, mid - 1, value_reg, context);
    }
    
    return true;
}

static bool generate_optimized_binary_search(FFIAssemblyGenerator *generator,
                                           PatternMatchContext *context,
                                           MatchAnalysisResult *analysis) {
    if (!generator || !context || !analysis || !analysis->suitable_for_binary_search) {
        return false;
    }
    
    emit_comment(generator, "Binary search for sparse integer match");
    
    // Build sorted array of values
    BinarySearchNode *nodes = calloc(analysis->integer_pattern_count, sizeof(BinarySearchNode));
    if (!nodes) return false;
    
    size_t node_count = 0;
    for (size_t i = 0; i < analysis->pattern_count; i++) {
        if (analysis->patterns[i].type == MATCH_PATTERN_INTEGER) {
            nodes[node_count].value = analysis->patterns[i].data.integer_value;
            nodes[node_count].pattern_index = i;
            node_count++;
        }
    }
    
    // Sort nodes by value
    qsort(nodes, node_count, sizeof(BinarySearchNode), compare_search_nodes);
    
    // Generate binary search tree
    bool success = generate_binary_search_recursive(generator, nodes, 0, node_count - 1,
                                                  context->match_value_reg, context);
    
    free(nodes);
    
    if (success) {
        // TODO: Increment binary searches generated counter when implemented
        (void)generator; // Suppress unused parameter warning
    }
    
    return success;
}

// =============================================================================
// MAIN OPTIMIZATION ENTRY POINT
// =============================================================================

bool optimize_match_expression(Optimizer *optimizer, ASTNode *match_stmt,
                              struct FFIAssemblyGenerator *generator,
                              struct PatternMatchContext *context) {
    if (!optimizer || !match_stmt || !generator || !context) {
        return false;
    }
    
    // Analyze match patterns
    MatchAnalysisResult *analysis = analyze_match_patterns(match_stmt);
    if (!analysis) {
        return false;
    }
    
    bool optimized = false;
    
    // Apply appropriate optimization
    if (analysis->suitable_for_jump_table) {
        optimized = generate_optimized_jump_table(generator, context, analysis);
        if (optimized) {
            atomic_fetch_add(&optimizer->stats.match_jump_tables_created, 1);
        }
    } else if (analysis->suitable_for_binary_search) {
        optimized = generate_optimized_binary_search(generator, context, analysis);
        if (optimized) {
            atomic_fetch_add(&optimizer->stats.match_binary_searches_created, 1);
        }
    }
    
    // Cleanup
    free(analysis->patterns);
    free(analysis);
    
    return optimized;
}