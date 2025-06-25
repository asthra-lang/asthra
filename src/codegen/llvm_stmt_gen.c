/**
 * Asthra Programming Language LLVM Statement Generation
 * Implementation of statement code generation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 */

#include "llvm_stmt_gen.h"
#include "llvm_basic_stmts.h"
#include "llvm_debug.h"
#include "llvm_for_loops.h"
#include "llvm_pattern_matching.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Generate code for statements
void generate_statement(LLVMBackendData *data, const ASTNode *node) {
    if (!data) {
        return;
    }

    if (!node) {
        return;
    }

    // Set debug location for this statement
    set_debug_location(data, node);

    switch (node->type) {
    case AST_RETURN_STMT:
        generate_return_statement(data, node);
        break;

    case AST_LET_STMT:
        generate_let_statement(data, node);
        break;

    case AST_ASSIGNMENT:
        generate_assignment_statement(data, node);
        break;

    case AST_IF_STMT:
        generate_if_statement(data, node);
        break;

    case AST_FOR_STMT:
        if (!generate_for_loop_iteration(data, node)) {
            llvm_backend_report_error(data, node, "Failed to generate for loop iteration");
        }
        break;

    case AST_BLOCK:
        generate_block_statement(data, node);
        break;

    case AST_EXPR_STMT:
        generate_expression_statement(data, node);
        break;

    case AST_BREAK_STMT:
        generate_break_statement(data, node);
        break;

    case AST_CONTINUE_STMT:
        generate_continue_statement(data, node);
        break;

    case AST_MATCH_STMT:
        generate_match_statement(data, node);
        break;

    default:
        llvm_backend_report_error_printf(data, node, "Unknown statement type: %d", node->type);
        break;
    }
}