/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Utilities Module - Include Aggregator
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * This file has been split into focused modules for better maintainability.
 * All implementation is now distributed across the following specialized files:
 *
 * - type_checking.h/c: Type compatibility checking and casting utilities
 * - type_inference.h/c: Expression type inference and analysis
 * - symbol_utilities.h/c: Symbol resolution and declaration utilities
 * - semantic_helpers.h/c: Type promotion, node analysis dispatch, and helper functions
 *
 * This include aggregator maintains backward compatibility while providing
 * a cleaner, more modular architecture that supports easier testing and
 * maintenance of the semantic analysis subsystem.
 */

// Include all split modules to maintain API compatibility
#include "semantic_helpers.h"
#include "symbol_utilities.h"
#include "type_checking.h"
#include "type_inference.h"