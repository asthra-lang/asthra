/**
 * Asthra Programming Language Comprehensive Test Suite - Reporting Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * AI feedback and reporting function declarations.
 */

#ifndef ASTHRA_TEST_COMPREHENSIVE_REPORTING_H
#define ASTHRA_TEST_COMPREHENSIVE_REPORTING_H

#include "test_comprehensive_core.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// REPORTING FUNCTION DECLARATIONS
// =============================================================================

// AI feedback recording
void asthra_record_ai_feedback(AsthraExtendedTestContext *ctx,
                              const char *feedback_type,
                              const char *feedback_message);

// Report generation
void asthra_generate_test_report(const char *output_file);
void asthra_generate_detailed_report(const char *output_file, 
                                   const AsthraExtendedTestContext *contexts,
                                   size_t context_count);

// C17 compliance validation
bool asthra_verify_c17_compliance(const char *source_file);
bool asthra_run_static_analysis(const char *source_file);

// Summary output
void asthra_print_comprehensive_summary(void);

// =============================================================================
// BACKWARD COMPATIBILITY ALIASES
// =============================================================================

// Legacy function aliases for backward compatibility
#define asthra_v12_record_ai_feedback asthra_record_ai_feedback
#define asthra_v12_generate_test_report asthra_generate_test_report
#define asthra_v12_generate_detailed_report asthra_generate_detailed_report
#define asthra_v12_verify_c17_compliance asthra_verify_c17_compliance
#define asthra_v12_run_static_analysis asthra_run_static_analysis
#define asthra_v12_print_comprehensive_summary asthra_print_comprehensive_summary

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_COMPREHENSIVE_REPORTING_H 
