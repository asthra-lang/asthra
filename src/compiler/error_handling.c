/**
 * Asthra Programming Language Compiler
 * Error handling and reporting
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../compiler.h"

// =============================================================================
// ERROR HANDLING
// =============================================================================

// TODO: add_compiler_error function will be implemented when error reporting
// system is integrated with the compilation pipeline

const AsthraCompilerError *asthra_compiler_get_errors(AsthraCompilerContext *ctx,
                                                      size_t *error_count) {
    if (!ctx || !error_count) {
        return NULL;
    }

    *error_count = ctx->error_count;
    return ctx->errors;
}

void asthra_compiler_clear_errors(AsthraCompilerContext *ctx) {
    if (ctx) {
        ctx->error_count = 0;
    }
}

// =============================================================================
// INTERNAL HELPER FUNCTIONS
// =============================================================================

// TODO: get_phase_name function will be implemented when phase-specific
// error reporting is integrated with the compilation pipeline
