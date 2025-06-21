/**
 * Real Program Test Generators Header
 * 
 * Contains functions for generating test programs with various
 * characteristics for comprehensive testing.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_GENERATORS_H
#define REAL_PROGRAM_TEST_GENERATORS_H

#include <stddef.h>

// =============================================================================
// PROGRAM GENERATOR FUNCTIONS
// =============================================================================

/**
 * Generate a large program with many variables
 * @param line_count Number of lines to generate
 * @return Dynamically allocated program string (caller must free)
 */
char* generate_large_program(size_t line_count);

/**
 * Generate a program with complex type usage
 * @return Dynamically allocated program string (caller must free)
 */
char* generate_complex_type_program(void);

/**
 * Generate a program with deep nesting
 * @param depth Nesting depth
 * @return Dynamically allocated program string (caller must free)
 */
char* generate_deep_nesting_program(size_t depth);

/**
 * Generate a program that combines multiple features
 * @param features Array of feature names
 * @param feature_count Number of features
 * @return Dynamically allocated program string (caller must free)
 */
char* generate_feature_combination_program(const char** features, size_t feature_count);

#endif // REAL_PROGRAM_TEST_GENERATORS_H