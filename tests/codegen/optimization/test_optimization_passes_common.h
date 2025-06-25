/**
 * Asthra Programming Language Compiler
 * Optimization Passes Testing - Common Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef TEST_OPTIMIZATION_PASSES_COMMON_H
#define TEST_OPTIMIZATION_PASSES_COMMON_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// MINIMAL TEST FRAMEWORK
// =============================================================================

#define DEFINE_TEST(name) static bool name(void)

#define RUN_TEST_SUITE(suite_name, ...)                                                            \
    do {                                                                                           \
        printf("Running %s test suite...\n", #suite_name);                                         \
        typedef bool (*test_func_t)(void);                                                         \
        test_func_t tests[] = {__VA_ARGS__};                                                       \
        int total = sizeof(tests) / sizeof(tests[0]);                                              \
        int passed = 0;                                                                            \
        for (int i = 0; i < total; i++) {                                                          \
            if (tests[i]()) {                                                                      \
                passed++;                                                                          \
                printf("  ✓ Test %d passed\n", i + 1);                                             \
            } else {                                                                               \
                printf("  ✗ Test %d failed\n", i + 1);                                             \
            }                                                                                      \
        }                                                                                          \
        printf("%s: %d/%d tests passed\n", #suite_name, passed, total);                            \
        return passed == total;                                                                    \
    } while (0)

// =============================================================================
// OPTIMIZATION TYPES AND STRUCTURES
// =============================================================================

typedef enum {
    OPTIMIZATION_LEVEL_O0,
    OPTIMIZATION_LEVEL_O1,
    OPTIMIZATION_LEVEL_O2,
    OPTIMIZATION_LEVEL_O3
} OptimizationLevel;

typedef enum {
    OP_MOV,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_CMP,
    OP_JMP,
    OP_JGT,
    OP_JGE,
    OP_RET,
    OP_NOP
} OpCode;

typedef enum { OPERAND_REGISTER, OPERAND_IMMEDIATE } OperandType;

typedef struct {
    OperandType type;
    union {
        int reg;
        int value;
    };
} Operand;

typedef struct {
    OpCode opcode;
    int dst_reg;
    Operand operand1;
    Operand operand2;
    bool is_used;
    bool is_dead;
    bool is_constant;
} Instruction;

typedef struct Optimizer Optimizer;
typedef struct InstructionBuffer InstructionBuffer;
typedef struct OptimizationContext OptimizationContext;

typedef struct {
    bool success;
    int optimizations_applied;
    int constant_folds;
    int dead_code_eliminated;
    int common_subexpressions;
    int strength_reductions;
    int cse_eliminations; // Common subexpression eliminations
    int loop_invariant_motions;
    int peephole_optimizations;
} OptimizationResult;

struct InstructionBuffer {
    Instruction *instructions;
    size_t count;
    size_t capacity;
};

struct Optimizer {
    OptimizationLevel level;
    bool enabled;
};

struct OptimizationContext {
    bool constant_folding_enabled;
    bool dead_code_elimination_enabled;
    bool common_subexpression_elimination_enabled;
    bool strength_reduction_enabled;
};

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Core infrastructure functions - test versions to avoid conflicts
Optimizer *test_optimizer_create(OptimizationLevel level);
void test_optimizer_destroy(Optimizer *opt);
InstructionBuffer *test_instruction_buffer_create(size_t capacity);
void test_instruction_buffer_destroy(InstructionBuffer *buffer);
bool test_instruction_buffer_add(InstructionBuffer *buffer, const Instruction *instr);
size_t test_instruction_buffer_size(InstructionBuffer *buffer);
OptimizationContext *test_optimization_context_create(void);
void test_optimization_context_destroy(OptimizationContext *ctx);

// Optimization pass functions - test versions
OptimizationResult test_optimizer_constant_folding(Optimizer *opt, InstructionBuffer *buffer);
OptimizationResult test_optimizer_dead_code_elimination(Optimizer *opt, InstructionBuffer *buffer);
OptimizationResult test_optimizer_common_subexpression_elimination(Optimizer *opt,
                                                                   InstructionBuffer *buffer);
OptimizationResult test_optimizer_strength_reduction(Optimizer *opt, InstructionBuffer *buffer);
OptimizationResult test_optimizer_peephole_optimization(Optimizer *opt, InstructionBuffer *buffer);

#endif // TEST_OPTIMIZATION_PASSES_COMMON_H
