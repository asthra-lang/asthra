/**
 * Asthra Programming Language LLVM Tools Integration
 * Provides integration with LLVM ecosystem tools (llc, opt, etc.)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_TOOLS_H
#define ASTHRA_LLVM_TOOLS_H

#include "../compiler.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// LLVM TOOL TYPES AND STRUCTURES
// =============================================================================

// LLVM optimization pass categories
typedef enum {
    ASTHRA_PASSES_NONE,       // No optimization passes
    ASTHRA_PASSES_BASIC,      // Basic cleanup passes (-O1 equivalent)
    ASTHRA_PASSES_STANDARD,   // Standard optimization (-O2 equivalent)
    ASTHRA_PASSES_AGGRESSIVE, // Aggressive optimization (-O3 equivalent)
    ASTHRA_PASSES_SIZE,       // Size optimization (-Os equivalent)
    ASTHRA_PASSES_CUSTOM      // Custom pass pipeline
} AsthraPassCategory;

// LLVM tool invocation options
typedef struct {
    AsthraOutputFormat output_format; // Using AsthraOutputFormat from compiler.h
    AsthraOptimizationLevel opt_level;
    AsthraTargetArch target_arch;
    const char *target_triple; // Custom target triple
    const char *cpu_type;      // Target CPU type (e.g., "generic", "skylake")
    const char *features;      // CPU features (e.g., "+avx2,+fma")
    bool debug_info;
    bool verbose;
    bool coverage; // Enable coverage instrumentation

    // Tool-specific options
    bool use_integrated_as;    // Use clang integrated assembler
    bool emit_llvm_after_opt;  // Emit LLVM IR after optimization
    const char *pass_pipeline; // Custom optimization pass pipeline

    // Output options
    const char *output_file;
    bool force_overwrite;
} AsthraLLVMToolOptions;

// Result from LLVM tool invocation
typedef struct {
    bool success;
    int exit_code;
    char *stdout_output;
    char *stderr_output;
    double execution_time_ms;
} AsthraLLVMToolResult;

// =============================================================================
// COMPONENT HEADERS
// =============================================================================

// Include all LLVM tool component headers
#include "llvm_compilation.h"
#include "llvm_coverage.h"
#include "llvm_linking.h"
#include "llvm_optimization.h"
#include "llvm_pipeline.h"
#include "llvm_process_execution.h"
#include "llvm_tool_detection.h"
#include "llvm_utilities.h"

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_TOOLS_H