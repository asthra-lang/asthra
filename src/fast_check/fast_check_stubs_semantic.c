// This file provides stub implementations for semantic analyzer bridge functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "../parser/ast_types.h"
#include "fast_check_engine.h"
#include <stdbool.h>
#include <stdlib.h>

// Include the actual header for proper type definitions
#include "../analysis/semantic_analyzer_core.h"
#include "fast_check_internal.h"

// =============================================================================
// SEMANTIC ANALYZER BRIDGE FUNCTIONS
// =============================================================================

SemanticAnalyzer *semantic_analyzer_create_internal(void) {
    // Call the actual function directly with proper types
    return semantic_analyzer_create();
}

void semantic_analyzer_destroy_internal(SemanticAnalyzer *analyzer) {
    // Call the actual function directly with proper types
    semantic_analyzer_destroy(analyzer);
}

void semantic_analyzer_reset_internal(SemanticAnalyzer *analyzer) {
    // For now, do nothing - reset functionality would need to be implemented
    (void)analyzer;
}

bool semantic_analyze_program_internal(SemanticAnalyzer *analyzer, ASTNode *program) {
    // For now, return true for successful parsing
    // Real implementation would call the actual semantic analysis
    (void)analyzer;
    return (program != NULL);
}

size_t semantic_get_error_count_internal(SemanticAnalyzer *analyzer) {
    // For now, return 0 - real implementation would get actual error count
    (void)analyzer;
    return 0;
}

#else // FAST_CHECK_USE_STUBS

// Dummy declaration to avoid empty translation unit warning
static int fast_check_stubs_semantic_dummy = 0;

#endif // FAST_CHECK_USE_STUBS