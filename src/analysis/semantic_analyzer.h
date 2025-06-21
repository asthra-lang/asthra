/**
 * Asthra Programming Language Compiler
 * Semantic Analysis Phase with C17 Modernization
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Main semantic analyzer header - includes all split components
 * 
 * Phase 2.2 Enhancements:
 * - C17 _Generic for type-safe semantic operations
 * - _Static_assert for type system validation
 * - Atomic operations for thread-safe symbol table
 * - Designated initializers for type descriptors
 */

#ifndef ASTHRA_SEMANTIC_ANALYZER_H
#define ASTHRA_SEMANTIC_ANALYZER_H

// Include all semantic analysis components
#include "semantic_types_defs.h"
#include "semantic_symbols_defs.h"
#include "semantic_errors_defs.h"
#include "semantic_analyzer_core.h"
#include "semantic_macros.h"

// Include additional specialized modules
#include "semantic_core.h"
#include "semantic_builtins.h"
#include "semantic_utilities.h"
#include "semantic_errors.h"
#include "semantic_statistics.h"
#include "semantic_scopes.h"
#include "semantic_declarations.h"
#include "semantic_statements.h"
#include "semantic_expressions.h"
#include "semantic_types.h"
#include "semantic_symbols.h"
#include "semantic_security.h"
#include "semantic_ffi.h"

#ifdef __cplusplus
extern "C" {
#endif

// All functionality is now provided through the included headers
// This file serves as a convenience include for the complete semantic analysis system

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_SEMANTIC_ANALYZER_H 
