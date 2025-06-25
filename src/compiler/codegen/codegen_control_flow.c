/**
 * Asthra Programming Language Compiler
 * Code generation control flow - if, for, match statements
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdio.h>

int c_generate_if_stmt(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_IF_STMT) {
        return -1;
    }

    // Handle if statements
    fprintf(output, "    if (");
    if (node->data.if_stmt.condition) {
        generate_c_code(output, node->data.if_stmt.condition);
    }
    fprintf(output, ") {\n");
    if (node->data.if_stmt.then_block) {
        generate_c_code(output, node->data.if_stmt.then_block);
    }
    fprintf(output, "    }\n");
    if (node->data.if_stmt.else_block) {
        fprintf(output, "    else {\n");
        generate_c_code(output, node->data.if_stmt.else_block);
        fprintf(output, "    }\n");
    }

    return 0;
}

int c_generate_for_stmt(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_FOR_STMT) {
        return -1;
    }

    // Handle for loops (for-in style)
    if (node->data.for_stmt.variable && node->data.for_stmt.iterable) {
        const char *iterator_var_name = node->data.for_stmt.variable;

        // Generate proper slice iteration
        fprintf(output, "    // Generated from Asthra for-in loop\n");
        fprintf(output, "    {\n");

        // Get the slice to iterate over
        fprintf(output, "        AsthraSliceHeader _slice = ");
        generate_c_code(output, node->data.for_stmt.iterable);
        fprintf(output, ";\n");

        // Determine element type from the iterable's type info
        const char *element_type = "int"; // Default
        // TODO: Once TypeInfo is properly exposed in code generation,
        // we can check if the iterable is a slice and get its element type

        // Generate the for loop
        fprintf(output, "        for (size_t _idx = 0; _idx < _slice.len; _idx++) {\n");
        fprintf(output,
                "            %s %s = *((%s*)((char*)_slice.ptr + _idx * _slice.element_size));\n",
                element_type, iterator_var_name, element_type);

        // Generate loop body
        if (node->data.for_stmt.body) {
            generate_c_code(output, node->data.for_stmt.body);
        }

        fprintf(output, "        }\n");
        fprintf(output, "    }\n");

    } else {
        // Other types of for loops (not implemented yet)
        fprintf(output, "    // TODO: Implement other for loop types\n");
    }

    return 0;
}

int c_generate_match_stmt(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_MATCH_STMT) {
        return -1;
    }

    // Generate code for match statement using if-else if structure
    if (node->data.match_stmt.expression && node->data.match_stmt.arms) {
        // Analyze the match expression once
        fprintf(output, "    // Generated from Asthra match statement (simplified)\n");
        fprintf(output, "    // Match expression: ");
        // generate_c_code(output, node->data.match_stmt.expression); // Would generate expr
        fprintf(output, " (expression evaluated)\n");

        // Generate if-else if chain for each arm
        for (size_t i = 0; i < node->data.match_stmt.arms->count; i++) {
            ASTNode *arm = node->data.match_stmt.arms->nodes[i];
            if (arm->type != AST_MATCH_ARM)
                continue; // Should be validated by semantic analysis

            ASTNode *pattern = arm->data.match_arm.pattern;
            ASTNode *body = arm->data.match_arm.body;

            if (i == 0) {
                fprintf(output, "    if (");
            } else {
                fprintf(output, "    else if (");
            }

            // Generate condition for pattern (simplified)
            if (pattern->type == AST_WILDCARD_PATTERN) {
                fprintf(output, "1 /* _ */"); // Wildcard always true
            } else if (pattern->type == AST_IDENTIFIER) {
                // For identifier patterns, bind variable. Simplified for now.
                fprintf(output, "1 /* let %s = expr */", pattern->data.identifier.name);
            } else {
                // Other patterns (enum, struct, literal) - highly complex, will be simplified
                fprintf(output, "0 /* Complex pattern not generated */");
            }

            fprintf(output, ") {\n");
            generate_c_code(output, body);
            fprintf(output, "    }\n");
        }

        // Add an else block for non-exhaustive matches (if no wildcard)
        // A real implementation would check for exhaustiveness here.
        // For now, we assume semantic analysis handles exhaustiveness warnings.
        fprintf(output,
                "    // Optional else for non-exhaustive matches (semantic check should warn)\n");
    }

    return 0;
}