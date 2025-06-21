/**
 * Asthra Programming Language Compiler
 * Code Generation Optimization Support
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_CODE_GENERATOR_OPTIMIZATION_H
#define ASTHRA_CODE_GENERATOR_OPTIMIZATION_H

#include "code_generator_types.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// OPTIMIZATION PASSES
// =============================================================================

// Basic optimizations
bool optimize_dead_code_elimination(CodeGenerator *generator);
bool optimize_constant_folding(CodeGenerator *generator);
bool optimize_register_coalescing(CodeGenerator *generator);
bool optimize_instruction_scheduling(CodeGenerator *generator);

// =============================================================================
// DEBUGGING AND DIAGNOSTICS
// =============================================================================

// Statistics and profiling
CodeGenStatistics code_generator_get_statistics(const CodeGenerator *generator);
void code_generator_reset_statistics(CodeGenerator *generator);
void code_generator_print_statistics(const CodeGenerator *generator);

// Debugging output
void code_generator_print_instructions(const CodeGenerator *generator);
void code_generator_print_register_allocation(const CodeGenerator *generator);
bool code_generator_validate_instructions(const CodeGenerator *generator);

// Error reporting
void code_generator_report_error(CodeGenerator *generator, CodeGenErrorCode code, 
                                const char *format, ...);
const char *code_generator_error_name(CodeGenErrorCode code);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_CODE_GENERATOR_OPTIMIZATION_H 
