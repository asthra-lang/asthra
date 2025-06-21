// This file provides stub implementations for engine management functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "fast_check_engine.h"
#include "fast_check_command.h"
#include "fast_semantic_cache.h"
#include "dependency_tracker.h"
#include <stdlib.h>
#include <pthread.h>

// Include the actual header for proper type definitions
#include "../analysis/semantic_analyzer_core.h"
#include "fast_check_internal.h"

// =============================================================================
// ENGINE MANAGEMENT
// =============================================================================

FastCheckEngine* fast_check_engine_create(void) {
    FastCheckEngine *engine = calloc(1, sizeof(FastCheckEngine));
    if (!engine) return NULL;
    
    engine->config = fast_check_config_create_default();
    engine->dependency_graph = dependency_graph_create();
    engine->semantic_cache = fast_semantic_cache_create(256);
    
    // Create real semantic analyzer (using void* to match header)
    engine->analyzer = (void*)semantic_analyzer_create_internal();
    if (!engine->analyzer) {
        fast_check_engine_destroy(engine);
        return NULL;
    }
    
    // Initialize mutex for thread safety
    if (pthread_mutex_init(&engine->engine_lock, NULL) != 0) {
        fast_check_engine_destroy(engine);
        return NULL;
    }
    
    return engine;
}

void fast_check_engine_destroy(FastCheckEngine *engine) {
    if (!engine) return;
    
    if (engine->config) fast_check_config_destroy(engine->config);
    if (engine->dependency_graph) dependency_graph_destroy(engine->dependency_graph);
    if (engine->semantic_cache) fast_semantic_cache_destroy(engine->semantic_cache);
    
    // Destroy real semantic analyzer
    if (engine->analyzer) {
        semantic_analyzer_destroy_internal((SemanticAnalyzer*)engine->analyzer);
    }
    
    // Clean up mutex
    pthread_mutex_destroy(&engine->engine_lock);
    
    free(engine);
}

FastCheckEngineStats fast_check_engine_get_stats(FastCheckEngine *engine) {
    if (!engine) {
        FastCheckEngineStats empty = {0};
        return empty;
    }
    return engine->stats;
}

bool fast_check_invalidate_cache(FastCheckEngine *engine, const char *filepath) {
    // Always succeed for stub
    return true;
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_engine_dummy = 0;

#endif // FAST_CHECK_USE_STUBS