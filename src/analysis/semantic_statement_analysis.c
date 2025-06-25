/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Statement Analysis Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analysis functions for statements
 */

#include "semantic_statement_analysis.h"
#include "../parser/ast.h"
#include "semantic_annotations.h"
#include "semantic_arrays.h"
#include "semantic_binary_unary.h"
#include "semantic_calls.h"
#include "semantic_const_declarations.h"
#include "semantic_core.h"
#include "semantic_declarations.h"
#include "semantic_errors.h"
#include "semantic_expression_utils.h"
#include "semantic_expressions.h"
#include "semantic_ffi.h"
#include "semantic_field_access.h"
#include "semantic_literals.h"
#include "semantic_loops.h"
#include "semantic_scopes.h"
#include "semantic_security.h"
#include "semantic_statements.h"
#include "semantic_statistics.h"
#include "semantic_structs.h"
#include "semantic_symbols.h"
#include "semantic_type_creation.h"
#include "semantic_type_helpers.h"
#include "semantic_types.h"
#include "semantic_utilities.h"
#include "type_info.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// C17 feature detection and compatibility
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201710L
#define ASTHRA_HAS_C17 1
#else
#define ASTHRA_HAS_C17 0
#endif

#if ASTHRA_HAS_C17 && !defined(__STDC_NO_ATOMICS__)
#define ASTHRA_HAS_ATOMICS 1
#else
#define ASTHRA_HAS_ATOMICS 0
#endif

// Atomic operation wrapper for fetch_add only (used in this file)
#if ASTHRA_HAS_ATOMICS
#define ATOMIC_FETCH_ADD(ptr, val) atomic_fetch_add(ptr, val)
#else
#define ATOMIC_FETCH_ADD(ptr, val) __sync_fetch_and_add(ptr, val)
#endif

bool semantic_analyze_statement(SemanticAnalyzer *analyzer, ASTNode *stmt) {
    if (!analyzer || !stmt)
        return false;

    ATOMIC_FETCH_ADD(&analyzer->stats.nodes_analyzed, 1);

    switch (stmt->type) {
    case AST_LET_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_let_statement(analyzer, stmt);

    case AST_BLOCK:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_block_statement(analyzer, stmt);

    case AST_EXPR_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_expression_statement(analyzer, stmt);

    case AST_RETURN_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_return_statement(analyzer, stmt);

    case AST_IF_STMT: {
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        bool result = analyze_if_statement(analyzer, stmt);
        if (result) {
            // Store type information for if-else expressions
            TypeDescriptor *type = semantic_get_expression_type(analyzer, stmt);
            if (type) {
                semantic_set_expression_type(analyzer, stmt, type);
                type_descriptor_release(type);
            }
        }
        return result;
    }

    case AST_IF_LET_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_if_let_statement(analyzer, stmt);

    case AST_MATCH_STMT:
        // Phase 3: Analyze statement annotations
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_match_statement(analyzer, stmt);

    case AST_SPAWN_STMT:
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, stmt)) {
            return false;
        }
        return analyze_spawn_statement(analyzer, stmt);

    case AST_SPAWN_WITH_HANDLE_STMT:
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, stmt)) {
            return false;
        }
        return analyze_spawn_with_handle_statement(analyzer, stmt);

    case AST_AWAIT_EXPR:
        // Note: await is an expression, but might appear as statement too
        // Phase 3: Analyze Tier 1 concurrency features
        if (!analyze_tier1_concurrency_feature(analyzer, stmt)) {
            return false;
        }
        return analyze_await_statement(analyzer, stmt);

    case AST_UNSAFE_BLOCK:
        // Analyze unsafe blocks with safety validation
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_unsafe_block_statement(analyzer, stmt);

    case AST_ASSIGNMENT:
        // Analyze assignment statements with mutability validation
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_assignment_validation(analyzer, stmt);

    case AST_FOR_STMT:
        // Analyze for loop statements
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_for_statement(analyzer, stmt);

    case AST_BREAK_STMT:
        // Analyze break statements
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_break_statement(analyzer, stmt);

    case AST_CONTINUE_STMT:
        // Analyze continue statements
        if (!analyze_statement_annotations(analyzer, stmt)) {
            return false;
        }
        return analyze_continue_statement(analyzer, stmt);

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, stmt->location,
                              "Unsupported statement type: %d", stmt->type);
        return false;
    }
}