/**
 * Asthra Programming Language Compiler
 * Compilation pipeline - entry point for modular compilation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "../compiler.h"
#include <stddef.h>

// Forward declarations for compilation functions
// These are implemented in separate modules:
// - asthra_compile_file: compilation_single_file.c
// - asthra_compile_files: compilation_multi_file.c

// Single file compilation is implemented in compilation_single_file.c
extern int asthra_compile_file(AsthraCompilerContext *ctx, const char *input_file,
                               const char *output_file);

// Multi-file compilation is implemented in compilation_multi_file.c
extern int asthra_compile_files(AsthraCompilerContext *ctx, const char **input_files,
                                size_t file_count, const char *output_file);

// =============================================================================
// COMPILATION PHASE STUBS
// =============================================================================

// TODO: These compilation phase stubs will be implemented when the modular
// pipeline orchestrator is fully integrated. For now, the main compilation
// logic is handled directly in the separate compilation modules.