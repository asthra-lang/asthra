/**
 * Real Program Validation Tests - Common Header
 * 
 * Phase 3: Real Testing Infrastructure Implementation
 * Day 2: Complete Program Test Cases
 * 
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef REAL_PROGRAM_VALIDATION_COMMON_H
#define REAL_PROGRAM_VALIDATION_COMMON_H

#include "real_program_test_framework.h"
#include "real_program_test_suite.h"
#include "real_program_test_false_positive.h"
#include "real_program_test_generators.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Valid program test suite creation
RealProgramTestSuite* create_valid_program_test_suite(void);

// Invalid program test suite creation
RealProgramTestSuite* create_invalid_program_test_suite(void);

// Performance test suite creation
RealProgramTestSuite* create_performance_test_suite(void);

// Feature validation functions
bool validate_multiline_strings_feature(const TestSuiteConfig* config);
bool validate_type_system_feature(const TestSuiteConfig* config);
bool validate_function_declarations_feature(const TestSuiteConfig* config);
bool validate_if_let_feature(const TestSuiteConfig* config);

// Main execution function
int run_comprehensive_validation(bool verbose);

#endif // REAL_PROGRAM_VALIDATION_COMMON_H