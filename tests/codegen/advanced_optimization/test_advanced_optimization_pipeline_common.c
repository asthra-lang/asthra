#include "test_advanced_optimization_pipeline_common.h"

// Test metadata
AsthraTestMetadata test_metadata[] = {
    {"test_optimization_pipeline_creation", __FILE__, __LINE__, "Test optimization pipeline setup",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_multi_pass_optimization", __FILE__, __LINE__, "Test multiple optimization passes",
     ASTHRA_TEST_SEVERITY_HIGH, 5000000000ULL, false, NULL},
    {"test_optimization_effectiveness", __FILE__, __LINE__,
     "Test optimization effectiveness metrics", ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false,
     NULL},
    {"test_optimization_validation", __FILE__, __LINE__, "Test optimization correctness validation",
     ASTHRA_TEST_SEVERITY_MEDIUM, 5000000000ULL, false, NULL}};

// Initialize optimization context with default passes
void init_optimization_context(MinimalOptimizationContext *ctx) {
    ctx->pass_count = 0;
    ctx->instruction_count = 0;
    ctx->loop_count = 0;
    ctx->total_optimizations = 0;
    ctx->overall_effectiveness = 0.0;
    ctx->pipeline_converged = false;

    // Initialize passes
    const char *pass_names[] = {
        "Dead Code Elimination", "Constant Folding",   "Common Subexpression Elimination",
        "Strength Reduction",    "Loop Unrolling",     "Peephole Optimization",
        "Function Inlining",     "Register Allocation"};

    for (int i = 0; i < OPT_COUNT; i++) {
        ctx->passes[i] = (MinimalOptimizationPass){.type = i,
                                                   .name = pass_names[i],
                                                   .enabled = true,
                                                   .priority = i + 1,
                                                   .execution_count = 0,
                                                   .effectiveness = 0.0,
                                                   .converged = false};
        ctx->pass_count++;
    }

    for (int i = 0; i < 128; i++) {
        ctx->instructions[i] = (MinimalInstruction){0};
    }

    for (int i = 0; i < 16; i++) {
        ctx->loops[i] = (MinimalLoop){0};
    }
}

// Add instruction to optimization context
void add_instruction(MinimalOptimizationContext *ctx, uint64_t addr, const char *instr) {
    if (ctx->instruction_count < 128) {
        MinimalInstruction *inst = &ctx->instructions[ctx->instruction_count++];
        inst->address = addr;
        inst->instruction = instr;
        inst->is_dead = false;
        inst->is_constant = false;
        inst->constant_value = 0;
        inst->is_redundant = false;
        inst->reference_count = 1;
    }
}

// Add loop to optimization context
void add_loop(MinimalOptimizationContext *ctx, uint64_t start, uint64_t end, int iterations) {
    if (ctx->loop_count < 16) {
        MinimalLoop *loop = &ctx->loops[ctx->loop_count++];
        loop->start_address = start;
        loop->end_address = end;
        loop->iteration_count = iterations;
        loop->unrollable = (iterations <= 8 && iterations > 0);
        loop->has_side_effects = false;
        loop->instruction_count = (end - start) / 4; // Assume 4-byte instructions
    }
}

