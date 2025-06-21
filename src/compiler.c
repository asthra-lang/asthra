/**
 * Asthra Programming Language Compiler
 * Main compiler file - entry point for compiler functionality
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "compiler.h"

// The compiler functionality is now split across multiple modules:
// - compiler_core.c: Core compiler context management and build info
// - argument_list.c: Argument list management with C17 flexible arrays
// - compilation_pipeline.c: Main compilation pipeline and multi-file handling
// - error_handling.c: Error handling and reporting
// - options_validation.c: Options validation and utility functions
// - code_generation.c: AST to C code generation

// These modules are compiled separately by the build system and linked together.
// This file serves as the main entry point and includes only the necessary headers. 
