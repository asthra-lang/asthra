/**
 * Asthra Programming Language Compiler
 * Code generation statements - statement generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdio.h>
#include <string.h>

int c_generate_let_stmt(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_LET_STMT) {
        return -1;
    }

    // Handle variable declarations with immutable-by-default semantics
    if (node->data.let_stmt.name) {
        // Use our type helper function to get the correct C type
        const char *c_type = get_c_type_string(node->data.let_stmt.type);

        // PHASE 3 ENHANCEMENT: Generate appropriate C const/mutable semantics
        if (node->data.let_stmt.is_mutable) {
            // Mutable variable - normal C variable
            fprintf(output, "    %s %s", c_type, node->data.let_stmt.name);
        } else {
            // Immutable variable - C const
            fprintf(output, "    const %s %s", c_type, node->data.let_stmt.name);
        }

        if (node->data.let_stmt.initializer) {
            fprintf(output, " = ");
            generate_c_code(output, node->data.let_stmt.initializer);
        }
        fprintf(output, ";\n");
    }

    return 0;
}

int c_generate_return_stmt(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_RETURN_STMT) {
        return -1;
    }

    // Handle return statements
    fprintf(output, "    return");

    // Special case: main function with void return type needs to return 0
    bool is_main_void_return = current_function_name &&
                               strcmp(current_function_name, "main") == 0 &&
                               current_function_returns_void;

    if (node->data.return_stmt.expression) {
        // Check if the expression is a unit literal
        if (node->data.return_stmt.expression->type == AST_UNIT_LITERAL) {
            // For unit literals in void main, return 0
            if (is_main_void_return) {
                fprintf(output, " 0");
            }
            // Otherwise, don't output anything for unit literals
        } else {
            fprintf(output, " ");
            generate_c_code(output, node->data.return_stmt.expression);
        }
    } else if (is_main_void_return) {
        // Empty return in void main should return 0
        fprintf(output, " 0");
    }
    fprintf(output, ";\n");

    return 0;
}

int c_generate_expr_stmt(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_EXPR_STMT) {
        return -1;
    }

    // Generate code for expression statement
    if (node->data.expr_stmt.expression) {
        generate_c_code(output, node->data.expr_stmt.expression);
        fprintf(output, ";\n");
    }

    return 0;
}

int c_generate_block(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_BLOCK) {
        return -1;
    }

    // Process all statements in the block
    if (node->data.block.statements) {
        ASTNodeList *statements = node->data.block.statements;
        for (size_t i = 0; i < statements->count; i++) {
            if (statements->nodes[i]) {
                generate_c_code(output, statements->nodes[i]);
            }
        }
    }

    return 0;
}