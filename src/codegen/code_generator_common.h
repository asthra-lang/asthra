/**
 * Asthra Programming Language Compiler
 * Code Generator Common Utilities Header
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Common utilities and dependencies for code generator modules
 */

#ifndef CODE_GENERATOR_COMMON_H
#define CODE_GENERATOR_COMMON_H

#include "code_generator.h"
#include "../analysis/type_info.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>

// C17 static assertions for implementation assumptions
_Static_assert(sizeof(atomic_uint_fast64_t) >= sizeof(uint64_t), "Atomic types must be at least 64-bit");
_Static_assert(sizeof(pthread_mutex_t) > 0, "pthread_mutex_t must be available");

// =============================================================================
// SHARED CONSTANTS AND MACROS
// =============================================================================

// Buffer sizes for code generation
#define DEFAULT_INSTRUCTION_BUFFER_SIZE 1024
#define DEFAULT_LABEL_MANAGER_SIZE 256
#define DEFAULT_SYMBOL_TABLE_SIZE 512
#define DEFAULT_STACK_FRAME_SIZE 256
#define DEFAULT_TYPEDEF_BUFFER_SIZE 1024
#define DEFAULT_CONSTRUCTOR_BUFFER_SIZE 512
#define DEFAULT_HELPER_BUFFER_SIZE 512

// Maximum number of registers that can be saved in prologue/epilogue
#define MAX_SAVED_REGISTERS 16

// Default stack alignment (16 bytes for x86_64)
#define STACK_ALIGNMENT 16

// =============================================================================
// SHARED HELPER FUNCTION DECLARATIONS
// =============================================================================

// Note: Function declarations that conflict with existing implementations
// in other files (like code_generator_abi.c and code_generator_statements.c)
// are not redeclared here to avoid duplicate symbol errors.

// This header provides common includes and utilities for the split modules
// without duplicating declarations that already exist in other headers.

#endif // CODE_GENERATOR_COMMON_H 
