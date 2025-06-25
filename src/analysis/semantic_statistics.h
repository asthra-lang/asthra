/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Statistics and Profiling Module Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Statistics collection and profiling for semantic analysis
 */

#ifndef ASTHRA_SEMANTIC_STATISTICS_H
#define ASTHRA_SEMANTIC_STATISTICS_H

#include "semantic_analyzer_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// STATISTICS MANAGEMENT
// =============================================================================

/**
 * Get current analysis statistics
 */
SemanticStatistics semantic_get_statistics(const SemanticAnalyzer *analyzer);

/**
 * Reset statistics counters
 */
void semantic_reset_statistics(SemanticAnalyzer *analyzer);

/**
 * Print statistics to stdout
 */
void semantic_print_statistics(const SemanticAnalyzer *analyzer);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_STATISTICS_H
