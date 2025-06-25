/**
 * Asthra Programming Language Compiler
 * Core compiler functionality - context management and main compilation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../runtime/asthra_runtime.h"
#include "../compiler.h"
#include "../parser/ast.h"
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "../version.h"

// =============================================================================
// COMPILER CONTEXT MANAGEMENT
// =============================================================================

AsthraCompilerContext *asthra_compiler_create(const AsthraCompilerOptions *options) {
    if (!options) {
        return NULL;
    }

    AsthraCompilerContext *ctx = calloc(1, sizeof(AsthraCompilerContext));
    if (!ctx) {
        return NULL;
    }

    // Copy options using C17 compound literal for temporary structure
    ctx->options = *options;

    // Initialize error tracking with C17 designated initializer pattern
    ctx->error_capacity = 64;
    ctx->errors = calloc(ctx->error_capacity, sizeof(AsthraCompilerError));
    if (!ctx->errors) {
        free(ctx);
        return NULL;
    }

    return ctx;
}

void asthra_compiler_destroy(AsthraCompilerContext *ctx) {
    if (!ctx) {
        return;
    }

    // Free argument lists
    asthra_argument_list_destroy(ctx->options.include_paths);
    asthra_argument_list_destroy(ctx->options.library_paths);
    asthra_argument_list_destroy(ctx->options.libraries);

    // Free errors
    if (ctx->errors) {
        free(ctx->errors);
    }

    // Free other components (when implemented)
    if (ctx->ast) {
        // TODO: Free AST
    }
    if (ctx->symbol_table) {
        // TODO: Free symbol table
    }
    if (ctx->type_checker) {
        // TODO: Free type checker
    }
    if (ctx->optimizer) {
        // TODO: Free optimizer
    }
    if (ctx->code_generator) {
        // TODO: Free code generator
    }

    free(ctx);
}

// =============================================================================
// VERSION AND BUILD INFORMATION
// =============================================================================

const char *asthra_compiler_get_version(void) {
    return ASTHRA_VERSION_STRING;
}

const char *asthra_compiler_get_build_info(void) {
    static char build_info[256];
    snprintf(build_info, sizeof(build_info), "Asthra Compiler v%s (built %s %s)",
             ASTHRA_VERSION_STRING, ASTHRA_BUILD_DATE, ASTHRA_BUILD_TIME);
    return build_info;
}
