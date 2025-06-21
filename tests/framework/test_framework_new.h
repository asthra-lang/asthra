/**
 * Asthra Programming Language
 * Test Framework - Main Header (Modernized Split Version)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Phase 5.1: Test Framework Modernization
 * - Split into logical modules for better maintainability
 * - All C17 features preserved from original implementation
 * - Thread-safe test statistics with atomic operations
 * - Type-safe assertion implementations for _Generic dispatch
 * - Enhanced test execution with timing and error handling
 * - JSON and text output formats
 */

#ifndef ASTHRA_TEST_FRAMEWORK_NEW_H
#define ASTHRA_TEST_FRAMEWORK_NEW_H

// Core types and common definitions
#include "test_types.h"

// Statistics management
#include "test_statistics.h"

// Test context management
#include "test_context.h"

// Test suite configuration and execution
#include "test_suite.h"

// Assertion functions and macros
#include "test_assertions.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// MODULE OVERVIEW
// =============================================================================

/*
 * The test framework has been split into the following modules:
 *
 * 1. test_types.h/.c      - Common types, enums, and timing utilities
 * 2. test_statistics.h/.c - Thread-safe statistics management
 * 3. test_context.h/.c    - Test execution context management
 * 4. test_suite.h/.c      - Test suite configuration and execution
 * 5. test_assertions.h/.c - Type-safe assertion implementations
 *
 * This main header includes all modules for backward compatibility
 * while allowing individual modules to be used independently.
 */

// =============================================================================
// COMPATIBILITY NOTES
// =============================================================================

/*
 * All functionality from the original test_framework.h is preserved:
 * - C17 _Static_assert validations
 * - Atomic operations for thread-safe statistics
 * - _Generic macros for polymorphic test operations
 * - All assertion types and convenience macros
 * - Test execution and configuration functions
 *
 * Usage remains identical to the original framework.
 */

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_TEST_FRAMEWORK_NEW_H
