/**
 * Common Definitions for pub/impl/self Parser Tests
 *
 * This header provides shared type definitions, utilities, and helper functions
 * for the modular pub/impl/self parser test suite.
 *
 * Copyright (c) 2024 Asthra Project
 */

#ifndef TEST_PUB_IMPL_SELF_COMMON_H
#define TEST_PUB_IMPL_SELF_COMMON_H

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test framework
#include "../framework/test_assertions.h"
#include "../framework/test_context.h"
#include "../framework/test_statistics.h"
#include "../framework/test_suite.h"

// Asthra parser components
#include "ast.h"
#include "lexer.h"
#include "parser.h"

// =============================================================================
// SHARED UTILITIES AND HELPERS
// =============================================================================

/**
 * Create a test lexer from source code
 */
Lexer *create_test_lexer(const char *source);

/**
 * Create a test parser from source code
 */
Parser *create_test_parser(const char *source);

/**
 * Clean up parser and associated lexer
 */
void cleanup_parser(Parser *parser);

/**
 * Test metadata for pub/impl/self tests
 */
extern AsthraTestMetadata pub_impl_self_base_metadata;

#endif // TEST_PUB_IMPL_SELF_COMMON_H
