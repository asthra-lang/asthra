/**
 * Asthra Programming Language Compiler
 * Graph Coloring Register Allocation Testing - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file provides common test fixture and utility definitions
 * for graph coloring register allocation tests.
 */

#ifndef ASTHRA_GRAPH_COLORING_COMMON_H
#define ASTHRA_GRAPH_COLORING_COMMON_H

#include "../framework/compiler_test_utils.h"
#include "../framework/test_framework.h"
#include "ast.h"
#include "code_generator_core.h"
#include "code_generator_instructions.h"
#include "code_generator_labels.h"
#include "register_allocator.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// TEST FIXTURE AND UTILITIES
// =============================================================================

/**
 * Test fixture for graph coloring allocation testing
 */
typedef struct {
    RegisterAllocator *allocator;
    InterferenceGraph *interference_graph;
    InstructionBuffer *instruction_buffer;
    LabelManager *label_manager;
    LivenessAnalysis *liveness;
    size_t num_virtual_registers;
    size_t num_physical_registers;
} GraphColoringTestFixture;

/**
 * Setup test fixture with a register allocator and interference graph
 */
GraphColoringTestFixture *setup_graph_coloring_fixture(void);

/**
 * Cleanup test fixture
 */
void cleanup_graph_coloring_fixture(GraphColoringTestFixture *fixture);

#endif // ASTHRA_GRAPH_COLORING_COMMON_H
