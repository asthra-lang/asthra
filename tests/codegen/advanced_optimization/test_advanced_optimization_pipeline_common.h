#ifndef TEST_ADVANCED_OPTIMIZATION_PIPELINE_COMMON_H
#define TEST_ADVANCED_OPTIMIZATION_PIPELINE_COMMON_H

#include "../framework/test_framework_minimal.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Optimization type enumeration
typedef enum {
    OPT_DEAD_CODE_ELIMINATION,
    OPT_CONSTANT_FOLDING,
    OPT_COMMON_SUBEXPRESSION,
    OPT_STRENGTH_REDUCTION,
    OPT_LOOP_UNROLLING,
    OPT_PEEPHOLE,
    OPT_INLINING,
    OPT_REGISTER_ALLOCATION,
    OPT_COUNT
} OptimizationType;

// Optimization pass structure
typedef struct {
    OptimizationType type;
    const char *name;
    bool enabled;
    int priority;
    int execution_count;
    double effectiveness; // 0.0 to 1.0
    bool converged;
} MinimalOptimizationPass;

// Instruction representation for optimization
typedef struct {
    uint64_t address;
    const char *instruction;
    bool is_dead;           // For dead code elimination
    bool is_constant;       // For constant folding
    int64_t constant_value; // Constant value if applicable
    bool is_redundant;      // For common subexpression elimination
    int reference_count;    // Reference counting
} MinimalInstruction;

// Loop structure for optimization
typedef struct {
    uint64_t start_address;
    uint64_t end_address;
    int iteration_count;
    bool unrollable;
    bool has_side_effects;
    int instruction_count;
} MinimalLoop;

// Main optimization context
typedef struct {
    MinimalOptimizationPass passes[OPT_COUNT];
    int pass_count;
    MinimalInstruction instructions[128];
    int instruction_count;
    MinimalLoop loops[16];
    int loop_count;
    int total_optimizations;
    double overall_effectiveness;
    bool pipeline_converged;
} MinimalOptimizationContext;

// Common function declarations
void init_optimization_context(MinimalOptimizationContext *ctx);
void add_instruction(MinimalOptimizationContext *ctx, uint64_t addr, const char *instr);
void add_loop(MinimalOptimizationContext *ctx, uint64_t start, uint64_t end, int iterations);
void run_optimization_pass(MinimalOptimizationContext *ctx, OptimizationType type);
bool run_optimization_pipeline(MinimalOptimizationContext *ctx, int max_iterations);

// Test function declarations
AsthraTestResult test_optimization_pipeline_creation(AsthraTestContext *context);
AsthraTestResult test_multi_pass_optimization(AsthraTestContext *context);
AsthraTestResult test_optimization_effectiveness(AsthraTestContext *context);
AsthraTestResult test_optimization_validation(AsthraTestContext *context);

// Test metadata
extern AsthraTestMetadata test_metadata[];

#endif // TEST_ADVANCED_OPTIMIZATION_PIPELINE_COMMON_H
