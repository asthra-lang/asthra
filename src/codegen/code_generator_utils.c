/**
 * Asthra Programming Language Compiler
 * Code Generator Utility Functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 * 
 * Core utility functions that coordinate the split functionality
 */

#include "code_generator.h"

// Include the split functionality
#include "codegen_assembly_output.c"
#include "codegen_statistics.c"
#include "codegen_error_handling.c"
#include "codegen_validation.c"
#include "codegen_arch_formatter.c"

// This file now serves as a coordination point for the code generator utilities
// The actual implementation is split into:
// - codegen_assembly_output.c: Assembly emission and file writing
// - codegen_statistics.c: Statistics tracking and reporting
// - codegen_error_handling.c: Error reporting and naming
// - codegen_validation.c: Instruction validation and debugging
// - codegen_arch_formatter.c: Architecture-specific assembly formatting