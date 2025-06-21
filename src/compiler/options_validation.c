/**
 * Asthra Programming Language Compiler
 * Options validation and utility functions
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../compiler.h"

// =============================================================================
// UTILITY FUNCTIONS
// =============================================================================

AsthraCompilerOptions asthra_compiler_default_options(void) {
    // C17 designated initializer for default options
    return (AsthraCompilerOptions) {
        .input_file = NULL,
        .output_file = NULL,
        .opt_level = ASTHRA_OPT_STANDARD,
        .target_arch = ASTHRA_TARGET_NATIVE,
        .debug_info = false,
        .verbose = false,
        .emit_llvm = false,
        .emit_asm = false,
        .no_stdlib = false,
        .include_paths = NULL,
        .library_paths = NULL,
        .libraries = NULL
    };
}

AsthraCompilerOptions asthra_compiler_options_create(const char *input_file, const char *output_file) {
    // C17 compound literal with designated initializers
    return (AsthraCompilerOptions) {
        .input_file = input_file,
        .output_file = output_file,
        .opt_level = ASTHRA_OPT_STANDARD,
        .target_arch = ASTHRA_TARGET_NATIVE,
        .debug_info = false,
        .verbose = false,
        .emit_llvm = false,
        .emit_asm = false,
        .no_stdlib = false,
        .include_paths = asthra_argument_list_create(8),
        .library_paths = asthra_argument_list_create(8),
        .libraries = asthra_argument_list_create(8)
    };
}

bool asthra_compiler_validate_options(const AsthraCompilerOptions *options) {
    if (!options) {
        return false;
    }

    // Validate input file
    if (!options->input_file || strlen(options->input_file) == 0) {
        return false;
    }

    // Validate optimization level
    if (options->opt_level < ASTHRA_OPT_NONE || options->opt_level > ASTHRA_OPT_AGGRESSIVE) {
        return false;
    }

    // Validate target architecture
    if (options->target_arch < ASTHRA_TARGET_X86_64 || options->target_arch > ASTHRA_TARGET_NATIVE) {
        return false;
    }

    // Validate mutually exclusive options
    if (options->emit_llvm && options->emit_asm) {
        return false; // Can't emit both LLVM IR and assembly
    }

    return true;
}

const char *asthra_get_target_triple(AsthraTargetArch arch) {
    // C17 compound literal for temporary string mapping
    static const struct {
        AsthraTargetArch arch;
        const char *triple;
    } target_map[] = {
        {ASTHRA_TARGET_X86_64, "x86_64-unknown-linux-gnu"},
        {ASTHRA_TARGET_ARM64, "aarch64-unknown-linux-gnu"},
        {ASTHRA_TARGET_WASM32, "wasm32-unknown-unknown"},
        {ASTHRA_TARGET_NATIVE, "native"}
    };

    for (size_t i = 0; i < sizeof(target_map) / sizeof(target_map[0]); i++) {
        if (target_map[i].arch == arch) {
            return target_map[i].triple;
        }
    }

    return "unknown";
}

const char *asthra_get_optimization_level_string(AsthraOptimizationLevel level) {
    // C17 compound literal for optimization level mapping
    static const struct {
        AsthraOptimizationLevel level;
        const char *string;
    } opt_map[] = {
        {ASTHRA_OPT_NONE, "O0"},
        {ASTHRA_OPT_BASIC, "O1"},
        {ASTHRA_OPT_STANDARD, "O2"},
        {ASTHRA_OPT_AGGRESSIVE, "O3"}
    };

    for (size_t i = 0; i < sizeof(opt_map) / sizeof(opt_map[0]); i++) {
        if (opt_map[i].level == level) {
            return opt_map[i].string;
        }
    }

    return "O2"; // Default fallback
} 
