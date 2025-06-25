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
    ASTHRA_PASSES_NONE,         // No optimization passes
    ASTHRA_PASSES_BASIC,        // Basic cleanup passes (-O1 equivalent)
    ASTHRA_PASSES_STANDARD,     // Standard optimization (-O2 equivalent)
    ASTHRA_PASSES_AGGRESSIVE,   // Aggressive optimization (-O3 equivalent)
    ASTHRA_PASSES_SIZE,         // Size optimization (-Os equivalent)
    ASTHRA_PASSES_CUSTOM        // Custom pass pipeline
} AsthraPassCategory;

// LLVM tool invocation options
typedef struct {
    AsthraOutputFormat output_format;  // Using AsthraOutputFormat from compiler.h
    AsthraOptimizationLevel opt_level;
    AsthraTargetArch target_arch;
    const char *target_triple;      // Custom target triple
    const char *cpu_type;           // Target CPU type (e.g., "generic", "skylake")
    const char *features;           // CPU features (e.g., "+avx2,+fma")
    bool debug_info;
    bool verbose;
    
    // Tool-specific options
    bool use_integrated_as;         // Use clang integrated assembler
    bool emit_llvm_after_opt;       // Emit LLVM IR after optimization
    const char *pass_pipeline;      // Custom optimization pass pipeline
    
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
// LLVM TOOL DETECTION AND VALIDATION
// =============================================================================

/**
 * Check if LLVM tools are available in PATH
 * Returns true if all required tools are found
 */
bool asthra_llvm_tools_available(void);

/**
 * Get the path to a specific LLVM tool
 * Returns NULL if tool not found
 */
const char* asthra_llvm_tool_path(const char *tool_name);

/**
 * Get LLVM version information
 * Returns version string or NULL on error
 */
const char* asthra_llvm_version(void);

// =============================================================================
// LLVM IR OPTIMIZATION (via opt)
// =============================================================================

/**
 * Optimize LLVM IR using the opt tool
 * 
 * @param input_file Path to input .ll or .bc file
 * @param options Tool invocation options
 * @return Result of optimization
 */
AsthraLLVMToolResult asthra_llvm_optimize(const char *input_file, 
                                          const AsthraLLVMToolOptions *options);

/**
 * Run specific optimization passes
 * 
 * @param input_file Path to input .ll or .bc file
 * @param passes Comma-separated list of passes (e.g., "mem2reg,inline,gvn")
 * @param output_file Output file path
 * @return Result of optimization
 */
AsthraLLVMToolResult asthra_llvm_run_passes(const char *input_file,
                                            const char *passes,
                                            const char *output_file);

// =============================================================================
// NATIVE CODE GENERATION (via llc)
// =============================================================================

/**
 * Generate assembly or object code using llc
 * 
 * @param input_file Path to input .ll or .bc file
 * @param options Tool invocation options
 * @return Result of code generation
 */
AsthraLLVMToolResult asthra_llvm_compile(const char *input_file,
                                         const AsthraLLVMToolOptions *options);

/**
 * Generate assembly code for specific target
 * 
 * @param input_file Path to input .ll or .bc file
 * @param output_file Output assembly file path
 * @param target_triple Target triple (NULL for host)
 * @return Result of assembly generation
 */
AsthraLLVMToolResult asthra_llvm_to_assembly(const char *input_file,
                                             const char *output_file,
                                             const char *target_triple);

// =============================================================================
// LINKING AND EXECUTABLE GENERATION (via clang)
// =============================================================================

/**
 * Link object files and libraries using clang
 * 
 * @param object_files Array of object file paths
 * @param num_objects Number of object files
 * @param options Tool invocation options
 * @return Result of linking
 */
AsthraLLVMToolResult asthra_llvm_link(const char **object_files,
                                      size_t num_objects,
                                      const AsthraLLVMToolOptions *options);

/**
 * Compile LLVM IR directly to executable using clang
 * 
 * @param input_file Path to input .ll or .bc file
 * @param output_file Output executable path
 * @param libraries Array of libraries to link
 * @param num_libraries Number of libraries
 * @return Result of compilation
 */
AsthraLLVMToolResult asthra_llvm_to_executable(const char *input_file,
                                               const char *output_file,
                                               const char **libraries,
                                               size_t num_libraries);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

/**
 * Get default output filename based on input and format
 * Caller must free returned string
 */
char* asthra_llvm_get_output_filename(const char *input_file,
                                     AsthraOutputFormat format);

/**
 * Convert Asthra optimization level to LLVM opt flags
 */
const char* asthra_llvm_opt_level_flag(AsthraOptimizationLevel level);

/**
 * Get target triple for Asthra target architecture
 */
const char* asthra_llvm_target_triple(AsthraTargetArch arch);

/**
 * Free LLVM tool result
 */
void asthra_llvm_tool_result_free(AsthraLLVMToolResult *result);

// =============================================================================
// PIPELINE INTEGRATION
// =============================================================================

/**
 * Run complete compilation pipeline using LLVM tools
 * 
 * @param ir_file Input LLVM IR file
 * @param output_file Final output file
 * @param format Desired output format
 * @param options Compilation options
 * @return Result of pipeline execution
 */
AsthraLLVMToolResult asthra_llvm_compile_pipeline(const char *ir_file,
                                                  const char *output_file,
                                                  AsthraOutputFormat format,
                                                  const AsthraCompilerOptions *options);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_TOOLS_H