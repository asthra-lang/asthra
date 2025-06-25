/**
 * Real Program Test Generators Implementation
 *
 * Contains functions for generating test programs with various
 * characteristics for comprehensive testing.
 *
 * Copyright (c) 2025 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "real_program_test_generators.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// PROGRAM GENERATORS
// =============================================================================

char *generate_large_program(size_t line_count) {
    if (line_count == 0) {
        return NULL;
    }

    // Estimate buffer size (assuming ~50 chars per line)
    size_t buffer_size = line_count * 60 + 1000;
    char *program = malloc(buffer_size);
    if (!program) {
        return NULL;
    }

    strcpy(program, "package main;\n\npub fn main(none) -> void {\n");

    for (size_t i = 0; i < line_count - 10; i++) {
        char line[128];
        snprintf(line, sizeof(line), "    let var_%zu: int = %zu;\n", i, i);
        strcat(program, line);
    }

    strcat(program, "}\n");

    return program;
}

char *generate_complex_type_program(void) {
    const char *program_template = "package main;\n"
                                   "\n"
                                   "pub fn main(none) -> void {\n"
                                   "    let simple: int = 42;\n"
                                   "    let text: string = \"hello\";\n"
                                   "    let flag: bool = true;\n"
                                   "    let numbers: []int = [1, 2, 3, 4, 5];\n"
                                   "    let first: int = numbers[0];\n"
                                   "}\n";

    size_t len = strlen(program_template) + 1;
    char *program = malloc(len);
    if (program) {
        strcpy(program, program_template);
    }

    return program;
}

char *generate_deep_nesting_program(size_t depth) {
    if (depth == 0) {
        return NULL;
    }

    size_t buffer_size = depth * 100 + 1000;
    char *program = malloc(buffer_size);
    if (!program) {
        return NULL;
    }

    strcpy(program, "package main;\n\npub fn main(none) -> void {\n");

    // Create nested if statements
    for (size_t i = 0; i < depth; i++) {
        char spaces[100];
        memset(spaces, ' ', (i + 1) * 4);
        spaces[(i + 1) * 4] = '\0';

        char line[200];
        snprintf(line, sizeof(line), "%sif true {\n", spaces);
        strcat(program, line);
    }

    // Add inner content
    char spaces[100];
    memset(spaces, ' ', (depth + 1) * 4);
    spaces[(depth + 1) * 4] = '\0';

    char inner[200];
    snprintf(inner, sizeof(inner), "%slet x: int = 42;\n", spaces);
    strcat(program, inner);

    // Close all if statements
    for (size_t i = 0; i < depth; i++) {
        size_t indent = (depth - i) * 4;
        char spaces[100];
        memset(spaces, ' ', indent);
        spaces[indent] = '\0';

        char line[100];
        snprintf(line, sizeof(line), "%s}\n", spaces);
        strcat(program, line);
    }

    strcat(program, "}\n");

    return program;
}

char *generate_feature_combination_program(const char **features, size_t feature_count) {
    if (!features || feature_count == 0) {
        return NULL;
    }

    // Simple implementation that creates a basic program
    const char *program_template = "package main;\n"
                                   "\n"
                                   "pub fn main(none) -> void {\n"
                                   "    let x: int = 42;\n"
                                   "    let text: string = \"hello\";\n"
                                   "}\n";

    size_t len = strlen(program_template) + 1;
    char *program = malloc(len);
    if (program) {
        strcpy(program, program_template);
    }

    return program;
}