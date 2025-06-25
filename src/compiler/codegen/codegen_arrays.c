/**
 * Asthra Programming Language Compiler
 * Code generation arrays - array/slice code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdio.h>

int c_generate_array_literal(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_ARRAY_LITERAL) {
        return -1;
    }

    // Generate array literal as slice initialization
    if (node->data.array_literal.elements) {
        ASTNodeList *elements = node->data.array_literal.elements;

        // For now, we'll generate a static array and create a slice from it
        // This is a simplified approach - a real implementation would use heap allocation
        fprintf(output, "({\n");

        // Determine element type from first element or type info
        const char *element_type = "int"; // Default to int
        // TODO: Once TypeInfo is properly exposed in code generation,
        // we can check the actual slice element type
        if (elements->count > 0 && elements->nodes[0]) {
            // Infer from first element
            ASTNode *first = elements->nodes[0];
            if (first->type == AST_STRING_LITERAL) {
                element_type = "char*";
            } else if (first->type == AST_FLOAT_LITERAL) {
                element_type = "double";
            } else if (first->type == AST_BOOL_LITERAL) {
                element_type = "int";
            }
        }

        // Generate static array
        fprintf(output, "        static %s _arr[] = {", element_type);
        for (size_t i = 0; i < elements->count; i++) {
            if (i > 0)
                fprintf(output, ", ");
            generate_c_code(output, elements->nodes[i]);
        }
        fprintf(output, "};\n");

        // Create slice header - note the expression must evaluate to the struct, not void
        fprintf(output,
                "        AsthraSliceHeader _slice = {.ptr = _arr, .len = %zu, .cap = %zu, "
                ".element_size = sizeof(%s), .ownership = ASTHRA_OWNERSHIP_GC, .is_mutable = "
                "0, .type_id = 0};\n",
                elements->count, elements->count, element_type);
        fprintf(output, "        _slice;\n"); // Return the slice
        fprintf(output, "    })");
    }

    return 0;
}

int c_generate_slice_expr(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_SLICE_EXPR) {
        return -1;
    }

    // Generate slice expression array[start:end]
    if (node->data.slice_expr.array) {
        fprintf(output, "asthra_slice_subslice(");
        generate_c_code(output, node->data.slice_expr.array);
        fprintf(output, ", ");

        // Start index (default to 0 if NULL)
        if (node->data.slice_expr.start) {
            generate_c_code(output, node->data.slice_expr.start);
        } else {
            fprintf(output, "0");
        }
        fprintf(output, ", ");

        // End index (default to length if NULL)
        if (node->data.slice_expr.end) {
            generate_c_code(output, node->data.slice_expr.end);
        } else {
            // Need to get the length of the array
            fprintf(output, "asthra_slice_get_len(");
            generate_c_code(output, node->data.slice_expr.array);
            fprintf(output, ")");
        }
        fprintf(output, ")");
    }

    return 0;
}

int c_generate_index_access(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_INDEX_ACCESS) {
        return -1;
    }

    // Generate array index access array[index]
    if (node->data.index_access.array && node->data.index_access.index) {
        // We need to dereference the element pointer and cast to the correct type
        const char *element_type = "int"; // Default to int
        // For now, we'll use a simple heuristic based on the expected result type
        // A proper implementation would use type inference from semantic analysis

        fprintf(output, "(*(%s*)asthra_slice_get_element(", element_type);
        generate_c_code(output, node->data.index_access.array);
        fprintf(output, ", ");
        generate_c_code(output, node->data.index_access.index);
        fprintf(output, "))");
    }

    return 0;
}

int c_generate_slice_length_access(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_SLICE_LENGTH_ACCESS) {
        return -1;
    }

    // Generate slice.len access
    if (node->data.slice_length_access.slice) {
        fprintf(output, "asthra_slice_get_len(");
        generate_c_code(output, node->data.slice_length_access.slice);
        fprintf(output, ")");
    }

    return 0;
}