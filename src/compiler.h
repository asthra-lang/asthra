/**
 * Asthra Programming Language Compiler
 * Core compiler interface and data structures
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_COMPILER_H
#define ASTHRA_COMPILER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// C17 feature detection
#if __STDC_VERSION__ >= 201710L
#define ASTHRA_C17_AVAILABLE 1
#else
#define ASTHRA_C17_AVAILABLE 0
#endif

// C17 static assertions for compile-time validation
#if ASTHRA_C17_AVAILABLE
#include <assert.h>
#define ASTHRA_STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)
#else
#define ASTHRA_STATIC_ASSERT(cond, msg) /* No static assert support */
#endif

#ifdef __cplusplus
extern "C" {
#endif

// Forward declarations
typedef struct AsthraCompilerContext AsthraCompilerContext;
typedef struct AsthraCompilerOptions AsthraCompilerOptions;
typedef struct AsthraCompilerError AsthraCompilerError;
typedef struct AsthraArgumentList AsthraArgumentList;

// Compilation phases
typedef enum {
    ASTHRA_PHASE_LEXING,
    ASTHRA_PHASE_PARSING,
    ASTHRA_PHASE_SEMANTIC_ANALYSIS,
    ASTHRA_PHASE_OPTIMIZATION,
    ASTHRA_PHASE_CODE_GENERATION,
    ASTHRA_PHASE_LINKING
} AsthraCompilerPhase;

// Optimization levels
typedef enum {
    ASTHRA_OPT_NONE,      // -O0: No optimization
    ASTHRA_OPT_BASIC,     // -O1: Basic optimizations
    ASTHRA_OPT_STANDARD,  // -O2: Standard optimizations (default)
    ASTHRA_OPT_AGGRESSIVE // -O3: Aggressive optimizations
} AsthraOptimizationLevel;

// Target architectures
typedef enum {
    ASTHRA_TARGET_X86_64,
    ASTHRA_TARGET_ARM64,
    ASTHRA_TARGET_WASM32,
    ASTHRA_TARGET_NATIVE // Use host architecture
} AsthraTargetArch;


// Output formats supported by the compiler
typedef enum {
    ASTHRA_FORMAT_DEFAULT,   // Default based on file extension
    ASTHRA_FORMAT_LLVM_IR,   // .ll - LLVM IR text format
    ASTHRA_FORMAT_LLVM_BC,   // .bc - LLVM bitcode
    ASTHRA_FORMAT_ASSEMBLY,  // .s  - Native assembly (via llc)
    ASTHRA_FORMAT_OBJECT,    // .o  - Object file (via llc)
    ASTHRA_FORMAT_EXECUTABLE // executable (via clang)
} AsthraOutputFormat;

// Position Independent Executable (PIE) mode
typedef enum {
    ASTHRA_PIE_DEFAULT,      // Use platform-specific defaults
    ASTHRA_PIE_FORCE_ENABLED,  // Explicitly enable PIE
    ASTHRA_PIE_FORCE_DISABLED  // Explicitly disable PIE
} AsthraPIEMode;

// Assembly syntax styles (deprecated - kept for API compatibility)
typedef enum {
    ASTHRA_ASM_SYNTAX_ATT,  // AT&T syntax (no longer used)
    ASTHRA_ASM_SYNTAX_INTEL // Intel syntax (no longer used)
} AsthraAssemblySyntax;

// C17 flexible array member for dynamic argument handling
struct AsthraArgumentList {
    size_t count;
    size_t capacity;
#if ASTHRA_C17_AVAILABLE
    const char *arguments[]; // C17 flexible array member
#else
    const char **arguments; // Fallback for older compilers
#endif
};

// Compiler options with C17 designated initializer support
struct AsthraCompilerOptions {
    const char *input_file;
    const char *output_file;
    AsthraOptimizationLevel opt_level;
    AsthraTargetArch target_arch;
    AsthraOutputFormat output_format; // Output format (IR, assembly, object, etc.)
    AsthraAssemblySyntax asm_syntax;  // Deprecated - kept for API compatibility
    bool debug_info;
    bool verbose;
    bool emit_llvm; // Deprecated - LLVM is now always used
    bool emit_asm;  // Deprecated - use output_format instead
    bool no_stdlib;
    bool coverage; // Enable coverage instrumentation
    AsthraPIEMode pie_mode; // Position Independent Executable mode

    // Dynamic path and library management using flexible arrays
    struct AsthraArgumentList *include_paths;
    struct AsthraArgumentList *library_paths;
    struct AsthraArgumentList *libraries;
};

// Error information
struct AsthraCompilerError {
    AsthraCompilerPhase phase;
    const char *file;
    size_t line;
    size_t column;
    const char *message;
    bool is_warning;
};

// Compiler context
struct AsthraCompilerContext {
    AsthraCompilerOptions options;
    AsthraCompilerError *errors;
    size_t error_count;
    size_t error_capacity;
    void *ast;
    void *symbol_table;
    void *type_checker;
    void *optimizer;
    void *code_generator;
};

// C17 compile-time validation of structure assumptions
ASTHRA_STATIC_ASSERT(sizeof(AsthraOptimizationLevel) == sizeof(int),
                     "AsthraOptimizationLevel must be int-sized for ABI compatibility");
ASTHRA_STATIC_ASSERT(sizeof(AsthraTargetArch) == sizeof(int),
                     "AsthraTargetArch must be int-sized for ABI compatibility");
ASTHRA_STATIC_ASSERT(sizeof(AsthraCompilerPhase) == sizeof(int),
                     "AsthraCompilerPhase must be int-sized for ABI compatibility");

// =============================================================================
// COMPILER INTERFACE
// =============================================================================

// Initialize compiler context
AsthraCompilerContext *asthra_compiler_create(const AsthraCompilerOptions *options);

// Cleanup compiler context
void asthra_compiler_destroy(AsthraCompilerContext *ctx);

// Compile a single file
int asthra_compile_file(AsthraCompilerContext *ctx, const char *input_file,
                        const char *output_file);

// Compile multiple files
int asthra_compile_files(AsthraCompilerContext *ctx, const char **input_files, size_t file_count,
                         const char *output_file);

// Get compilation errors
const AsthraCompilerError *asthra_compiler_get_errors(AsthraCompilerContext *ctx,
                                                      size_t *error_count);

// Clear compilation errors
void asthra_compiler_clear_errors(AsthraCompilerContext *ctx);

// =============================================================================
// ARGUMENT LIST MANAGEMENT (C17 FLEXIBLE ARRAYS)
// =============================================================================

// Create argument list with C17 flexible array member
struct AsthraArgumentList *asthra_argument_list_create(size_t initial_capacity);

// Destroy argument list
void asthra_argument_list_destroy(struct AsthraArgumentList *list);

// Add argument to list
bool asthra_argument_list_add(struct AsthraArgumentList **list, const char *argument);

// Get argument count
size_t asthra_argument_list_count(const struct AsthraArgumentList *list);

// Get argument by index
const char *asthra_argument_list_get(const struct AsthraArgumentList *list, size_t index);

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

// Get default compiler options using C17 designated initializers
AsthraCompilerOptions asthra_compiler_default_options(void);

// Create compiler options with C17 compound literals
AsthraCompilerOptions asthra_compiler_options_create(const char *input_file,
                                                     const char *output_file);

// Validate compiler options
bool asthra_compiler_validate_options(const AsthraCompilerOptions *options);

// Get target triple string
const char *asthra_get_target_triple(AsthraTargetArch arch);

// Get optimization level string
const char *asthra_get_optimization_level_string(AsthraOptimizationLevel level);
char *asthra_backend_get_output_filename(int type, const char *input_file, const char *output_file);

// Get backend type string

// Get compiler version string
const char *asthra_compiler_get_version(void);

// Get compiler build information
const char *asthra_compiler_get_build_info(void);

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_COMPILER_H
