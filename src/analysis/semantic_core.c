/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Core Module
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analyzer module coordination
 */

#include "semantic_core.h"

// This file serves as a coordination point for the semantic analyzer
// The implementation is split into separate compilation units:
// - semantic_lifecycle.c: Creation, destruction, and reset of the analyzer
// - semantic_main_analysis.c: Main analysis functions for programs and declarations
// - semantic_statement_analysis.c: Statement analysis functions
// - semantic_expression_analysis.c: Expression analysis functions
//
// Each of these files is compiled separately and linked together.