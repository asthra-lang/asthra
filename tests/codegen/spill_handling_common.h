/**
 * Asthra Programming Language Compiler
 * Register Spill Handling Testing - Common Definitions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file provides common test fixture and utility definitions
 * for register spill handling tests.
 */

#ifndef ASTHRA_SPILL_HANDLING_COMMON_H
#define ASTHRA_SPILL_HANDLING_COMMON_H

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
 * Test fixture for spill handling testing
 */
typedef struct {
    RegisterAllocator *allocator;
    SpillManager *spill_manager;
    InstructionBuffer *instruction_buffer;
    LabelManager *label_manager;
    LivenessAnalysis *liveness;
    size_t num_virtual_registers;
    size_t num_physical_registers;
    size_t stack_frame_size;
} SpillHandlingTestFixture;

/**
 * Setup test fixture with spill handling components
 */
SpillHandlingTestFixture *setup_spill_handling_fixture(void);

/**
 * Cleanup test fixture
 */
void cleanup_spill_handling_fixture(SpillHandlingTestFixture *fixture);

#endif // ASTHRA_SPILL_HANDLING_COMMON_H
