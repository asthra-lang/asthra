/**
 * Asthra Programming Language Compiler
 * Common types and definitions shared across parser components
 * 
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#ifndef ASTHRA_PARSER_COMMON_H
#define ASTHRA_PARSER_COMMON_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// Source location for error reporting and AST nodes
typedef struct SourceLocation {
    char *filename;
    int line;
    int column;
    int offset;
} SourceLocation;

#ifdef __cplusplus
}
#endif

#endif // ASTHRA_PARSER_COMMON_H 
