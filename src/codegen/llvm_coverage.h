/**
 * Asthra Programming Language LLVM Coverage
 * Header for code coverage support using LLVM tools
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_LLVM_COVERAGE_H
#define ASTHRA_LLVM_COVERAGE_H

#include "llvm_tools.h"
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// COVERAGE FUNCTIONS
// =============================================================================

/**
 * Check if LLVM coverage tools are available
 * @return true if llvm-cov and llvm-profdata are available
 */
bool asthra_llvm_coverage_available(void);

/**
 * Merge raw profile data files into a single indexed profile
 * @param profraw_files Array of .profraw file paths
 * @param num_files Number of files to merge
 * @param output_profdata Output .profdata file path
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_merge_profile_data(const char **profraw_files, size_t num_files,
                                                    const char *output_profdata);

/**
 * Generate coverage report in various formats
 * @param executable Path to instrumented executable
 * @param profdata Path to indexed profile data
 * @param format Output format: "html", "text", "lcov", "json"
 * @param output_path Output file or directory path
 * @param source_filters Optional array of source file filters
 * @param num_filters Number of source filters
 * @return Tool execution result
 */
AsthraLLVMToolResult asthra_llvm_coverage_report(const char *executable, const char *profdata,
                                                 const char *format, const char *output_path,
                                                 const char **source_filters, size_t num_filters);

/**
 * Get coverage summary statistics
 * @param executable Path to instrumented executable
 * @param profdata Path to indexed profile data
 * @param line_coverage Output parameter for line coverage percentage
 * @param function_coverage Output parameter for function coverage percentage
 * @param region_coverage Output parameter for region coverage percentage
 * @return true on success, false on error
 */
bool asthra_llvm_coverage_summary(const char *executable, const char *profdata,
                                  double *line_coverage, double *function_coverage,
                                  double *region_coverage);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_LLVM_COVERAGE_H