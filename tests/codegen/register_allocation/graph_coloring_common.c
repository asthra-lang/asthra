/**
 * Asthra Programming Language Compiler
 * Graph Coloring Register Allocation Testing - Common Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file implements common test fixture and utility functions
 * for graph coloring register allocation tests.
 */

#include "graph_coloring_common.h"

// =============================================================================
// TEST FIXTURE IMPLEMENTATION
// =============================================================================

/**
 * Setup test fixture with a register allocator and interference graph
 */
GraphColoringTestFixture *setup_graph_coloring_fixture(void) {
    GraphColoringTestFixture *fixture = calloc(1, sizeof(GraphColoringTestFixture));
    if (!fixture)
        return NULL;

    fixture->num_virtual_registers = 16;
    fixture->num_physical_registers = 8;

    fixture->allocator = register_allocator_create();
    if (!fixture->allocator) {
        free(fixture);
        return NULL;
    }

    fixture->interference_graph = interference_graph_create();
    if (!fixture->interference_graph) {
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->instruction_buffer = instruction_buffer_create(1024);
    if (!fixture->instruction_buffer) {
        interference_graph_destroy(fixture->interference_graph);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->label_manager = label_manager_create(16);
    if (!fixture->label_manager) {
        instruction_buffer_destroy(fixture->instruction_buffer);
        interference_graph_destroy(fixture->interference_graph);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    fixture->liveness = liveness_analysis_create();
    if (!fixture->liveness) {
        label_manager_destroy(fixture->label_manager);
        instruction_buffer_destroy(fixture->instruction_buffer);
        interference_graph_destroy(fixture->interference_graph);
        register_allocator_destroy(fixture->allocator);
        free(fixture);
        return NULL;
    }

    return fixture;
}

/**
 * Cleanup test fixture
 */
void cleanup_graph_coloring_fixture(GraphColoringTestFixture *fixture) {
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
    if (fixture->interference_graph) {
        interference_graph_destroy(fixture->interference_graph);
    }
    if (fixture->allocator) {
        register_allocator_destroy(fixture->allocator);
    }
    free(fixture);
}
