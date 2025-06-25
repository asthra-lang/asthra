/**
 * Asthra Programming Language
 * If-Let Statement Testing - Integration and Performance Tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 4.4: Integration tests and Phase 4.5: Performance and edge case tests
 */

#ifndef IF_LET_TEST_INTEGRATION_H
#define IF_LET_TEST_INTEGRATION_H

#include "if_let_test_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// PHASE 4.4: INTEGRATION TESTS
// =============================================================================

/**
 * Test: End-to-end if-let compilation (Mock)
 * Tests if-let statements within complete function contexts
 * @return true if test passes, false otherwise
 */
bool test_end_to_end_compilation_mock(void);

/**
 * Test: Grammar compliance validation
 * Verifies that if-let statements conform to the PEG grammar
 * @return true if test passes, false otherwise
 */
bool test_grammar_compliance(void);

// =============================================================================
// PHASE 4.5: PERFORMANCE AND EDGE CASE TESTS
// =============================================================================

/**
 * Test: Performance with complex nested patterns
 * Tests parser performance with deeply nested pattern structures
 * @return true if test passes, false otherwise
 */
bool test_complex_nested_patterns(void);

/**
 * Test: Memory management validation
 * Ensures no memory leaks in if-let parsing
 * @return true if test passes, false otherwise
 */
bool test_memory_management(void);

/**
 * Run all Phase 4.4 integration tests
 * @return true if all tests pass, false otherwise
 */
bool run_integration_tests(void);

/**
 * Run all Phase 4.5 performance and edge case tests
 * @return true if all tests pass, false otherwise
 */
bool run_performance_tests(void);

#ifdef __cplusplus
}
#endif

#endif // IF_LET_TEST_INTEGRATION_H
