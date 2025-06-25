// This file provides stub implementations for dependency graph functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "dependency_tracker.h"
#include <stdbool.h>
#include <stdlib.h>

// =============================================================================
// DEPENDENCY GRAPH
// =============================================================================

DependencyGraph *dependency_graph_create(void) {
    return calloc(1, sizeof(DependencyGraph));
}

void dependency_graph_destroy(DependencyGraph *graph) {
    free(graph);
}

bool dependency_graph_add_file(DependencyGraph *graph, const char *filepath) {
    return graph != NULL && filepath != NULL;
}

bool dependency_graph_add_dependency(DependencyGraph *graph, const char *from, const char *to) {
    return graph != NULL && from != NULL && to != NULL;
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_dependency_dummy = 0;

#endif // FAST_CHECK_USE_STUBS