#ifndef BENCHMARK_TEST_CASES_H
#define BENCHMARK_TEST_CASES_H

#include "test_framework.h"

// =============================================================================
// TEST CASE FUNCTION DECLARATIONS
// =============================================================================

/**
 * Test benchmark timer accuracy
 */
AsthraTestResult test_benchmark_timer_accuracy(AsthraTestContext *context);

/**
 * Test benchmark configuration validation
 */
AsthraTestResult test_benchmark_configuration(AsthraTestContext *context);

/**
 * Test simple benchmark execution
 */
AsthraTestResult test_simple_benchmark_execution(AsthraTestContext *context);

/**
 * Test benchmark with setup and teardown
 */
AsthraTestResult test_benchmark_with_setup_teardown(AsthraTestContext *context);

/**
 * Test benchmark suite execution
 */
AsthraTestResult test_benchmark_suite_execution(AsthraTestContext *context);

/**
 * Test statistical analysis calculations
 */
AsthraTestResult test_statistical_analysis(AsthraTestContext *context);

#endif // BENCHMARK_TEST_CASES_H
