/**
 * Asthra Programming Language Compiler
 * Code generation expressions - expression generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "codegen_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int c_generate_binary_expr(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_BINARY_EXPR) {
        return -1;
    }

    // Handle binary expressions
    generate_c_code(output, node->data.binary_expr.left);

    // Output the correct operator
    switch (node->data.binary_expr.operator) {
    case BINOP_ADD:
        fprintf(output, " + ");
        break;
    case BINOP_SUB:
        fprintf(output, " - ");
        break;
    case BINOP_MUL:
        fprintf(output, " * ");
        break;
    case BINOP_DIV:
        fprintf(output, " / ");
        break;
    case BINOP_MOD:
        fprintf(output, " %% ");
        break;
    case BINOP_EQ:
        fprintf(output, " == ");
        break;
    case BINOP_NE:
        fprintf(output, " != ");
        break;
    case BINOP_LT:
        fprintf(output, " < ");
        break;
    case BINOP_LE:
        fprintf(output, " <= ");
        break;
    case BINOP_GT:
        fprintf(output, " > ");
        break;
    case BINOP_GE:
        fprintf(output, " >= ");
        break;
    case BINOP_AND:
        fprintf(output, " && ");
        break;
    case BINOP_OR:
        fprintf(output, " || ");
        break;
    case BINOP_BITWISE_AND:
        fprintf(output, " & ");
        break;
    case BINOP_BITWISE_OR:
        fprintf(output, " | ");
        break;
    case BINOP_BITWISE_XOR:
        fprintf(output, " ^ ");
        break;
    case BINOP_LSHIFT:
        fprintf(output, " << ");
        break;
    case BINOP_RSHIFT:
        fprintf(output, " >> ");
        break;
    default:
        fprintf(output, " /* unknown op */ ");
        break;
    }

    generate_c_code(output, node->data.binary_expr.right);

    return 0;
}

int c_generate_unary_expr(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_UNARY_EXPR) {
        return -1;
    }

    // Handle unary expressions
    switch (node->data.unary_expr.operator) {
    case UNOP_MINUS:
        fprintf(output, "-");
        break;
    case UNOP_NOT:
        fprintf(output, "!");
        break;
    case UNOP_BITWISE_NOT:
        fprintf(output, "~");
        break;
    case UNOP_DEREF:
        fprintf(output, "*");
        break;
    case UNOP_ADDRESS_OF:
        fprintf(output, "&");
        break;
    default:
        fprintf(output, "/* unknown unary op */");
        break;
    }
    generate_c_code(output, node->data.unary_expr.operand);

    return 0;
}

int c_generate_call_expr(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_CALL_EXPR) {
        return -1;
    }

    // Handle function calls (especially log() and panic())
    if (node->data.call_expr.function && node->data.call_expr.function->type == AST_IDENTIFIER &&
        strcmp(node->data.call_expr.function->data.identifier.name, "log") == 0) {
        fprintf(output, "    printf(");

        // Process arguments
        if (node->data.call_expr.args) {
            ASTNodeList *args = node->data.call_expr.args;
            for (size_t i = 0; i < args->count; i++) {
                if (args->nodes[i]) {
                    if (i > 0) {
                        fprintf(output, ", ");
                    }
                    generate_c_code(output, args->nodes[i]);
                }
            }
        }

        fprintf(output, ");\n    printf(\"\\n\")");
    } else if (node->data.call_expr.function &&
               node->data.call_expr.function->type == AST_IDENTIFIER &&
               strcmp(node->data.call_expr.function->data.identifier.name, "panic") == 0) {
        // Handle panic() - print to stderr and exit
        fprintf(output, "    fprintf(stderr, \"panic: \");\n");
        fprintf(output, "    fprintf(stderr, ");

        // Process arguments (expect one string argument)
        if (node->data.call_expr.args && node->data.call_expr.args->count > 0) {
            generate_c_code(output, node->data.call_expr.args->nodes[0]);
        }

        fprintf(output, ");\n");
        fprintf(output, "    fprintf(stderr, \"\\n\");\n");
        fprintf(output, "    exit(1)");
    } else if (node->data.call_expr.function &&
               node->data.call_expr.function->type == AST_IDENTIFIER &&
               strcmp(node->data.call_expr.function->data.identifier.name, "args") == 0) {
        // Handle args() - generate inline stub for testing
        // TODO: Use asthra_runtime_get_args() once runtime linking is implemented
        fprintf(output,
                "((AsthraSliceHeader){.ptr = NULL, .len = 0, .cap = 0, .element_size = "
                "sizeof(char*), .ownership = ASTHRA_OWNERSHIP_GC, .is_mutable = 0, .type_id = 0})");
    } else {
        // Other function calls
        if (node->data.call_expr.function) {
            generate_c_code(output, node->data.call_expr.function);
            fprintf(output, "(");

            if (node->data.call_expr.args) {
                ASTNodeList *args = node->data.call_expr.args;
                for (size_t i = 0; i < args->count; i++) {
                    if (args->nodes[i]) {
                        if (i > 0) {
                            fprintf(output, ", ");
                        }
                        generate_c_code(output, args->nodes[i]);
                    }
                }
            }

            fprintf(output, ")");
        }
    }

    return 0;
}

int c_generate_identifier(FILE *output, ASTNode *node) {
    if (!output || !node || node->type != AST_IDENTIFIER) {
        return -1;
    }

    // Output identifier
    if (node->data.identifier.name) {
        fprintf(output, "%s", node->data.identifier.name);
    }

    return 0;
}