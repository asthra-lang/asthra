/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Main Analysis Functions
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Core semantic analysis functions for programs and declarations
 */

#include "../parser/ast.h"
#include "semantic_annotations.h"
#include "semantic_const_declarations.h"
#include "semantic_core.h"
#include "semantic_declarations.h"
#include "semantic_errors.h"
#include "semantic_ffi.h"
#include "semantic_scopes.h"
#include "semantic_statistics.h"
#include "semantic_structs.h"
#include "semantic_symbols.h"
#include "semantic_types.h"
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

bool semantic_analyze_program(SemanticAnalyzer *analyzer, ASTNode *program) {
    if (!analyzer || !program || program->type != AST_PROGRAM) {
        return false;
    }

    // Analyze imports first
    if (program->data.program.imports) {
        ASTNodeList *imports = program->data.program.imports;
        size_t import_count = ast_node_list_size(imports);

        for (size_t i = 0; i < import_count; i++) {
            ASTNode *import_decl = ast_node_list_get(imports, i);
            if (!semantic_analyze_declaration(analyzer, import_decl)) {
                return false;
            }
        }
    }

    // Then analyze declarations
    if (program->data.program.declarations) {
        ASTNodeList *declarations = program->data.program.declarations;
        size_t decl_count = ast_node_list_size(declarations);

        for (size_t i = 0; i < decl_count; i++) {
            ASTNode *decl = ast_node_list_get(declarations, i);
            if (!semantic_analyze_declaration(analyzer, decl)) {
                return false;
            }
        }
    }

    // Return false if any errors were found during analysis
    return analyzer->error_count == 0;
}

bool semantic_analyze_declaration(SemanticAnalyzer *analyzer, ASTNode *decl) {
    if (!analyzer || !decl) {
        return false;
    }

    ATOMIC_FETCH_ADD(&analyzer->stats.nodes_analyzed, 1);

    switch (decl->type) {
    case AST_IMPORT_DECL:
        return analyze_import_declaration(analyzer, decl);

    case AST_VISIBILITY_MODIFIER:
        return analyze_visibility_modifier(analyzer, decl);

    case AST_FUNCTION_DECL:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_function_declaration(analyzer, decl);

    case AST_STRUCT_DECL:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_struct_declaration(analyzer, decl);

    case AST_ENUM_DECL:
        // Phase 4: Integrate enum declaration analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_enum_declaration(analyzer, decl);

    case AST_EXTERN_DECL:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_extern_declaration(analyzer, decl);

    case AST_IMPL_BLOCK:
        // Phase 3: Integrate annotation analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_impl_block(analyzer, decl);

    case AST_CONST_DECL:
        // Phase 2: Integrate const declaration analysis into pipeline
        if (!analyze_declaration_annotations(analyzer, decl)) {
            return false;
        }
        return analyze_const_declaration(analyzer, decl);

    default:
        semantic_report_error(analyzer, SEMANTIC_ERROR_NONE, decl->location,
                              "Unsupported declaration type: %d", decl->type);
        return false;
    }
}
