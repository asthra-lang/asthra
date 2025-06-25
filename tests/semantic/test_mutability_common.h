/**
 * Asthra Programming Language
 * Common declarations for mutability tests
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef TEST_MUTABILITY_COMMON_H
#define TEST_MUTABILITY_COMMON_H

#include <stdbool.h>

// Forward declarations
struct SemanticAnalyzer;
typedef struct SemanticAnalyzer SemanticAnalyzer;

// Test helper function declarations
bool test_mutability_success(const char *source, const char *test_name);
bool test_mutability_error(const char *source, int expected_error, const char *test_name);

#endif // TEST_MUTABILITY_COMMON_H