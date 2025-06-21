/**
 * Asthra Programming Language Compiler
 * Register Allocator Header
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_REGISTER_ALLOCATOR_H
#define ASTHRA_REGISTER_ALLOCATOR_H

#include "code_generator_types.h"
#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations for register allocation components
typedef struct SpillManager SpillManager;
typedef struct LivenessAnalysis LivenessAnalysis;
typedef struct InterferenceGraph InterferenceGraph;

// Register allocator functions
Register allocate_register(CodeGenerator *generator);
void release_register(CodeGenerator *generator, Register reg);
bool is_register_available(CodeGenerator *generator, Register reg);
void mark_register_used(CodeGenerator *generator, Register reg);
void mark_register_free(CodeGenerator *generator, Register reg);

// Spill management functions
SpillManager* spill_manager_create(void);
void spill_manager_destroy(SpillManager* manager);

// Liveness analysis functions
LivenessAnalysis* liveness_analysis_create(void);
void liveness_analysis_destroy(LivenessAnalysis* analysis);

// Interference graph functions
InterferenceGraph* interference_graph_create(void);
void interference_graph_destroy(InterferenceGraph* graph);

#ifdef __cplusplus
}
#endif

#endif /* ASTHRA_REGISTER_ALLOCATOR_H */ 
