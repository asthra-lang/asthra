/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Core Module
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core semantic analyzer module that includes lifecycle and main analysis functions
 */

#include "semantic_core.h"

// Include the split functionality
#include "semantic_lifecycle.c"
#include "semantic_main_analysis.c"

// This file now serves as a coordination point for the semantic analyzer
// The actual implementation is split into:
// - semantic_lifecycle.c: Creation, destruction, and reset of the analyzer
// - semantic_main_analysis.c: Main analysis functions for programs, declarations, statements, and expressions