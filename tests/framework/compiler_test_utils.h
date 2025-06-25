/**
 * Asthra Programming Language
 * Compiler Test Utilities
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Main header that includes all compiler test utility modules
 * This file provides backward compatibility and convenience
 */

#ifndef ASTHRA_COMPILER_TEST_UTILS_H
#define ASTHRA_COMPILER_TEST_UTILS_H

// Include all the modular test utilities
#include "lexer_test_utils.h"
#include "parser_test_utils.h"
#include "performance_test_utils.h"
#include "semantic_test_utils.h"
#include "test_assertions.h"
#include "test_data_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// CONVENIENCE INCLUDES
// =============================================================================
// This header now serves as a convenience include that brings in all the
// specialized test utility modules. Each module can also be included
// individually as needed.

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_COMPILER_TEST_UTILS_H
