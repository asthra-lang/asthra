// This file provides stub implementations for semantic analyzer bridge functions
// when the actual fast_check implementation is not available.
#ifdef FAST_CHECK_USE_STUBS

#include "fast_check_engine.h"
#include "../parser/ast_types.h"
#include <stdlib.h>
#include <stdbool.h>

// Forward declarations to avoid header conflicts
typedef struct SemanticAnalyzer SemanticAnalyzer;

// External functions from semantic analyzer
extern void* semantic_analyzer_create(void);
extern void semantic_analyzer_destroy(void* analyzer);

// =============================================================================
// SEMANTIC ANALYZER BRIDGE FUNCTIONS
// =============================================================================

SemanticAnalyzer *semantic_analyzer_create_internal(void) {
    // Call the actual function via void* return to avoid type conflicts
    void* analyzer = semantic_analyzer_create();
    return (SemanticAnalyzer*)analyzer;
}

void semantic_analyzer_destroy_internal(SemanticAnalyzer *analyzer) {
    // Call the actual function
    semantic_analyzer_destroy((void*)analyzer);
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