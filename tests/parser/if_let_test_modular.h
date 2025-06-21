/**
 * Asthra Programming Language
 * If-Let Statement Testing - Modular Interface
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Umbrella header providing 100% backward compatibility with the original
 * test_if_let_phase4_comprehensive.c interface while using modular architecture
 */

#ifndef IF_LET_TEST_MODULAR_H
#define IF_LET_TEST_MODULAR_H

// Include all modular components
#include "if_let_test_common.h"
#include "if_let_test_parser.h"
#include "if_let_test_semantic.h"
#include "if_let_test_codegen.h"
#include "if_let_test_integration.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// BACKWARD COMPATIBILITY INTERFACE
// =============================================================================

/**
 * Run all if-let comprehensive tests
 * Provides the same interface as the original comprehensive test file
 * @return 0 if all tests pass, 1 otherwise
 */
int run_if_let_comprehensive_tests(void);

/**
 * Main test runner function
 * Equivalent to the original main() function
 * @return 0 if all tests pass, 1 otherwise
 */
int if_let_test_main(void);

#ifdef __cplusplus
}
#endif

#endif // IF_LET_TEST_MODULAR_H 