// Run a specific optimization pass
void run_optimization_pass(MinimalOptimizationContext *ctx, OptimizationType type) {
    MinimalOptimizationPass *pass = &ctx->passes[type];
    pass->execution_count++;

    int optimizations_made = 0;
    double effectiveness = 0.0;

    switch (type) {
    case OPT_DEAD_CODE_ELIMINATION: {
        // Mark unused instructions as dead
        for (int i = 0; i < ctx->instruction_count; i++) {
            MinimalInstruction *inst = &ctx->instructions[i];
            if (inst->reference_count == 0 && !inst->is_dead) {
                inst->is_dead = true;
                optimizations_made++;
            }
        }
        effectiveness = (double)optimizations_made / ctx->instruction_count;
        break;
    }

    case OPT_CONSTANT_FOLDING: {
        // Fold constant expressions
        for (int i = 0; i < ctx->instruction_count; i++) {
            MinimalInstruction *inst = &ctx->instructions[i];
            if (strstr(inst->instruction, "add") && strstr(inst->instruction, "$")) {
                // Simulate constant folding for immediate adds
                if (!inst->is_constant) {
                    inst->is_constant = true;
                    inst->constant_value = 42; // Simulated constant
                    optimizations_made++;
                }
            }
        }
        effectiveness = (double)optimizations_made / ctx->instruction_count;
        break;
    }

    case OPT_COMMON_SUBEXPRESSION: {
        // Eliminate redundant expressions
        for (int i = 0; i < ctx->instruction_count; i++) {
            for (int j = i + 1; j < ctx->instruction_count; j++) {
                MinimalInstruction *inst1 = &ctx->instructions[i];
                MinimalInstruction *inst2 = &ctx->instructions[j];

                if (strcmp(inst1->instruction, inst2->instruction) == 0 && !inst2->is_redundant) {
                    inst2->is_redundant = true;
                    optimizations_made++;
                }
            }
        }
        effectiveness = (double)optimizations_made / ctx->instruction_count;
        break;
    }

    case OPT_STRENGTH_REDUCTION: {
        // Replace expensive operations with cheaper ones
        for (int i = 0; i < ctx->instruction_count; i++) {
            MinimalInstruction *inst = &ctx->instructions[i];
            if (strstr(inst->instruction, "mul") && strstr(inst->instruction, "$2")) {
                // Replace mul by 2 with shift
                optimizations_made++;
            }
            if (strstr(inst->instruction, "div") && strstr(inst->instruction, "$4")) {
                // Replace div by 4 with shift
                optimizations_made++;
            }
        }
        effectiveness = (double)optimizations_made / ctx->instruction_count;
        break;
    }

    case OPT_LOOP_UNROLLING: {
        // Unroll small loops
        for (int i = 0; i < ctx->loop_count; i++) {
            MinimalLoop *loop = &ctx->loops[i];
            if (loop->unrollable && loop->iteration_count <= 4) {
                loop->instruction_count *= loop->iteration_count;
                optimizations_made++;
            }
        }
        effectiveness = (ctx->loop_count > 0) ? (double)optimizations_made / ctx->loop_count : 0.0;
        break;
    }

    case OPT_PEEPHOLE: {
        // Local optimizations
        for (int i = 0; i < ctx->instruction_count - 1; i++) {
            MinimalInstruction *inst1 = &ctx->instructions[i];
            MinimalInstruction *inst2 = &ctx->instructions[i + 1];

            // Look for mov-mov redundancy
            if (strstr(inst1->instruction, "mov") && strstr(inst2->instruction, "mov")) {
                optimizations_made++;
            }
        }
        effectiveness = (double)optimizations_made / ctx->instruction_count;
        break;
    }

    case OPT_INLINING: {
        // Inline small functions
        int function_calls = 0;
        for (int i = 0; i < ctx->instruction_count; i++) {
            if (strstr(ctx->instructions[i].instruction, "call")) {
                function_calls++;
                // Simulate inlining decision
                if (function_calls <= 3) {
                    optimizations_made++;
                }
            }
        }
        effectiveness = (function_calls > 0) ? (double)optimizations_made / function_calls : 0.0;
        break;
    }

    case OPT_REGISTER_ALLOCATION: {
        // Optimize register usage
        int register_spills = 0;
        for (int i = 0; i < ctx->instruction_count; i++) {
            if (strstr(ctx->instructions[i].instruction, "spill")) {
                register_spills++;
            }
        }
        // Assume we can eliminate some spills
        optimizations_made = register_spills / 2;
        effectiveness = (register_spills > 0) ? (double)optimizations_made / register_spills : 0.0;
        break;
    }

    case OPT_COUNT:
        // Not a real optimization pass, just enum count
        break;
    }

    pass->effectiveness = effectiveness;
    pass->converged = (optimizations_made == 0);
    ctx->total_optimizations += optimizations_made;
}

// Run the complete optimization pipeline
bool run_optimization_pipeline(MinimalOptimizationContext *ctx, int max_iterations) {
    bool converged = false;
    int iteration = 0;

    while (!converged && iteration < max_iterations) {
        converged = true;

        // Run passes in priority order
        for (int i = 0; i < ctx->pass_count; i++) {
            if (ctx->passes[i].enabled) {
                run_optimization_pass(ctx, ctx->passes[i].type);
                if (!ctx->passes[i].converged) {
                    converged = false;
                }
            }
        }

        iteration++;
    }

    // Calculate overall effectiveness
    double total_effectiveness = 0.0;
    int active_passes = 0;

    for (int i = 0; i < ctx->pass_count; i++) {
        if (ctx->passes[i].enabled && ctx->passes[i].execution_count > 0) {
            total_effectiveness += ctx->passes[i].effectiveness;
            active_passes++;
        }
    }

    ctx->overall_effectiveness = (active_passes > 0) ? total_effectiveness / active_passes : 0.0;
    ctx->pipeline_converged = converged;

    return converged;
}
