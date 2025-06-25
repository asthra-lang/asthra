/**
 * Asthra Programming Language Compiler
 * Register Spill Handling Testing - Common Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements common test fixture and utility functions
 * for register spill handling tests.
 */

#include "spill_handling_common.h"

// =============================================================================
// TEST FIXTURE IMPLEMENTATION
// =============================================================================

/**
 * Setup test fixture with spill handling components
 */
SpillHandlingTestFixture *setup_spill_handling_fixture(void) {
    SpillHandlingTestFixture *fixture = calloc(1, sizeof(SpillHandlingTestFixture));
    if (!fixture)
        return NULL;

    fixture->num_virtual_registers = 16;
    fixture->num_physical_registers = 4; // Limited registers to force spilling
    fixture->stack_frame_size = 256;

    fixture->allocator = register_allocator_create();
    if (!fixture->allocator) {
        free(fixture);
        return NULL;
    }

    fixture->spill_manager = spill_manager_create();
    if (!fixture->spill_manager) {
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->instruction_buffer = instruction_buffer_create(1024);
    if (!fixture->instruction_buffer) {
        spill_manager_destroy(fixture->spill_manager);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->label_manager = label_manager_create(16);
    if (!fixture->label_manager) {
        instruction_buffer_destroy(fixture->instruction_buffer);
        spill_manager_destroy(fixture->spill_manager);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->liveness = liveness_analysis_create();
    if (!fixture->liveness) {
        label_manager_destroy(fixture->label_manager);
        instruction_buffer_destroy(fixture->instruction_buffer);
        spill_manager_destroy(fixture->spill_manager);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    return fixture;
}

/**
 * Cleanup test fixture
 */
void cleanup_spill_handling_fixture(SpillHandlingTestFixture *fixture) {
    if (!fixture)
        return;

    if (fixture->liveness) {
        liveness_analysis_destroy(fixture->liveness);
    }
    if (fixture->label_manager) {
        label_manager_destroy(fixture->label_manager);
    }
    if (fixture->instruction_buffer) {
        instruction_buffer_destroy(fixture->instruction_buffer);
    }
    if (fixture->spill_manager) {
        spill_manager_destroy(fixture->spill_manager);
    }
    if (fixture->allocator) {
        register_allocator_destroy(fixture->allocator);
    }
    free(fixture);
}
