/**
 * Asthra Programming Language
 * Test Data Management Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Test data management functions for file I/O and temporary files
 */

#ifndef ASTHRA_TEST_DATA_UTILS_H
#define ASTHRA_TEST_DATA_UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// TEST DATA MANAGEMENT
// =============================================================================

/**
 * Load test source code from file
 * @param filename The file to load
 * @return The file contents (caller must free)
 */
char* load_test_file(const char* filename);

/**
 * Save test output to file
 * @param filename The file to save to
 * @param content The content to save
 * @return true if save succeeded
 */
bool save_test_output(const char* filename, const char* content);

/**
 * Compare two test files for equality
 * @param expected_file The expected output file
 * @param actual_file The actual output file
 * @return true if files are equal
 */
bool compare_test_files(const char* expected_file, const char* actual_file);

/**
 * Create a temporary test file with content
 * @param content The content to write
 * @param suffix File suffix (e.g., ".ast")
 * @return The temporary filename (caller must free)
 */
char* create_temp_test_file(const char* content, const char* suffix);

/**
 * Clean up temporary test files
 * @param filename The temporary file to remove
 */
void cleanup_temp_test_file(const char* filename);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_DATA_UTILS_H
