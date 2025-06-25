/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Expression Analysis Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Expression analysis includes - brings together specialized modules
 */

#include "semantic_expressions.h"
#include "semantic_arrays.h"
#include "semantic_binary_unary.h"
#include "semantic_calls.h"
#include "semantic_expression_utils.h"
#include "semantic_field_access.h"
#include "semantic_literals.h"

// This file now serves as an include aggregator for expression analysis.
// The actual semantic_analyze_expression function is implemented in semantic_core.c
// and calls the specialized functions from the included modules.