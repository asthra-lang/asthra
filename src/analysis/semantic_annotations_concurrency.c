/**
 * Asthra Programming Language Compiler
 * Semantic Analysis - Annotations Concurrency Module Implementation
 *
 * Copyright (c) 2024 Asthra Project
 * Licensed under the terms specified in LICENSE
 *
 * Implementation of Tier 1/2 concurrency annotation handling for semantic annotations
 * Split from semantic_annotations.c for better modularity
 */

#include "semantic_annotations_concurrency.h"
#include "semantic_annotations_errors.h"
#include "semantic_annotations_validation.h"
#include <string.h>

// =============================================================================
// CONCURRENCY ANNOTATION FUNCTIONS
// =============================================================================

bool validate_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *tag_node,
                                           ASTNode *target_node) {
    if (!analyzer || !tag_node || !target_node) {
        return false;
    }

    // Validate that this is actually a non_deterministic annotation
    if (tag_node->type != AST_SEMANTIC_TAG) {
        return false;
    }

    const char *annotation_name = tag_node->data.semantic_tag.name;
    if (!annotation_name || strcmp(annotation_name, "non_deterministic") != 0) {
        return false;
    }

    // Validate context appropriateness
    return validate_annotation_context(analyzer, annotation_name, target_node);
}

bool requires_non_deterministic_annotation(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node) {
        return false;
    }

    // Check if this node represents a Tier 2 concurrency feature that requires
    // #[non_deterministic] annotation

    switch (node->type) {
        // Tier 2 features that require #[non_deterministic] annotation
        // Note: In the current plan, Tier 2 features would be in stdlib,
        // but if we had select statements or advanced await with timeouts,
        // they would be checked here

        // For future Tier 2 features:
        // case AST_SELECT_STMT:
        // case AST_AWAIT_WITH_TIMEOUT:
        //     return true;

    default:
        return false;
    }
}

bool validate_tier2_concurrency_annotation(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node) {
        return false;
    }

    // Check if the node requires non-deterministic annotation
    if (requires_non_deterministic_annotation(analyzer, node)) {
        if (!has_non_deterministic_annotation(node)) {
            // Report error for missing annotation
            const char *feature_name = "unknown feature";

            switch (node->type) {
            // Future Tier 2 features:
            // case AST_SELECT_STMT:
            //     feature_name = "select statement";
            //     break;
            // case AST_AWAIT_WITH_TIMEOUT:
            //     feature_name = "await with timeout";
            //     break;
            default:
                feature_name = "advanced concurrency feature";
                break;
            }

            report_missing_non_deterministic_annotation(analyzer, node, feature_name);
            return false;
        }
    }

    return true;
}

bool analyze_tier1_concurrency_feature(SemanticAnalyzer *analyzer, ASTNode *node) {
    if (!analyzer || !node) {
        return false;
    }

    switch (node->type) {
    case AST_SPAWN_STMT:
        // Basic spawn is Tier 1 - no annotation required
        // TODO: Validate the spawned function exists and is callable
        return true;

    case AST_SPAWN_WITH_HANDLE_STMT:
        // spawn_with_handle is Tier 1 - no annotation required
        // TODO: Validate the handle variable and spawned function
        return true;

    case AST_AWAIT_EXPR:
        // Basic await is Tier 1 - no annotation required
        // TODO: Validate the awaited handle is valid
        return true;

    default:
        return false;
    }
}
