/**
 * Asthra Programming Language Compiler
 * Code generation - AST to C code translation (Main Dispatcher)
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include <stdio.h>
#include <stdlib.h>

#include "../parser/ast.h"
#include "code_generation.h"
#include "codegen/codegen_internal.h"

// =============================================================================
// CODE GENERATION DISPATCHER
// =============================================================================

// Main dispatcher function to generate C code from AST
int generate_c_code(FILE *output, ASTNode *node) {
    if (!output || !node) {
        return -1;
    }

    switch (node->type) {
    // Program and function declarations
    case AST_PROGRAM:
        return c_generate_program(output, node);

    case AST_FUNCTION_DECL:
        return c_generate_function_decl(output, node);

    // Statements
    case AST_BLOCK:
        return c_generate_block(output, node);

    case AST_EXPR_STMT:
        return c_generate_expr_stmt(output, node);

    case AST_LET_STMT:
        return c_generate_let_stmt(output, node);

    case AST_RETURN_STMT:
        return c_generate_return_stmt(output, node);

    // Control flow
    case AST_IF_STMT:
        return c_generate_if_stmt(output, node);

    case AST_FOR_STMT:
        return c_generate_for_stmt(output, node);

    case AST_MATCH_STMT:
        return c_generate_match_stmt(output, node);

    // Expressions
    case AST_BINARY_EXPR:
        return c_generate_binary_expr(output, node);

    case AST_UNARY_EXPR:
        return c_generate_unary_expr(output, node);

    case AST_CALL_EXPR:
        return c_generate_call_expr(output, node);

    case AST_IDENTIFIER:
        return c_generate_identifier(output, node);

    // Literals
    case AST_STRING_LITERAL:
        return c_generate_string_literal(output, node);

    case AST_INTEGER_LITERAL:
        return c_generate_integer_literal(output, node);

    case AST_FLOAT_LITERAL:
        return c_generate_float_literal(output, node);

    case AST_BOOL_LITERAL:
        return c_generate_bool_literal(output, node);

    case AST_UNIT_LITERAL:
        return c_generate_unit_literal(output, node);

    // Arrays and slices
    case AST_ARRAY_LITERAL:
        return c_generate_array_literal(output, node);

    case AST_SLICE_EXPR:
        return c_generate_slice_expr(output, node);

    case AST_INDEX_ACCESS:
        return c_generate_index_access(output, node);

    case AST_SLICE_LENGTH_ACCESS:
        return c_generate_slice_length_access(output, node);

    case AST_SLICE_TYPE:
        // This shouldn't appear in expression context, but handle it for completeness
        fprintf(output, "/* slice type */");
        break;

    default:
        // For unhandled node types, just continue
        break;
    }

    return 0;
}
