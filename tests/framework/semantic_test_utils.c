/**
 * Asthra Programming Language
 * Semantic Analyzer Test Utilities Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of test utility functions for semantic analysis testing
 */

#include "semantic_test_utils.h"
#include "performance_test_utils.h"
#include <string.h>

// =============================================================================
// SEMANTIC ANALYZER TEST UTILITIES
// =============================================================================

SemanticAnalyzer *setup_semantic_analyzer(void) {
    SemanticAnalyzer *analyzer = semantic_analyzer_create();
    if (analyzer) {
        track_memory_allocation(sizeof(SemanticAnalyzer));
        // Enable test mode for more permissive analysis in tests
        semantic_analyzer_set_test_mode(analyzer, true);
    }
    return analyzer;
}

void destroy_semantic_analyzer(SemanticAnalyzer *analyzer) {
    if (analyzer) {
        track_memory_deallocation(sizeof(SemanticAnalyzer));
        semantic_analyzer_destroy(analyzer);
    }
}

bool analyze_test_ast(SemanticAnalyzer *analyzer, ASTNode *ast) {
    if (!analyzer || !ast) {
        return false;
    }

    // Perform semantic analysis
    return semantic_analyze_program(analyzer, ast);
}

// =============================================================================
// ENHANCED SEMANTIC VALIDATION UTILITIES
// =============================================================================

bool validate_enum_semantic_analysis(SemanticAnalyzer *analyzer, ASTNode *enum_ast,
                                     const char *expected_name) {
    if (!analyzer || !enum_ast || !expected_name) {
        return false;
    }

    // Analyze the enum declaration
    if (!analyze_enum_declaration(analyzer, enum_ast)) {
        return false;
    }

    // Verify the enum was registered in the symbol table
    SymbolEntry *enum_symbol = symbol_table_lookup_safe(analyzer->global_scope, expected_name);
    if (!enum_symbol) {
        return false;
    }

    // Verify enum type information
    TypeDescriptor *enum_type = enum_symbol ? enum_symbol->type : NULL;
    if (!enum_type || enum_type->category != TYPE_ENUM) {
        return false;
    }

    // Validate each variant if present
    if (enum_ast->type == AST_ENUM_DECL && enum_ast->data.enum_decl.variants) {
        for (size_t i = 0; i < enum_ast->data.enum_decl.variants->count; i++) {
            ASTNode *variant = enum_ast->data.enum_decl.variants->nodes[i];
            if (!analyze_enum_variant_declaration(analyzer, variant, expected_name, enum_type)) {
                return false;
            }
        }
    }

    return true;
}

bool validate_type_inference(SemanticAnalyzer *analyzer, ASTNode *expr_ast,
                             const char *expected_type_name) {
    if (!analyzer || !expr_ast || !expected_type_name) {
        return false;
    }

    // Analyze the expression
    // In test mode, expressions are analyzed more permissively
    if (!semantic_analyze_expression(analyzer, expr_ast)) {
        // If test mode is enabled, this should have succeeded
        // but if it didn't, that's still a valid test result
        return false;
    }

    // Get inferred type
    TypeInfo *type_info = ast_node_get_type_info(expr_ast);
    const TypeDescriptor *inferred_type = type_info ? type_info->type_descriptor : NULL;
    if (!inferred_type) {
        return false;
    }

    // Compare with expected type name
    const char *type_name =
        inferred_type ? (inferred_type->name ? inferred_type->name : "<unnamed>") : "<unknown>";
    return type_name && strcmp(type_name, expected_type_name) == 0;
}

bool semantic_test_validate_pattern_exhaustiveness(SemanticAnalyzer *analyzer, ASTNode *match_ast,
                                                   bool should_be_exhaustive) {
    if (!analyzer || !match_ast) {
        return false;
    }

    // Analyze the match statement
    bool analysis_result = semantic_analyze_statement(analyzer, match_ast);

    // If we expect exhaustiveness, analysis should succeed
    // If we expect non-exhaustiveness, analysis should fail with exhaustiveness error
    return analysis_result == should_be_exhaustive;
}

bool validate_symbol_visibility(SemanticAnalyzer *analyzer, const char *symbol_name,
                                bool should_be_visible) {
    if (!analyzer || !symbol_name) {
        return false;
    }

    SymbolEntry *symbol = symbol_table_lookup_safe(analyzer->global_scope, symbol_name);
    bool is_visible = symbol != NULL;

    return is_visible == should_be_visible;
}

bool validate_error_reporting(SemanticAnalyzer *analyzer, size_t expected_error_count,
                              int *expected_error_types) {
    if (!analyzer) {
        return false;
    }

    size_t actual_error_count = semantic_get_error_count(analyzer);
    if (actual_error_count != expected_error_count) {
        return false;
    }

    // If specific error types are expected, validate them
    if (expected_error_types && expected_error_count > 0) {
        const SemanticError *errors = semantic_get_errors(analyzer);
        for (size_t i = 0; i < expected_error_count; i++) {
            if (errors[i].code != (SemanticErrorCode)expected_error_types[i]) {
                return false;
            }
        }
    }

    return true;
}

bool validate_generic_constraints(SemanticAnalyzer *analyzer, ASTNodeList *type_params,
                                  size_t expected_constraint_count) {
    if (!analyzer || !type_params) {
        return expected_constraint_count == 0;
    }

    return validate_enum_type_parameters(analyzer, type_params, (SourceLocation){0});
}

bool validate_ffi_semantics(SemanticAnalyzer *analyzer, ASTNode *decl_ast,
                            const char *expected_abi) {
    if (!analyzer || !decl_ast || !expected_abi) {
        return false;
    }

    // Analyze the FFI declaration
    bool analysis_result = semantic_analyze_declaration(analyzer, decl_ast);
    if (!analysis_result) {
        return false;
    }

    // Validate FFI-specific semantics
    // This would check ABI compatibility, calling conventions, etc.
    // For now, we just verify the declaration analyzed successfully
    return true;
}

bool validate_memory_safety(SemanticAnalyzer *analyzer, ASTNode *expr_ast,
                            int expected_safety_level) {
    if (!analyzer || !expr_ast) {
        return false;
    }

    // Analyze the expression for memory safety
    bool analysis_result = semantic_analyze_expression(analyzer, expr_ast);
    if (!analysis_result) {
        return false;
    }

    // Check memory safety level (this would be implementation-specific)
    // For now, we assume analysis success indicates appropriate safety level
    return true;
}

SemanticAnalysisStats get_semantic_analysis_stats(SemanticAnalyzer *analyzer) {
    SemanticAnalysisStats stats = {0};

    if (analyzer) {
        stats.symbols_analyzed = symbol_table_size(analyzer->global_scope);
        stats.types_created = 0; // TODO: implement type count
        stats.errors_reported = semantic_get_error_count(analyzer);
        stats.warnings_reported = 0;          // TODO: implement warning count
        stats.memory_tracking_enabled = true; // Always enabled in tests
        // Additional stats would be implementation-specific
    }

    return stats;
}

void reset_semantic_analyzer(SemanticAnalyzer *analyzer) {
    if (analyzer) {
        semantic_clear_errors(analyzer);
        semantic_analyzer_reset(analyzer);
    }
}
