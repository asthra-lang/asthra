/**
 * Asthra Programming Language Compiler
 * Code generation literals - literal value generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdio.h>

int c_generate_string_literal(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_STRING_LITERAL) {
        return -1;
    }

    // Output string literal
    if (node->data.string_literal.value) {
        fprintf(output, "\"%s\"", node->data.string_literal.value);
    }

    return 0;
}

int c_generate_integer_literal(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_INTEGER_LITERAL) {
        return -1;
    }

    // Output integer literal
    fprintf(output, "%lld", (long long)node->data.integer_literal.value);

    return 0;
}

int c_generate_float_literal(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_FLOAT_LITERAL) {
        return -1;
    }

    // Output float literal
    fprintf(output, "%f", node->data.float_literal.value);

    return 0;
}

int c_generate_bool_literal(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_BOOL_LITERAL) {
        return -1;
    }

    // Output boolean literal
    fprintf(output, "%s", node->data.bool_literal.value ? "1" : "0");

    return 0;
}

int c_generate_unit_literal(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_UNIT_LITERAL) {
        return -1;
    }

    // Output unit literal as empty (void)
    // In C, unit type is typically represented as void or no value
    // We don't output anything for unit literals

    return 0;
}