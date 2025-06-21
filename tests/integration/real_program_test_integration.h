/**
 * Real Program Test Integration Header
 * 
 * Contains functions for integration testing including end-to-end
 * compilation, semantic analysis integration, and component integration.
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_TEST_INTEGRATION_H
#define REAL_PROGRAM_TEST_INTEGRATION_H

#include <stdbool.h>
#include "real_program_test_utils.h"

// =============================================================================
// INTEGRATION TESTING FUNCTIONS
// =============================================================================

/**
 * Test complete source-to-executable pipeline
 * @param source The source code to compile
 * @param output_name The name for the output executable
 * @param config Test configuration
 * @return true if compilation and execution succeeded
 */
bool test_source_to_executable(const char* source, const char* output_name, const TestSuiteConfig* config);

/**
 * Test parser and semantic analyzer integration
 * @param source The source code to test
 * @param config Test configuration
 * @return true if parser and semantic analysis integration works correctly
 */
bool test_parser_semantic_integration(const char* source, const TestSuiteConfig* config);

/**
 * Check integration between multiple components
 * @param source The source code to test
 * @param components Array of component names to check
 * @param component_count Number of components
 * @param config Test configuration
 * @return true if all components integrate properly
 */
bool check_component_integration(const char* source, const char** components, 
                               size_t component_count, const TestSuiteConfig* config);

#endif // REAL_PROGRAM_TEST_INTEGRATION_H